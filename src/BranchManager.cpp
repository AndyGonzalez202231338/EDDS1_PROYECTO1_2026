#include "BranchManager.h"

BranchManager::BranchManager() : _head(nullptr), _size(0) {}

BranchManager::~BranchManager() {
    clear();
}

bool BranchManager::addBranch(int id,const std::string& nombre, const std::string& ubicacion, int tiempoIngreso, int tiempoPreparacion, int intervaloDespacho) {
    if (findBranch(id) != nullptr) return false; // id duplicado

    Branch* b = new Branch(id, nombre, ubicacion, tiempoIngreso, tiempoPreparacion, intervaloDespacho);
    Node* n = new Node(b);
    n->next = _head;
    _head = n;
    ++_size;
    return true;
}

bool BranchManager::removeBranch(int id) {
    Node* curr = _head;
    Node* prev = nullptr;

    while (curr != nullptr) {
        if (curr->branch->getId() == id) {
            if (prev == nullptr) _head = curr->next;
            else prev->next = curr->next;

            delete curr->branch;
            delete curr;
            --_size;
            return true;
        }
        prev = curr;
        curr = curr->next;
    }
    return false;
}

Branch* BranchManager::findBranch(int id) {
    Node* curr = _head;
    while (curr != nullptr) {
        if (curr->branch->getId() == id) return curr->branch;
        curr = curr->next;
    }
    return nullptr;
}

const Branch* BranchManager::findBranch(int id) const {
    Node* curr = _head;
    while (curr != nullptr) {
        if (curr->branch->getId() == id) return curr->branch;
        curr = curr->next;
    }
    return nullptr;
}

int BranchManager::size() const {
    return _size;
}

bool BranchManager::isEmpty() const {
    return _size == 0;
}

void BranchManager::clear() {
    Node* curr = _head;
    while (curr != nullptr) {
        Node* next = curr->next;
        delete curr->branch;
        delete curr;
        curr = next;
    }
    _head = nullptr;
    _size = 0;
}

void BranchManager::forEach(const std::function<void(const Branch&)>& fn) const {
    Node* curr = _head;
    while (curr != nullptr) {
        fn(*curr->branch);
        curr = curr->next;
    }
}

bool BranchManager::transferProduct(const std::string& barcode, int originId, int destId, bool /*byTime*/) {
    Branch* origin = findBranch(originId);
    Branch* dest   = findBranch(destId);
    if (!origin || !dest || originId == destId) return false;

    Product* p = origin->searchByBarcode(barcode);
    if (!p) return false;

    Product copy = *p;
    copy.branchId = destId;

    if (!dest->insertProduct(copy)) return false;   // duplicado o error en destino
    if (!origin->removeProduct(barcode)) {
        dest->removeProduct(barcode);               // rollback entre sucursales
        return false;
    }
    return true;
}
