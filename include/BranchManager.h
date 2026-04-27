#ifndef BRANCH_MANAGER_H
#define BRANCH_MANAGER_H

#include <string>
#include <functional> // <-- agregar
#include "Branch.h"

class BranchManager {
public:
    BranchManager();
    ~BranchManager();

    BranchManager(const BranchManager&) = delete;
    BranchManager& operator=(const BranchManager&) = delete;

    bool addBranch(int id,
                   const std::string& nombre,
                   const std::string& ubicacion,
                   int tiempoIngreso,
                   int tiempoPreparacion,
                   int intervaloDespacho);

    bool removeBranch(int id);
    Branch* findBranch(int id);
    const Branch* findBranch(int id) const;

    int size() const;
    bool isEmpty() const;
    void clear();

    bool transferProduct(const std::string& barcode, int originId, int destId, bool byTime);
    void forEach(const std::function<void(const Branch&)>& fn) const;

private:
    struct Node {
        Branch* branch;
        Node* next;
        explicit Node(Branch* b) : branch(b), next(nullptr) {}
    };

    Node* _head;
    int _size;
};

#endif