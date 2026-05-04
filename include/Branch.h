#ifndef BRANCH_H
#define BRANCH_H

#include <string>
#include <functional>
#include <mutex>
#include "Product.h"
#include "LinkedList.h"
#include "SortedLinkedList.h"
#include "AVLTree.h"
#include "BTree.h"
#include "BPlusTree.h"
#include "HashTable.h"
#include "Queue.h"

class Branch {
public:
    Queue<Product> queueIngreso;
    Queue<Product> queuePreparacion;
    Queue<Product> queueSalida;

    Branch();
    Branch(int id,
           const std::string& nombre,
           const std::string& ubicacion,
           int tiempoIngreso,
           int tiempoPreparacion,
           int intervaloDespacho);

    // Metadatos
    int getId() const;
    const std::string& getNombre() const;
    const std::string& getUbicacion() const;
    int getTiempoIngreso() const;
    int getTiempoPreparacion() const;
    int getIntervaloDespacho() const;

    // Inventario (atómico + rollback)
    bool insertProduct(const Product& p);
    bool removeProduct(const std::string& barcode);

    // Búsquedas
    Product* searchByBarcode(const std::string& barcode);
    Product* searchByName(const std::string& name);
    void searchByCategory(const std::string& category, Product** results, int& count, int maxResults = 1000) const;
    void searchByDateRange(const std::string& d1, const std::string& d2, Product** results, int& count, int maxResults = 1000) const;

    bool isInventoryEmpty() const;
    void getProducts(const std::function<void(const Product&)>& fn) const;

    // Raw structure access for BranchBenchmark (no mutex, single-threaded use only)
    LinkedList&             rawList()       { return _list; }
    const LinkedList&       rawList()       const { return _list; }
    SortedLinkedList&       rawSortedList() { return _sortedList; }
    const SortedLinkedList& rawSortedList() const { return _sortedList; }
    AVLTree&                rawAVL()        { return _avl; }
    BTree&                  rawBTree()      { return _btree; }
    BPlusTree&              rawBPlus()      { return _bplus; }
    HashTable&              rawHash()       { return _hash; }

    // Thread-safe queue wrappers (usados por TransferService y RealtimeTransfer)
    void enqueueIngreso(const Product& p);
    void dequeueIngreso();
    void enqueuePreparacion(const Product& p);
    void dequeuePreparacion();
    void enqueueSalida(const Product& p);
    void dequeueSalida();

private:
    mutable std::mutex _mtx;
    int _id;
    std::string _nombre;
    std::string _ubicacion;
    int _tiempoIngreso;
    int _tiempoPreparacion;
    int _intervaloDespacho;

    // Inventario propio por sucursal (no compartido)
    LinkedList       _list;
    SortedLinkedList _sortedList;
    AVLTree          _avl;
    BTree            _btree;
    BPlusTree        _bplus;
    HashTable        _hash;

    void rollbackInsert(const Product& p,
                        bool inList, bool inSorted, bool inAVL,
                        bool inBTree, bool inBPlus, bool inHash);
};

#endif