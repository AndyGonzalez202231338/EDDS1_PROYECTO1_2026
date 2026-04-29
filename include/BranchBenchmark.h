#ifndef BRANCH_BENCHMARK_H
#define BRANCH_BENCHMARK_H

#include "Branch.h"
#include "Benchmark.h"  // reuse BenchmarkResult

/*
 * BranchBenchmark
 * Ejecuta el mismo benchmark comparativo que Benchmark, pero sobre las
 * estructuras internas de una sucursal individual en lugar del catálogo global.
 *
 * Búsqueda: LinkedList, SortedList, AVL (por nombre) + HashTable (por barcode)
 * Inserción: LinkedList, SortedList, AVL, BTree, BPlus, HashTable
 * Eliminación: LinkedList, SortedList, AVL, BTree, BPlus, HashTable
 */
class BranchBenchmark {
public:
    BranchBenchmark(Branch& branch, int N = 20, int M = 5);

    void run();

    const BenchmarkResult* getResults()     const { return _results;      }
    int                    getResultCount() const { return _resultCount;   }

private:
    Branch& _branch;
    int     _N, _M;
    static const int MAX_RESULTS = 48;
    BenchmarkResult  _results[MAX_RESULTS];
    int              _resultCount;

    void runSearchBenchmark();
    void runInsertBenchmark();
    void runRemoveBenchmark();

    std::string randomName()    const;
    std::string firstSortName() const;
    std::string lastListName()  const;
    std::string missName()      const;
    std::string randomBarcode() const;

    Product benchProduct() const;
};

#endif // BRANCH_BENCHMARK_H
