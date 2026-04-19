#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "Catalog.h"
#include <string>

/*
 * SearchMode
 * Modos de busqueda por nombre para el benchmark comparativo.
 * El PDF pide comparar: lista sin ordenar, lista ordenada y AVL.
 */
enum class SearchMode { LIST, SORTED, AVL };

/*
 * BenchmarkResult
 * Resultado de una serie de mediciones para una operacion,
 * estructura y caso especifico.
 */
struct BenchmarkResult {
    std::string operation;   // "busqueda", "insercion", "eliminacion"
    std::string structure;   // "LinkedList", "SortedList", "AVL", "BTree", "BPlus"
    std::string caseType;    // "hit_random", "miss", "hit_first", "hit_last", "general"
    double      avgTimeUs;   // promedio en microsegundos por operacion individual
    double      minTimeUs;   // minimo observado
    double      maxTimeUs;   // maximo observado
    int         N;           // consultas por repeticion
    int         M;           // repeticiones
};

/*
 * Benchmark
 * Mide y compara tiempos de busqueda, insercion y eliminacion
 * entre las estructuras del catalogo segun lo requerido en la Seccion 8
 * del proyecto.
 *
 * Seccion de busqueda (N=20, M=5):
 *   Estructura    Clave         Casos
 *   LinkedList    nombre        hit_random, miss, hit_first, hit_last
 *   SortedList    nombre        hit_random, miss, hit_first, hit_last
 *   AVL           nombre        hit_random, miss, hit_first, hit_last
 *
 * Seccion de insercion (N=20, M=5):
 *   LinkedList, SortedList, AVL, BTree, BPlus
 *   Caso: insertar un producto nuevo, medir tiempo por insercion.
 *   Nota: el producto se elimina despues de cada serie para no acumular.
 *
 * Seccion de eliminacion (N=20, M=5):
 *   LinkedList, SortedList, AVL, BTree, BPlus
 *   Caso: eliminar un producto existente, medir tiempo por eliminacion.
 *   Nota: el producto se vuelve a insertar despues para no agotar el catalogo.
 */
class Benchmark {
public:
    /*
     * Constructor
     * Precondicion: catalog inicializado con productos; N > 0; M > 0.
     */
    Benchmark(Catalog& catalog, int N = 20, int M = 5);

    /*
     * run
     * Ejecuta las tres secciones del benchmark: busqueda, insercion,
     * eliminacion. Almacena todos los resultados internamente.
     * Precondicion: catalog tiene al menos 4 productos.
     * Complejidad: O(N * M * max_costo_operacion)
     */
    void run();

    const BenchmarkResult* getResults()    const { return _results;      }
    int                    getResultCount() const { return _resultCount;   }

    /*
     * reportResults
     * Imprime las tres tablas comparativas en stdout.
     * Precondicion: run() ejecutado previamente.
     */
    void reportResults() const;

    /*
     * measureSearch
     * Mide una busqueda individual en el modo indicado.
     * Retorna tiempo en microsegundos.
     */
    double measureSearch(const std::string& key, SearchMode mode) const;

private:
    Catalog& _catalog;
    int      _N;
    int      _M;

    /**
     * Capacidad: 4 casos x 3 estructuras busqueda = 12
     *          + 1 caso  x 5 estructuras insercion = 5
     *          + 1 caso  x 5 estructuras eliminacion = 5
     *          Total = 22 resultados maximos
     */
    static const int MAX_RESULTS = 32;
    BenchmarkResult  _results[MAX_RESULTS];
    int              _resultCount;

    // Busqueda
    void runSearchBenchmark();

    
    /**
     * Mide el tiempo de insertar un producto en cada estructura
     * de forma independiente (no atomica) para aislar el costo individual.
     */
    void runInsertBenchmark();

    /**
     * Mide el tiempo de eliminar un producto de cada estructura
     * de forma independiente, reinsertandolo despues de cada serie.
     */
    void runRemoveBenchmark();

    // Claves para los casos de busqueda
    std::string randomKey()  const;
    std::string firstKey()   const;
    std::string lastKey()    const;
    std::string missKey()    const;

    // Producto temporal para pruebas de insercion/eliminacion
    Product benchProduct() const;

    // Impresion para formar tablas comparativas en reportResults()
    void printSearchTable()  const;
    void printInsertTable()  const;
    void printRemoveTable()  const;
    void printRow(const BenchmarkResult& r) const;
    void printSectionHeader(const std::string& title,const std::string& cols) const;
};

#endif