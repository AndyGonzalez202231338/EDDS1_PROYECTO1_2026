#ifndef BRANCHMANAGER_H
#define BRANCHMANAGER_H

#include "Branch.h"
#include "Graph.h"

class BranchManager {
public:
    BranchManager();
    ~BranchManager();

    // Sucursales
    bool    addBranch(const Branch& b);
    Branch* findBranch(int id) const;
    void    listBranches() const;

    // Red
    void addConnection(int orig, int dest, double tiempo, double costo, bool bidirectional);

    // Transferencia
    bool transferProduct(const std::string& barcode, int originId, int destId, bool byTime);

    // Carga CSV
    bool loadBranchesCSV(const std::string& path);
    bool loadConnectionsCSV(const std::string& path);
    bool loadProductsCSV(const std::string& path);

    Graph& getGraph();

private:
    Branch* _branches[100];  // arreglo estático, máx 100 sucursales
    int     _branchCount;
    Graph   _graph;
};

#endif // BRANCHMANAGER_H