#include "BranchManager.h"
#include <fstream>
#include <sstream>

BranchManager::BranchManager() : _branchCount(0) {
    for (int i = 0; i < 100; ++i) _branches[i] = nullptr;
}

BranchManager::~BranchManager() {
    for (int i = 0; i < _branchCount; ++i) {
        delete _branches[i];
        _branches[i] = nullptr;
    }
}

bool BranchManager::addBranch(const Branch& b) {
    if (_branchCount >= 100) return false;
    _branches[_branchCount++] = new Branch(b.id, b.name, b.location, b.t_ingreso, b.t_traspaso, b.t_despacho);
    _graph.addBranch(b.id);
    return true;
}

Branch* BranchManager::findBranch(int id) const {
    for (int i = 0; i < _branchCount; ++i)
        if (_branches[i]->id == id) return _branches[i];
    return nullptr;
}

void BranchManager::listBranches() const {
    for (int i = 0; i < _branchCount; ++i)
        printf("Branch %d: %s (%s)\n",
               _branches[i]->id,
               _branches[i]->name.c_str(),
               _branches[i]->location.c_str());
}

void BranchManager::addConnection(int orig, int dest, double tiempo, double costo, bool bidirectional) {
    _graph.addEdge(orig, dest, tiempo, costo, bidirectional);
}

bool BranchManager::transferProduct(const std::string& barcode, int originId, int destId, bool byTime) {
    Branch* origin = findBranch(originId);
    Branch* dest   = findBranch(destId);
    if (!origin || !dest) return false;

    Product* p = origin->findByBarcode(barcode);
    if (!p) return false;

    if (!origin->removeProduct(barcode)) return false;
    if (!dest->addProduct(*p)) {
        origin->addProduct(*p); // rollback
        return false;
    }

    int path[200];
    int pathLen = 0;
    if (byTime)
        _graph.dijkstraByTime(originId, destId, path, pathLen);
    else
        _graph.dijkstraByCost(originId, destId, path, pathLen);

    (void)path; (void)pathLen;
    return true;
}

bool BranchManager::loadBranchesCSV(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return false;
    std::string line;
    std::getline(f, line); // header
    while (std::getline(f, line)) {
        std::istringstream ss(line);
        std::string token;
        int id, ti, tt, td;
        std::string name, loc;
        std::getline(ss, token, ','); id = std::stoi(token);
        std::getline(ss, name,  ',');
        std::getline(ss, loc,   ',');
        std::getline(ss, token, ','); ti = std::stoi(token);
        std::getline(ss, token, ','); tt = std::stoi(token);
        std::getline(ss, token, ','); td = std::stoi(token);
        addBranch(Branch(id, name, loc, ti, tt, td));
    }
    return true;
}

bool BranchManager::loadConnectionsCSV(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return false;
    std::string line;
    std::getline(f, line);
    while (std::getline(f, line)) {
        std::istringstream ss(line);
        std::string token;
        int orig, dest;
        double tiempo, costo;
        bool bidi;
        std::getline(ss, token, ','); orig   = std::stoi(token);
        std::getline(ss, token, ','); dest   = std::stoi(token);
        std::getline(ss, token, ','); tiempo = std::stod(token);
        std::getline(ss, token, ','); costo  = std::stod(token);
        std::getline(ss, token, ','); bidi   = (token == "1" || token == "true");
        addConnection(orig, dest, tiempo, costo, bidi);
    }
    return true;
}

bool BranchManager::loadProductsCSV(const std::string& path) {
    (void)path;
    return false; // delegar en cada Branch según requerimiento
}

Graph& BranchManager::getGraph() { return _graph; }
