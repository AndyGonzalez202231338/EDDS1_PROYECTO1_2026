#ifndef TRANSFER_SERVICE_H
#define TRANSFER_SERVICE_H

#include <string>
#include "BranchManager.h"
#include "Graph.h"

struct TransferResult {
    bool        ok;
    std::string error;
    int         path[200];
    int         length;
    double      totalTime;
    std::string steps[600];
    int         stepCount;

    TransferResult() : ok(false), length(0), totalTime(0.0), stepCount(0) {}
};

class TransferService {
public:
    TransferService(BranchManager& bm, Graph& g);

    // Simulates queue-based transfer and performs the actual product movement on success.
    // byTime=true  -> optimize path by tiempo; byTime=false -> optimize by costo
    TransferResult simulate(const std::string& barcode,
                            int originId,
                            int destId,
                            bool byTime);

private:
    BranchManager& _bm;
    Graph&         _g;

    void addStep(TransferResult& r, const std::string& step);
};

#endif // TRANSFER_SERVICE_H
