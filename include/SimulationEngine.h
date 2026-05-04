#ifndef SIMULATION_ENGINE_H
#define SIMULATION_ENGINE_H

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include "Product.h"
#include "BranchManager.h"
#include "Graph.h"
#include "Stack.h"

// ── Producto en tránsito con metadatos de simulación ─────────────────────────

struct SimEntry {
    std::string transferId;
    Product     product;
    int         arrivalTick; // tick en que entró a la cola actual
    int         path[200];
    int         pathLen;
    int         routePos;    // índice en path del branch donde está actualmente
    bool        preHandled = false; // true: origen ya fue gestionado por el caller
};

// ── Estado de colas por sucursal ─────────────────────────────────────────────

struct BranchSimState {
    int branchId         = -1;
    int tiempoIngreso    = 1;
    int tiempoTraspaso   = 1;
    int intervaloDespacho = 1;
    std::deque<SimEntry> colaIngreso;
    std::deque<SimEntry> colaTraspaso;
    std::deque<SimEntry> colaSalida;
    int lastDispatchTick = -99999; // tick del último despacho
};

// ── Snapshots para la API ─────────────────────────────────────────────────────

struct SimQueueEntry {
    std::string transferId;
    std::string barcode;
    std::string productName;
    int         waitTicks;
    double      etaTicks;
};

struct BranchSimSnapshot {
    int         branchId;
    std::string branchName;
    std::vector<SimQueueEntry> colaIngreso;
    std::vector<SimQueueEntry> colaTraspaso;
    std::vector<SimQueueEntry> colaSalida;
};

struct CompletedTransferInfo {
    std::string transferId;
    std::string barcode;
    int         completedTick;
    int         originId;
    int         destId;
    bool        ok;
    std::string error;
};

struct SimSnapshot {
    int currentTick;
    std::vector<BranchSimSnapshot>    branches;
    std::vector<CompletedTransferInfo> recentCompleted;
};

// ── Transferencia pendiente en el batch ───────────────────────────────────────

struct BatchTransferRequest {
    std::string barcode;
    int         originId;
    int         destId;
    bool        byTime;
    std::string label;    // descripcion opcional del usuario
    int         quantity; // cantidad a transferir (0 = stock completo)
};

// Resultado de ejecutar el batch completo
struct BatchExecuteResult {
    std::string barcode;
    int         originId;
    int         destId;
    std::string transferId; // vacio si hubo error al iniciar
    std::string error;
};

// ── Motor de simulación ───────────────────────────────────────────────────────

class SimulationEngine {
public:
    SimulationEngine(BranchManager& bm, Graph& g);
    ~SimulationEngine();

    // Inicia una transferencia en la simulación. Devuelve transferId o "" en error.
    // quantity=0 significa "stock completo del producto"; >0 transfiere esa cantidad.
    std::string startTransfer(const std::string& barcode,
                              int originId, int destId, bool byTime,
                              std::string& error,
                              int quantity = 0);

    // ── Batch queue ──────────────────────────────────────────────────────────
    // Agrega una transferencia a la cola pendiente sin ejecutarla aún.
    // Devuelve false si la validación básica falla (error se rellena).
    bool addToBatch(const BatchTransferRequest& req, std::string& error);

    // Lista de transferencias pendientes (en el orden en que se agregaron).
    std::vector<BatchTransferRequest> getBatch() const;

    // Elimina todas las transferencias pendientes sin ejecutarlas.
    void clearBatch();

    // Ejecuta todas las pendientes en orden usando una Stack interna.
    // Retorna un resultado por cada transferencia.
    std::vector<BatchExecuteResult> executeBatch();

    // Snapshot thread-safe del estado actual.
    SimSnapshot getSnapshot() const;

    int getCurrentTick() const { return _tick.load(); }

private:
    BranchManager&   _bm;
    Graph&           _g;
    mutable std::mutex _mutex;
    std::atomic<int>   _tick{0};
    std::atomic<bool>  _running{false};
    std::thread        _thread;

    std::unordered_map<int, BranchSimState> _states;
    std::vector<CompletedTransferInfo>       _completed;
    std::vector<BatchTransferRequest>        _pendingBatch;

    static std::string generateId();

    void runLoop();
    void processTick();

    BranchSimState& getOrCreateState(int branchId);
    void completeTransfer(SimEntry& e, int tick);

    // ETA en ticks desde ahora hasta que el producto llegue al destino.
    // queueName: "ingreso" | "traspaso" | "salida"
    double computeEta(const SimEntry& e, const std::string& queueName) const;
};

#endif // SIMULATION_ENGINE_H
