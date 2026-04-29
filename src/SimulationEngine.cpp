#include "SimulationEngine.h"
#include "Branch.h"
#include <chrono>
#include <random>
#include <algorithm>

// ── ID generator ─────────────────────────────────────────────────────────────

std::string SimulationEngine::generateId() {
    static std::mutex m;
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<int> dist(1000, 9999);
    std::lock_guard<std::mutex> lk(m);
    auto ts = std::chrono::steady_clock::now().time_since_epoch().count();
    return "sim_" + std::to_string(ts % 10000000LL) + "_" + std::to_string(dist(rng));
}

// ── Constructor / destructor ─────────────────────────────────────────────────

SimulationEngine::SimulationEngine(BranchManager& bm, Graph& g)
    : _bm(bm), _g(g)
{
    _running = true;
    _thread  = std::thread(&SimulationEngine::runLoop, this);
}

SimulationEngine::~SimulationEngine() {
    _running = false;
    if (_thread.joinable()) _thread.join();
}

// ── Tick loop ─────────────────────────────────────────────────────────────────

void SimulationEngine::runLoop() {
    while (_running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ++_tick;
        std::lock_guard<std::mutex> lk(_mutex);
        processTick();
    }
}

// ── State helpers ─────────────────────────────────────────────────────────────

BranchSimState& SimulationEngine::getOrCreateState(int branchId) {
    auto it = _states.find(branchId);
    if (it != _states.end()) return it->second;

    BranchSimState s;
    s.branchId = branchId;
    Branch* b  = _bm.findBranch(branchId);
    if (b) {
        s.tiempoIngreso    = std::max(1, b->getTiempoIngreso());
        s.tiempoTraspaso   = std::max(1, b->getTiempoPreparacion());
        s.intervaloDespacho = std::max(1, b->getIntervaloDespacho());
    }
    _states[branchId] = std::move(s);
    return _states[branchId];
}

void SimulationEngine::completeTransfer(SimEntry& e, int tick) {
    CompletedTransferInfo ci;
    ci.transferId    = e.transferId;
    ci.barcode       = e.product.barcode;
    ci.completedTick = tick;
    ci.originId      = e.path[0];
    ci.destId        = e.path[e.pathLen - 1];
    ci.ok            = false;

    Branch* dest = _bm.findBranch(ci.destId);

    if (e.preHandled) {
        // Transferencia con quantity: insertar en destino y descontar de origen aquí.
        Branch* origin = _bm.findBranch(ci.originId);
        if (dest) {
            Product arrived = e.product;
            arrived.branchId = ci.destId;
            arrived.status   = ProductStatus::AVAILABLE;
            if (dest->insertProduct(arrived)) {
                ci.ok = true;
                // Descontar quantity del origen
                if (origin) {
                    Product* orig = origin->searchByBarcode(ci.barcode);
                    if (orig) {
                        int remaining = orig->stock - static_cast<int>(e.product.stock);
                        if (remaining <= 0) {
                            origin->removeProduct(ci.barcode);
                        } else {
                            Product reduced = *orig;
                            reduced.stock   = remaining;
                            origin->removeProduct(ci.barcode);
                            origin->insertProduct(reduced);
                        }
                    }
                }
            } else {
                ci.error = "Error al insertar en destino";
            }
        } else {
            ci.error = "Sucursal destino no encontrada";
        }
    } else {
        // Comportamiento original: mover el producto completo de origen a destino.
        Branch* origin = _bm.findBranch(ci.originId);
        if (origin && dest) {
            Product* prod = origin->searchByBarcode(ci.barcode);
            if (prod) {
                Product moved  = *prod;
                moved.branchId = ci.destId;
                moved.status   = ProductStatus::AVAILABLE;
                if (dest->insertProduct(moved)) {
                    if (origin->removeProduct(ci.barcode)) {
                        ci.ok = true;
                    } else {
                        dest->removeProduct(ci.barcode); // rollback
                        ci.error = "Error al eliminar de origen";
                    }
                } else {
                    ci.error = "Error al insertar en destino";
                }
            } else {
                ci.error = "Producto ya no existe en origen";
            }
        } else {
            ci.error = "Sucursal no encontrada al completar";
        }
    }

    _completed.push_back(ci);
}

// ── Tick processing ───────────────────────────────────────────────────────────
//
// Orden de procesamiento por sucursal:
//   1. colaSalida  → dispatch a colaIngreso de la siguiente sucursal
//   2. colaTraspaso → colaSalida (si ya esperó tiempoTraspaso)
//   3. colaIngreso  → colaTraspaso (o completa si es destino)
//
// Los movimientos de colaSalida se acumulan y se aplican AL FINAL
// para no procesar en el mismo tick el producto recién llegado.

void SimulationEngine::processTick() {
    int tick = _tick.load();

    struct PendingMove { SimEntry entry; int targetBranchId; }; // -1 = completar
    std::vector<PendingMove> pending;

    // Snapshot de IDs activos para evitar iterar sobre un mapa en modificación
    std::vector<int> ids;
    ids.reserve(_states.size());
    for (auto& [id, _] : _states) ids.push_back(id);

    for (int bId : ids) {
        BranchSimState& s = _states[bId];

        // 1. colaSalida: despachar un producto si intervaloDespacho ha expirado
        if (!s.colaSalida.empty()) {
            if (tick - s.lastDispatchTick >= s.intervaloDespacho) {
                SimEntry e = s.colaSalida.front();
                s.colaSalida.pop_front();
                s.lastDispatchTick = tick;
                e.routePos++;
                e.arrivalTick = tick;
                int nextId = e.path[e.routePos];
                pending.push_back({e, nextId});
            }
        }

        // 2. colaTraspaso: mover al frente a colaSalida si ya esperó tiempoTraspaso
        if (!s.colaTraspaso.empty()) {
            SimEntry& front = s.colaTraspaso.front();
            if (tick - front.arrivalTick >= s.tiempoTraspaso) {
                SimEntry e = front;
                s.colaTraspaso.pop_front();
                e.arrivalTick = tick;
                s.colaSalida.push_back(e);
            }
        }

        // 3. colaIngreso: mover al frente a colaTraspaso o completar si es destino
        if (!s.colaIngreso.empty()) {
            SimEntry& front = s.colaIngreso.front();
            if (tick - front.arrivalTick >= s.tiempoIngreso) {
                SimEntry e = front;
                s.colaIngreso.pop_front();
                bool isDest = (e.routePos == e.pathLen - 1);
                if (isDest) {
                    pending.push_back({e, -1});
                } else {
                    e.arrivalTick = tick;
                    s.colaTraspaso.push_back(e);
                }
            }
        }
    }

    // Aplicar movimientos pendientes
    for (auto& pm : pending) {
        if (pm.targetBranchId == -1) {
            completeTransfer(pm.entry, tick);
        } else {
            getOrCreateState(pm.targetBranchId).colaIngreso.push_back(pm.entry);
        }
    }
}

// ── Iniciar transferencia ─────────────────────────────────────────────────────

std::string SimulationEngine::startTransfer(const std::string& barcode,
                                            int originId, int destId, bool byTime,
                                            std::string& error,
                                            int quantity)
{
    Branch* origin = _bm.findBranch(originId);
    if (!origin) { error = "Sucursal origen no existe (ID=" + std::to_string(originId) + ")"; return ""; }
    Branch* dest = _bm.findBranch(destId);
    if (!dest)   { error = "Sucursal destino no existe (ID=" + std::to_string(destId) + ")"; return ""; }
    if (originId == destId) { error = "Origen y destino son la misma sucursal"; return ""; }

    Product* prod = origin->searchByBarcode(barcode);
    if (!prod) { error = "Producto '" + barcode + "' no encontrado en sucursal origen"; return ""; }

    PathResult pr = byTime ? _g.shortestPathByTime(originId, destId)
                           : _g.shortestPathByCost(originId, destId);
    if (pr.length == 0) {
        error = "No existe ruta entre sucursal " + std::to_string(originId) +
                " y " + std::to_string(destId);
        return "";
    }

    int qty = (quantity > 0) ? quantity : prod->stock;

    std::string id = generateId();
    SimEntry e;
    e.transferId    = id;
    e.product       = *prod;
    e.product.stock = qty;
    e.product.status = ProductStatus::IN_TRANSIT;
    e.preHandled    = (quantity > 0); // origen ya gestionado por el caller
    e.pathLen     = pr.length;
    e.routePos    = 0;
    for (int i = 0; i < pr.length; ++i) e.path[i] = pr.path[i];

    {
        std::lock_guard<std::mutex> lk(_mutex);
        e.arrivalTick = _tick.load();
        getOrCreateState(originId).colaIngreso.push_back(e);
    }
    return id;
}

// ── ETA ───────────────────────────────────────────────────────────────────────

double SimulationEngine::computeEta(const SimEntry& e, const std::string& queueName) const {
    int tick    = _tick.load();
    int pos     = e.routePos;
    int elapsed = tick - e.arrivalTick;
    double eta  = 0.0;

    // Tiempo restante en la cola actual
    Branch* cur = _bm.findBranch(e.path[pos]);
    if (!cur) return -1.0;

    bool atDest = (pos == e.pathLen - 1);

    if (queueName == "ingreso") {
        int rem = cur->getTiempoIngreso() - elapsed;
        eta += (rem > 0) ? rem : 0;
        if (!atDest) {
            eta += cur->getTiempoPreparacion();
            eta += cur->getIntervaloDespacho();
            double w = _g.edgeWeight(e.path[pos], e.path[pos + 1], false);
            if (w > 0) eta += w;
        }
    } else if (queueName == "traspaso") {
        int rem = cur->getTiempoPreparacion() - elapsed;
        eta += (rem > 0) ? rem : 0;
        if (!atDest) {
            eta += cur->getIntervaloDespacho();
            double w = _g.edgeWeight(e.path[pos], e.path[pos + 1], false);
            if (w > 0) eta += w;
        }
    } else { // salida
        // El producto está esperando ser despachado
        if (!atDest && pos + 1 < e.pathLen) {
            double w = _g.edgeWeight(e.path[pos], e.path[pos + 1], false);
            if (w > 0) eta += w;
        }
    }

    // Sucursales restantes después de la posición actual
    for (int i = pos + 1; i < e.pathLen; ++i) {
        Branch* b = _bm.findBranch(e.path[i]);
        if (!b) continue;
        eta += b->getTiempoIngreso();
        if (i < e.pathLen - 1) {
            eta += b->getTiempoPreparacion();
            eta += b->getIntervaloDespacho();
            double w = _g.edgeWeight(e.path[i], e.path[i + 1], false);
            if (w > 0) eta += w;
        }
    }
    return eta;
}

// ── Batch queue ───────────────────────────────────────────────────────────────

bool SimulationEngine::addToBatch(const BatchTransferRequest& req, std::string& error) {
    if (!_bm.findBranch(req.originId)) {
        error = "Sucursal origen no existe (ID=" + std::to_string(req.originId) + ")";
        return false;
    }
    if (!_bm.findBranch(req.destId)) {
        error = "Sucursal destino no existe (ID=" + std::to_string(req.destId) + ")";
        return false;
    }
    if (req.originId == req.destId) {
        error = "Origen y destino son la misma sucursal";
        return false;
    }
    if (req.barcode.empty()) {
        error = "Barcode no puede estar vacio";
        return false;
    }
    std::lock_guard<std::mutex> lk(_mutex);
    _pendingBatch.push_back(req);
    return true;
}

std::vector<BatchTransferRequest> SimulationEngine::getBatch() const {
    std::lock_guard<std::mutex> lk(_mutex);
    return _pendingBatch;
}

void SimulationEngine::clearBatch() {
    std::lock_guard<std::mutex> lk(_mutex);
    _pendingBatch.clear();
}

std::vector<BatchExecuteResult> SimulationEngine::executeBatch() {
    // Tomar el batch y limpiar la lista pendiente
    std::vector<BatchTransferRequest> toExecute;
    {
        std::lock_guard<std::mutex> lk(_mutex);
        toExecute.swap(_pendingBatch);
    }

    // Cargar en una pila (LIFO) en orden inverso para que al sacar queden en orden original
    Stack<BatchTransferRequest> stack;
    for (int i = static_cast<int>(toExecute.size()) - 1; i >= 0; --i)
        stack.push(toExecute[i]);

    std::vector<BatchExecuteResult> results;
    while (!stack.isEmpty()) {
        BatchTransferRequest req = stack.pop();
        BatchExecuteResult r;
        r.barcode  = req.barcode;
        r.originId = req.originId;
        r.destId   = req.destId;
        r.transferId = startTransfer(req.barcode, req.originId, req.destId, req.byTime, r.error, req.quantity);
        results.push_back(r);
    }
    return results;
}

// ── Snapshot ──────────────────────────────────────────────────────────────────

SimSnapshot SimulationEngine::getSnapshot() const {
    std::lock_guard<std::mutex> lk(_mutex);
    int tick = _tick.load();

    SimSnapshot snap;
    snap.currentTick = tick;

    auto fillQueue = [&](const std::deque<SimEntry>& q,
                         std::vector<SimQueueEntry>& out,
                         const std::string& qName) {
        for (const auto& e : q) {
            SimQueueEntry sq;
            sq.transferId  = e.transferId;
            sq.barcode     = e.product.barcode;
            sq.productName = e.product.name;
            sq.waitTicks   = tick - e.arrivalTick;
            sq.etaTicks    = computeEta(e, qName);
            out.push_back(sq);
        }
    };

    for (const auto& [bId, s] : _states) {
        if (s.colaIngreso.empty() && s.colaTraspaso.empty() && s.colaSalida.empty())
            continue;

        Branch* b = _bm.findBranch(bId);
        BranchSimSnapshot bs;
        bs.branchId   = bId;
        bs.branchName = b ? b->getNombre() : "Sucursal " + std::to_string(bId);
        fillQueue(s.colaIngreso,  bs.colaIngreso,  "ingreso");
        fillQueue(s.colaTraspaso, bs.colaTraspaso, "traspaso");
        fillQueue(s.colaSalida,   bs.colaSalida,   "salida");
        snap.branches.push_back(std::move(bs));
    }

    // Últimas 20 transferencias completadas
    int start = std::max(0, (int)_completed.size() - 20);
    for (int i = start; i < (int)_completed.size(); ++i)
        snap.recentCompleted.push_back(_completed[i]);

    return snap;
}
