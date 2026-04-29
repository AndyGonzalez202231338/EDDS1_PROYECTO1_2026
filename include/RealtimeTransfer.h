#ifndef REALTIME_TRANSFER_H
#define REALTIME_TRANSFER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <functional>
#include "BranchManager.h"
#include "Graph.h"

enum class QueueType { INGRESO, PREPARACION, SALIDA };

struct QueueSnapshot {
    int       branchId;
    QueueType queue;
    int       position;
    int       elapsedSeconds;
};

struct TransferProgress {
    std::string              id;
    bool                     completed    = false;
    double                   totalAccumulated = 0;
    int                      currentBranch = -1;
    QueueType                currentQueue  = QueueType::INGRESO;
    std::vector<std::string> steps;
    std::vector<QueueSnapshot> queues;
    std::string              error;
};

class RealtimeTransferManager {
public:
    // Validates and launches thread. Returns transferId or "" on error (sets error).
    std::string startTransfer(const std::string& barcode,
                              int originId, int destId, bool byTime,
                              BranchManager& bm, Graph& graph,
                              std::string& error);

    // Thread-safe read. Returns false if id not found.
    bool getProgress(const std::string& id, TransferProgress& out);

private:
    std::unordered_map<std::string, TransferProgress> _active;
    std::mutex _mutex;

    void runTransfer(std::string transferId, std::string barcode,
                     int originId, int destId, bool byTime,
                     BranchManager* bm, Graph* g);

    void updateProgress(const std::string& id,
                        const std::function<void(TransferProgress&)>& fn);

    static std::string generateId();
};

#endif // REALTIME_TRANSFER_H
