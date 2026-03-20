#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "Catalog.h"
#include <string>

/*
 * Modos de busqueda para el benchmark.
 * LIST     : busqueda secuencial en LinkedList
 * SORTED   : busqueda secuencial en SortedLinkedList
 * AVL      : busqueda binaria en AVLTree
 */
enum class SearchMode { LIST, SORTED, AVL };

/*
 * BenchmarkResult
 * Resultado de una serie de mediciones para un modo y caso especifico.
 */
struct BenchmarkResult {
    std::string  mode;        // nombre del modo ("LinkedList", "SortedList", "AVL")
    std::string  caseType;    // "hit_random", "miss", "hit_first", "hit_last"
    double       avgTimeUs;   // tiempo promedio en microsegundos
    double       minTimeUs;   // tiempo minimo observado
    double       maxTimeUs;   // tiempo maximo observado
    int          N;           // consultas por repeticion
    int          M;           // numero de repeticiones
};

/*
 * Benchmark
 * Mide y compara tiempos de busqueda entre LinkedList, SortedLinkedList
 * y AVLTree ejecutando N consultas por prueba, repetidas M veces.
 *
 * Casos de prueba:
 *   hit_random : busqueda de un elemento que existe (posicion aleatoria)
 *   miss       : busqueda de un elemento que no existe
 *   hit_first  : busqueda del primer elemento (mejor caso para lista)
 *   hit_last   : busqueda del ultimo elemento (peor caso para lista)
 *
 * Complejidades propias del Benchmark (sin contar costo de busqueda):
 *   run            : O(N * M * costo_busqueda)
 *   measureSearch  : O(N * costo_busqueda)
 *   reportResults  : O(r)  donde r = cantidad de resultados almacenados
 */
class Benchmark {
public:
    /*
     * Constructor
     * Precondicion: catalog inicializado con productos cargados; N > 0; M > 0.
     * Postcondicion: benchmark listo para ejecutarse.
     */
    Benchmark(Catalog& catalog, int N = 20, int M = 5);

    /*
     * run
     * Ejecuta todas las pruebas para los tres modos y cuatro casos.
     * Precondicion: catalog tiene al menos 1 producto.
     * Postcondicion: resultados almacenados internamente; listos para reportar.
     * Complejidad: O(N * M * costo_busqueda) por prueba.
     */
    void run();

    /*
     * measureSearch
     * Mide el tiempo promedio de N busquedas del mismo key en el modo indicado.
     * Precondicion: mode valido; key no vacio.
     * Postcondicion: retorna tiempo promedio en microsegundos.
     * Complejidad: O(N * costo_busqueda_del_modo)
     */
    double measureSearch(const std::string& key, SearchMode mode) const;

    /*
     * reportResults
     * Imprime una tabla comparativa en stdout con todos los resultados.
     * Precondicion: run() ejecutado previamente.
     * Postcondicion: tabla formateada impresa en stdout.
     * Complejidad: O(r)
     */
    void reportResults() const;

private:
    Catalog& _catalog;           // referencia al catalogo a medir
    int      _N;                 // consultas por repeticion
    int      _M;                 // numero de repeticiones
    BenchmarkResult _results[12]; // 3 modos x 4 casos
    int      _resultCount;       // cantidad de resultados almacenados

    // Selecciona una clave aleatoria de los productos del catalogo
    std::string randomKey() const;

    // Selecciona la clave del primer producto en la lista
    std::string firstKey() const;

    // Selecciona la clave del ultimo producto en la lista
    std::string lastKey() const;

    // Retorna una clave que no existe en el catalogo
    std::string missKey() const;

    // Imprime una fila de la tabla de resultados
    void printRow(const BenchmarkResult& r) const;

    // Imprime el encabezado de la tabla
    void printHeader() const;
};

#endif