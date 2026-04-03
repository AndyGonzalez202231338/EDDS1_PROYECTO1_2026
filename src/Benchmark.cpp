#include "Benchmark.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>
#include <sstream>
#include <vector>

#define RST     "\033[0m"
#define BOLD    "\033[1m"
#define CRED    "\033[91m"
#define CGRN    "\033[92m"
#define CYEL    "\033[93m"
#define CBLU    "\033[94m"
#define CMAG    "\033[95m"
#define CCYN    "\033[96m"
#define CWHT    "\033[97m"
#define BYEL    "\033[43m"

using Clock = std::chrono::high_resolution_clock;
using Us    = std::chrono::duration<double, std::micro>;

Benchmark::Benchmark(Catalog& catalog, int N, int M)
    : _catalog(catalog), _N(N), _M(M), _resultCount(0) {}


void Benchmark::run() {
    if (_catalog.getList().isEmpty()) {
        std::cout << "  Catalogo vacio, carga al menos un producto primero.\n";
        return;
    }
    _resultCount = 0;
    runSearchBenchmark();
    runInsertBenchmark();
    runRemoveBenchmark();
}

/**
 * runSearchBenchmark
 * Mide el tiempo de busqueda por nombre en LinkedList, SortedLinkedList y AVL.
 * Se prueban 4 casos: hit random, miss, hit first, hit last.
 * Cada caso se ejecuta M veces con N consultas repetidas, y se promedia el resultado.
 * Los resultados se almacenan en _results para su posterior impresion en reportResults(). 
 */ 
void Benchmark::runSearchBenchmark() {
    struct Caso { const char* label; std::string key; };
    Caso casos[] = {
        { "hit_random", randomKey() },
        { "miss       ", missKey()  },
        { "hit_first  ", firstKey() },
        { "hit_last   ", lastKey()  }
    };

    struct Modo { SearchMode mode; const char* name; };
    Modo modos[] = {
        { SearchMode::LIST,   "LinkedList" },
        { SearchMode::SORTED, "SortedList" },
        { SearchMode::AVL,    "AVL"        }
    };

    for (auto& caso : casos) {
        for (auto& modo : modos) {
            double total = 0.0, minT = 1e18, maxT = 0.0;

            for (int rep = 0; rep < _M; ++rep) {
                auto t0 = Clock::now();
                for (int n = 0; n < _N; ++n)
                    measureSearch(caso.key, modo.mode);
                auto t1 = Clock::now();
                double elapsed = Us(t1 - t0).count();
                total += elapsed;
                if (elapsed < minT) minT = elapsed;
                if (elapsed > maxT) maxT = elapsed;
            }

            BenchmarkResult& r = _results[_resultCount++];
            r.operation = "busqueda";
            r.structure = modo.name;
            r.caseType  = caso.label;
            r.avgTimeUs = total / (_M * _N);
            r.minTimeUs = minT  / _N;
            r.maxTimeUs = maxT  / _N;
            r.N = _N; r.M = _M;
        }
    }
}

/**
 * runInsertBenchmark
 * Mide el costo de insertar en cada estructura de forma independiente.
 * Para aislar el costo individual se inserta directamente en la
 * estructura, no via Catalog::addProduct (que inserta en todas).
 * Se usa un producto temporal con barcode unico de benchmark.
 * Al finalizar cada serie se elimina el producto del catalogo completo.
 * Complejidad: O(M * N * costo_insercion)
 */
void Benchmark::runInsertBenchmark() {
    Product bp = benchProduct();

    struct Estructura { const char* name; };
    Estructura estructuras[] = {
        { "LinkedList" },
        { "SortedList" },
        { "AVL"        },
        { "BTree"      },
        { "BPlus"      }
    };

    for (auto& est : estructuras) {
        double total = 0.0, minT = 1e18, maxT = 0.0;
        std::string name(est.name);

        for (int rep = 0; rep < _M; ++rep) {
            double elapsed = 0.0;

            for (int n = 0; n < _N; ++n) {
                // Crear variante unica para no violar unicidad de barcode
                std::string varBC = bp.barcode + "_" + std::to_string(rep * _N + n);
                Product var = bp;
                var.barcode = varBC;
                var.name    = bp.name + std::to_string(rep * _N + n);

                auto t0 = Clock::now();

                     if (name == "LinkedList") _catalog.getList().insertFront(var);
                else if (name == "SortedList") _catalog.getSortedList().insertSorted(var);
                else if (name == "AVL")        _catalog.getAVL().insert(var);
                else if (name == "BTree")      _catalog.getBTree().insert(var);
                else if (name == "BPlus")      _catalog.getBPlus().insert(var);

                auto t1 = Clock::now();
                elapsed += Us(t1 - t0).count();

                // Limpiar inmediatamente para no acumular
                if      (name == "LinkedList") _catalog.getList().remove(varBC);
                else if (name == "SortedList") _catalog.getSortedList().remove(var.name);
                else if (name == "AVL")        _catalog.getAVL().remove(var.name);
                else if (name == "BTree")      _catalog.getBTree().remove(var.expiry_date);
                else if (name == "BPlus")      _catalog.getBPlus().remove(var.category);
            }

            total += elapsed;
            if (elapsed < minT) minT = elapsed;
            if (elapsed > maxT) maxT = elapsed;
        }

        BenchmarkResult& r = _results[_resultCount++];
        r.operation = "insercion";
        r.structure = est.name;
        r.caseType  = "general  ";
        r.avgTimeUs = total / (_M * _N);
        r.minTimeUs = minT  / _N;
        r.maxTimeUs = maxT  / _N;
        r.N = _N; r.M = _M;
    }
}

/**
 * runRemoveBenchmark
 * Mide el costo de eliminar de cada estructura de forma independiente.
 * Se toma el primer producto de la lista como sujeto de prueba.
 * Se inserta de vuelta en la estructura despues de cada eliminacion.
 */
void Benchmark::runRemoveBenchmark() {
    // Obtener un producto real del catalogo como sujeto
    const ListNode* head = _catalog.getList().getHead();
    if (!head) return;
    Product subj = head->data;
    struct Estructura { const char* name; };
    Estructura estructuras[] = {
        { "LinkedList" },
        { "SortedList" },
        { "AVL"        },
        { "BTree"      },
        { "BPlus"      }
    };

    for (auto& est : estructuras) {
        double total = 0.0, minT = 1e18, maxT = 0.0;
        std::string name(est.name);

        for (int rep = 0; rep < _M; ++rep) {
            double elapsed = 0.0;

            for (int n = 0; n < _N; ++n) {
                auto t0 = Clock::now();

                if      (name == "LinkedList") _catalog.getList().remove(subj.barcode);
                else if (name == "SortedList") _catalog.getSortedList().remove(subj.name);
                else if (name == "AVL")        _catalog.getAVL().remove(subj.name);
                else if (name == "BTree")      _catalog.getBTree().remove(subj.expiry_date);
                else if (name == "BPlus")      _catalog.getBPlus().remove(subj.category);

                auto t1 = Clock::now();
                elapsed += Us(t1 - t0).count();

                // Reinsertar para la siguiente iteracion
                if      (name == "LinkedList") _catalog.getList().insertFront(subj);
                else if (name == "SortedList") _catalog.getSortedList().insertSorted(subj);
                else if (name == "AVL")        _catalog.getAVL().insert(subj);
                else if (name == "BTree")      _catalog.getBTree().insert(subj);
                else if (name == "BPlus")      _catalog.getBPlus().insert(subj);
            }

            total += elapsed;
            if (elapsed < minT) minT = elapsed;
            if (elapsed > maxT) maxT = elapsed;
        }

        BenchmarkResult& r = _results[_resultCount++];
        r.operation = "eliminacion";
        r.structure = est.name;
        r.caseType  = "general    ";
        r.avgTimeUs = total / (_M * _N);
        r.minTimeUs = minT  / _N;
        r.maxTimeUs = maxT  / _N;
        r.N = _N; r.M = _M;
    }
}

/**
 * measureSearch
 * Mide el tiempo de una busqueda individual en el modo indicado.
 * Retorna tiempo en microsegundos.
 * Para busqueda por nombre: LinkedList, SortedList, AVL.
 */  
double Benchmark::measureSearch(const std::string& key, SearchMode mode) const {
    auto t0 = Clock::now();
    switch (mode) {
        case SearchMode::LIST:
            _catalog.getList().searchByName(key);
            break;
        case SearchMode::SORTED:
            _catalog.getSortedList().searchSequential(key);
            break;
        case SearchMode::AVL:
            _catalog.getAVL().search(key);
            break;
    }
    auto t1 = Clock::now();
    return Us(t1 - t0).count();
}

void Benchmark::reportResults() const {
    if (_resultCount == 0) {
        std::cout << CRED <<"  Sin resultados. Ejecuta run() primero.\n"<< RST;
        return;
    }
    printSearchTable();
    printInsertTable();
    printRemoveTable();

    std::cout << CMAG << "\n  N=" << _N << " operaciones por repeticion, "
              << "M=" << _M << " repeticiones.\n"
              << "  Tiempos en microsegundos (us) por operacion individual.\n" << RST;
}

void Benchmark::printSectionHeader(const std::string& title, const std::string& cols) const {
    std::cout << CYEL << "\n" << std::string(72, '=') << "\n" << RST;
    std::cout << BYEL << "  " << title << RST << "\n";
    std::cout << CYEL << std::string(72, '=') << RST << "\n";
    std::cout << CYEL << cols << RST << "\n";
    std::cout << CYEL << std::string(72, '-') << RST << "\n";
}

void Benchmark::printRow(const BenchmarkResult& r) const {
    std::cout << std::left << std::fixed << std::setprecision(4)
    << CGRN << "  " << std::setw(13) << r.structure << RST
    << "  " << std::setw(13) << r.caseType
    << "  " << std::setw(12) << r.avgTimeUs
    << "  " << std::setw(12) << r.minTimeUs
    << "  " << std::setw(12) << r.maxTimeUs
    << "\n";
}

void Benchmark::printSearchTable() const {
    printSectionHeader(
        "BUSQUEDA POR NOMBRE  (Lista vs SortedList vs AVL)                      ",
        "  Estructura      Caso           Avg (us)      Min (us)      Max (us)"
    );
    const char* casos[] = { "hit_random", "miss       ", "hit_first  ", "hit_last   " };
    for (auto& caso : casos) {
        std::cout << CCYN << "\n  -- Caso: " << caso << " --\n" << RST;
        for (int i = 0; i < _resultCount; ++i) {
            if (_results[i].operation == "busqueda" && _results[i].caseType  == caso) {
                printRow(_results[i]);
            }
        }
    }
}

void Benchmark::printInsertTable() const {
    printSectionHeader(
        "INSERCION  (costo individual por estructura)                           ",
        "  Estructura      Caso           Avg (us)      Min (us)      Max (us)"
    );
    for (int i = 0; i < _resultCount; ++i) {
        if (_results[i].operation == "insercion") {
            printRow(_results[i]);
        }
    }
}

void Benchmark::printRemoveTable() const {
    printSectionHeader(
        "ELIMINACION  (costo individual por estructura)                         ",
        "  Estructura      Caso           Avg (us)      Min (us)      Max (us)"
    );
    for (int i = 0; i < _resultCount; ++i) {
        if (_results[i].operation == "eliminacion") {
            printRow(_results[i]);
        }
    }
}

std::string Benchmark::randomKey() const {
    const ListNode* node = _catalog.getList().getHead();
    return node ? node->data.name : "";
}

std::string Benchmark::firstKey() const {
    const SortedListNode* node = _catalog.getSortedList().getHead();
    return node ? node->data.name : "";
}

std::string Benchmark::lastKey() const {
    const ListNode* node = _catalog.getList().getHead();
    const ListNode* last = node;
    while (node) { last = node; node = node->next; }
    return last ? last->data.name : "";
}

std::string Benchmark::missKey() const {
    return "ZZZZZ_NO_EXISTE_9999";
}

Product Benchmark::benchProduct() const {
    return Product("BenchProd", "BENCH_BC_000", "Benchmark", "2099-01-01", "BenchMarca", 1.0, 1);
}