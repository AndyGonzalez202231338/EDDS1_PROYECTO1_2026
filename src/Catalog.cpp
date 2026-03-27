#include "Catalog.h"
#include "CSVLoader.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>

Catalog::Catalog(const std::string& logPath): _logger(logPath) {}

Catalog::~Catalog() {}

bool Catalog::addProduct(const Product& p) {
    if (!p.isValid()) {
        _logger.logError("addProduct: producto invalido barcode=" + p.barcode);
        return false;
    }
    // Verificar duplicado por barcode en la lista
    if (_list.searchSequential(p.barcode) != nullptr) {
        _logger.logDuplicate(p.barcode);
        return false;
    }
    // Si alguna falla, rollback de las anteriores.
    bool inList    = false;
    bool inSorted  = false;
    bool inAVL     = false;
    bool inBTree   = false;
    bool inBPlus   = false;
    try {
        _list.insertFront(p); inList   = true;
        _sortedList.insertSorted(p); inSorted = true;
        _avl.insert(p); inAVL    = true;
        _btree.insert(p); inBTree  = true;
        _bplus.insert(p); inBPlus  = true;
    } catch (...) {
        _logger.logError("addProduct: excepcion al insertar barcode=" + p.barcode);
        rollback(p, inList, inSorted, inAVL, inBTree, inBPlus);
        return false;
    }

    return true;
}

// eliminacion en las 5 estructuras
bool Catalog::removeProduct(const std::string& barcode) {
    // Verificar que el producto existe antes de intentar eliminar
    Product* existing = _list.searchSequential(barcode);
    if (existing == nullptr) return false;

    // Guardar nombre y datos antes de eliminar para propagar a las demas estructuras
    std::string name     = existing->name;
    std::string date     = existing->expiry_date;
    std::string category = existing->category;

    _list.remove(barcode);
    _sortedList.remove(name);
    _avl.remove(name);
    _btree.remove(date);
    _bplus.remove(category);

    return true;
}

bool Catalog::loadFromCSV(const std::string& path) {
    return CSVLoader::load(path, *this, _logger);
}

Product* Catalog::findByName(const std::string& name) {
    return _avl.search(name);
}

Product* Catalog::findByBarcode(const std::string& barcode) {
    return _list.searchSequential(barcode);
}

void Catalog::findByCategory(const std::string& category, Product** results, int& count) const {
    _bplus.searchCategory(category, results, count, MAX_RESULTS);
}

void Catalog::findByDateRange(const std::string& d1, const std::string& d2, Product** results, int& count) const {
    _btree.rangeSearch(d1, d2, results, count, MAX_RESULTS);
}

// listAllByName, recorrido inorder del AVL
void Catalog::listAllByName() const {
    if (_avl.isEmpty()) {
        std::cout << "El catalogo esta vacio.\n";
        return;
    }

    int count = 0;
    _avl.inorder([&count](const Product& p) {
        std::cout << "----------------------------\n";
        std::cout << p.toString() << "\n";
        ++count;
    });
    std::cout << "----------------------------\n";
    std::cout << "Total: " << count << " productos.\n";
}

// benchmarkSearch
void Catalog::benchmarkSearch(int N, int M) const {
    if (_list.isEmpty()) {
        std::cout << "El catalogo esta vacio, no se puede medir.\n";
        return;
    }

    // Obtener una clave existente (primer elemento de la lista)
    const ListNode* head = _list.getHead();
    std::string existingKey = head->data.name;
    std::string missingKey  = "ZZZZZ_NO_EXISTE";

    // Tipos de busqueda a medir
    struct Caso { std::string label; std::string key; };
    Caso casos[] = {
        { "hit_aleatorio", existingKey },
        { "miss          ", missingKey  }
    };

    std::cout << "\n=== Benchmark de busqueda (N=" << N << ", M=" << M << ") ===\n";
    std::cout << std::left
              << std::setw(16) << "Caso"
              << std::setw(18) << "LinkedList (us)"
              << std::setw(18) << "SortedList (us)"
              << std::setw(18) << "AVL (us)"
              << "\n";
    std::cout << std::string(70, '-') << "\n";
    for (auto& caso : casos) {
        double totalList   = 0;
        double totalSorted = 0;
        double totalAVL    = 0;

        for (int m = 0; m < M; ++m) {
            // LinkedList
            auto t0 = std::chrono::high_resolution_clock::now();
            for (int n = 0; n < N; ++n)
                const_cast<LinkedList&>(_list).searchByName(caso.key);
            auto t1 = std::chrono::high_resolution_clock::now();
            totalList += std::chrono::duration<double, std::micro>(t1 - t0).count();

            // SortedLinkedList
            t0 = std::chrono::high_resolution_clock::now();
            for (int n = 0; n < N; ++n)
                const_cast<SortedLinkedList&>(_sortedList).searchSequential(caso.key);
            t1 = std::chrono::high_resolution_clock::now();
            totalSorted += std::chrono::duration<double, std::micro>(t1 - t0).count();

            // AVL
            t0 = std::chrono::high_resolution_clock::now();
            for (int n = 0; n < N; ++n)
                const_cast<AVLTree&>(_avl).search(caso.key);
            t1 = std::chrono::high_resolution_clock::now();
            totalAVL += std::chrono::duration<double, std::micro>(t1 - t0).count();
        }

        double avgList   = totalList   / (M * N);
        double avgSorted = totalSorted / (M * N);
        double avgAVL    = totalAVL    / (M * N);

        std::cout << std::left  << std::setw(16) << caso.label
                  << std::fixed << std::setprecision(4)
                  << std::setw(18) << avgList
                  << std::setw(18) << avgSorted
                  << std::setw(18) << avgAVL
                  << "\n";
    }
    std::cout << std::string(70, '-') << "\n";
}

void Catalog::generateDotFiles() const {
    // AVL
    {
        std::ofstream out("output/avl.dot");
        if (out.is_open()) {
            _avl.toDot(out);
            std::cout << "Generado: output/avl.dot\n";
        } else {
            std::cerr << "No se pudo crear output/avl.dot\n";
        }
    }
    // BTree
    {
        std::ofstream out("output/btree.dot");
        if (out.is_open()) {
            _btree.toDot(out);
            std::cout << "Generado: output/btree.dot\n";
        } else {
            std::cerr << "No se pudo crear output/btree.dot\n";
        }
    }
    // BPlusTree
    {
        std::ofstream out("output/bplustree.dot");
        if (out.is_open()) {
            _bplus.toDot(out);
            std::cout << "Generado: output/bplustree.dot\n";
        } else {
            std::cerr << "No se pudo crear output/bplustree.dot\n";
        }
    }
}

// Acceso a estructuras internas
LinkedList&       Catalog::getList()       { return _list;       }
SortedLinkedList& Catalog::getSortedList() { return _sortedList; }
AVLTree&          Catalog::getAVL()        { return _avl;        }
Logger&           Catalog::getLogger()     { return _logger;     }

// rollback (privado)
void Catalog::rollback(const Product& p, bool insertedList, bool insertedSorted, bool insertedAVL, bool insertedBTree, bool insertedBPlus) {
    if (insertedBPlus)  _bplus.remove(p.category);
    if (insertedBTree)  _btree.remove(p.expiry_date);
    if (insertedAVL)    _avl.remove(p.name);
    if (insertedSorted) _sortedList.remove(p.name);
    if (insertedList)   _list.remove(p.barcode);
}