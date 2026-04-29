#include "RealtimeTransfer.h"
#include "Branch.h"
#include <thread>
#include <chrono>
#include <random>
#include <string>

std::string RealtimeTransferManager::generateId() {
    static std::mutex idMtx;
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<int> dist(1000, 9999);
    std::lock_guard<std::mutex> lk(idMtx);
    auto ts = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::to_string(ts % 10000000LL) + "_" + std::to_string(dist(rng));
}

std::string RealtimeTransferManager::startTransfer(
    const std::string& barcode, int originId, int destId, bool byTime,
    BranchManager& bm, Graph& graph, std::string& error)
{
    Branch* origin = bm.findBranch(originId);
    if (!origin) { error = "Sucursal origen no existe (ID=" + std::to_string(originId) + ")"; return ""; }
    Branch* dest = bm.findBranch(destId);
    if (!dest)   { error = "Sucursal destino no existe (ID=" + std::to_string(destId) + ")"; return ""; }
    if (originId == destId) { error = "Origen y destino son la misma sucursal"; return ""; }
    if (!origin->searchByBarcode(barcode)) {
        error = "Producto '" + barcode + "' no encontrado en sucursal origen";
        return "";
    }
    PathResult pr = byTime ? graph.shortestPathByTime(originId, destId)
                           : graph.shortestPathByCost(originId, destId);
    if (pr.length == 0) {
        error = "No existe ruta entre sucursal " + std::to_string(originId) +
                " y " + std::to_string(destId);
        return "";
    }

    std::string id = generateId();
    {
        std::lock_guard<std::mutex> lk(_mutex);
        TransferProgress prog;
        prog.id               = id;
        prog.completed        = false;
        prog.totalAccumulated = 0;
        prog.currentBranch    = originId;
        prog.currentQueue     = QueueType::INGRESO;
        _active[id]           = std::move(prog);
    }

    std::thread t(&RealtimeTransferManager::runTransfer, this,
                  id, barcode, originId, destId, byTime, &bm, &graph);
    t.detach();
    return id;
}

bool RealtimeTransferManager::getProgress(const std::string& id, TransferProgress& out) {
    std::lock_guard<std::mutex> lk(_mutex);
    auto it = _active.find(id);
    if (it == _active.end()) return false;
    out = it->second;
    return true;
}

void RealtimeTransferManager::updateProgress(
    const std::string& id, const std::function<void(TransferProgress&)>& fn)
{
    std::lock_guard<std::mutex> lk(_mutex);
    auto it = _active.find(id);
    if (it != _active.end()) fn(it->second);
}

void RealtimeTransferManager::runTransfer(
    std::string transferId, std::string barcode,
    int originId, int destId, bool byTime,
    BranchManager* bm, Graph* g)
{
    PathResult pr = byTime ? g->shortestPathByTime(originId, destId)
                           : g->shortestPathByCost(originId, destId);
    if (pr.length == 0) {
        updateProgress(transferId, [](TransferProgress& p) {
            p.completed = true;
            p.error = "Ruta no encontrada al iniciar hilo";
        });
        return;
    }

    {
        Branch* origin = bm->findBranch(originId);
        if (!origin || !origin->searchByBarcode(barcode)) {
            updateProgress(transferId, [](TransferProgress& p) { p.completed = true; });
            return;
        }
    }

    double accumulated = 0.0;

    // Sleeps `secs` seconds, ticking accumulated time and updating the snapshot elapsed.
    auto sleepTick = [&](int secs, int bId, QueueType qt) {
        for (int s = 0; s < secs; ++s) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            accumulated += 1.0;
            double acc   = accumulated;
            int    elapsed = s + 1;
            updateProgress(transferId, [acc, elapsed](TransferProgress& p) {
                p.totalAccumulated = acc;
                if (!p.queues.empty()) p.queues[0].elapsedSeconds = elapsed;
            });
        }
        (void)bId; (void)qt;
    };

    for (int i = 0; i < pr.length; ++i) {
        int     bId  = pr.path[i];
        Branch* b    = bm->findBranch(bId);
        if (!b) continue;
        bool isLast       = (i == pr.length - 1);
        std::string bName = b->getNombre();

        // ── Ingreso ──────────────────────────────────────────────────────────
        {
            int t = b->getTiempoIngreso();
            std::string step = "Ingreso en " + bName + " (+" + std::to_string(t) + " min)";
            updateProgress(transferId, [bId, t, step](TransferProgress& p) {
                p.currentBranch = bId;
                p.currentQueue  = QueueType::INGRESO;
                p.steps.push_back(step);
                p.queues = {{ bId, QueueType::INGRESO, 1, 0 }};
            });
            sleepTick(t, bId, QueueType::INGRESO);
        }

        if (!isLast) {
            int nextId         = pr.path[i + 1];
            Branch* nextB      = bm->findBranch(nextId);
            std::string nextName = nextB ? nextB->getNombre()
                                        : "Sucursal " + std::to_string(nextId);

            // ── Preparacion ───────────────────────────────────────────────
            {
                int t = b->getTiempoPreparacion();
                std::string step = "Preparacion en " + bName + " (+" + std::to_string(t) + " min)";
                updateProgress(transferId, [bId, t, step](TransferProgress& p) {
                    p.currentBranch = bId;
                    p.currentQueue  = QueueType::PREPARACION;
                    p.steps.push_back(step);
                    p.queues = {{ bId, QueueType::PREPARACION, 1, 0 }};
                });
                sleepTick(t, bId, QueueType::PREPARACION);
            }

            // ── Salida ────────────────────────────────────────────────────
            {
                int t = b->getIntervaloDespacho();
                std::string step = "Salida desde " + bName + " hacia " + nextName +
                                   " (+" + std::to_string(t) + " min)";
                updateProgress(transferId, [bId, t, step](TransferProgress& p) {
                    p.currentBranch = bId;
                    p.currentQueue  = QueueType::SALIDA;
                    p.steps.push_back(step);
                    p.queues = {{ bId, QueueType::SALIDA, 1, 0 }};
                });
                sleepTick(t, bId, QueueType::SALIDA);
            }

            // ── Tránsito ──────────────────────────────────────────────────
            {
                double tEdge    = g->edgeWeight(bId, nextId, false);
                if (tEdge < 0) tEdge = 0;
                int    tEdgeInt = static_cast<int>(tEdge);
                std::string step = "Transito " + bName + " -> " + nextName +
                                   " (+" + std::to_string(tEdgeInt) + " min)";
                updateProgress(transferId, [step](TransferProgress& p) {
                    p.steps.push_back(step);
                    p.queues = {};
                });
                sleepTick(tEdgeInt, nextId, QueueType::INGRESO);
            }
        } else {
            // ── Entrega final ─────────────────────────────────────────────
            std::string step = "Entrega en " + bName + " - producto disponible";
            updateProgress(transferId, [step](TransferProgress& p) {
                p.steps.push_back(step);
                p.queues = {};
            });
        }
    }

    // ── Mover producto en inventario ─────────────────────────────────────────
    std::string moveError;
    Branch* origin = bm->findBranch(originId);
    Branch* dest   = bm->findBranch(destId);
    if (origin && dest) {
        Product* prod = origin->searchByBarcode(barcode);
        if (prod) {
            Product moved  = *prod;
            moved.branchId = destId;
            moved.status   = ProductStatus::AVAILABLE;
            if (dest->insertProduct(moved)) {
                if (!origin->removeProduct(barcode)) {
                    dest->removeProduct(barcode);
                    moveError = "Error al eliminar de origen (rollback aplicado)";
                }
            } else {
                moveError = "Error al insertar en destino";
            }
        } else {
            moveError = "Producto ya no existe en origen al finalizar";
        }
    } else {
        moveError = "Sucursal no encontrada al finalizar transferencia";
    }

    double finalAcc = accumulated;
    updateProgress(transferId, [finalAcc, moveError](TransferProgress& p) {
        p.completed        = true;
        p.totalAccumulated = finalAcc;
        if (!moveError.empty()) p.error = moveError;
    });
}
