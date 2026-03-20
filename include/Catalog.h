#ifndef CATALOG_H
#define CATALOG_H

#include "Product.h"
#include "LinkedList.h"
#include "SortedLinkedList.h"
#include "AVLTree.h"
#include "BTree.h"
#include "BPlusTree.h"
#include "Logger.h"
#include <string>
#include <fstream>

// Tamano maximo de resultados para operaciones de busqueda multiple
static const int MAX_RESULTS = 2048;

/*
 * Catalog
 * Controlador principal del sistema.
 * Mantiene todas las estructuras de datos sincronizadas:
 *   - LinkedList      : lista no ordenada (busqueda secuencial / benchmark)
 *   - SortedLinkedList: lista ordenada por nombre (benchmark comparativo)
 *   - AVLTree         : ordenado por nombre (busqueda binaria O(log n))
 *   - BTree           : ordenado por fecha de caducidad (rangos)
 *   - BPlusTree       : ordenado por categoria (recorrido de hojas)
 *
 * Toda insercion y eliminacion es atomica: si falla en alguna estructura,
 * se hace rollback en las que ya se insertaron y se registra el error.
 */
class Catalog {
public:
    /*
     * Constructor
     * Precondicion: logPath es una ruta valida con permisos de escritura.
     * Postcondicion: todas las estructuras inicializadas vacias; logger listo.
     */
    explicit Catalog(const std::string& logPath = "errors.log");

    /*
     * Destructor
     * Postcondicion: todas las estructuras liberadas; logger cerrado.
     */
    ~Catalog();

    // Prohibir copia
    Catalog(const Catalog&)            = delete;
    Catalog& operator=(const Catalog&) = delete;

    /*
     * addProduct
     * Inserta p en todas las estructuras de forma atomica.
     * Si falla en alguna, hace rollback en las anteriores y loggea el error.
     * Precondicion: p.isValid() == true.
     * Postcondicion: p insertado en las 5 estructuras, o en ninguna si hubo fallo.
     * Retorna: true si la insercion fue exitosa en todas.
     * Complejidad: O(log n) dominada por AVL y arboles B.
     */
    bool addProduct(const Product& p);

    /*
     * removeProduct
     * Elimina el producto con ese barcode de todas las estructuras.
     * Precondicion: barcode puede o no existir.
     * Postcondicion: producto eliminado de las 5 estructuras si existia.
     * Retorna: true si el producto existia y fue eliminado.
     * Complejidad: O(log n)
     */
    bool removeProduct(const std::string& barcode);

    /*
     * loadFromCSV
     * Delega en CSVLoader para cargar productos desde un archivo.
     * Precondicion: path apunta a un archivo CSV con el formato esperado.
     * Postcondicion: productos validos insertados; errores en errors.log.
     * Retorna: true si el archivo pudo abrirse.
     * Complejidad: O(n * log n) donde n = lineas validas del CSV.
     */
    bool loadFromCSV(const std::string& path);

    /*
     * findByName
     * Busqueda binaria en AVLTree.
     * Precondicion: name no vacio.
     * Postcondicion: retorna puntero al Product o nullptr.
     * Complejidad: O(log n)
     */
    Product* findByName(const std::string& name);

    /*
     * findByBarcode
     * Busqueda secuencial en LinkedList (no hay tabla hash).
     * Precondicion: barcode no vacio.
     * Postcondicion: retorna puntero al Product o nullptr.
     * Complejidad: O(n)
     */
    Product* findByBarcode(const std::string& barcode);

    /*
     * findByCategory
     * Busqueda en BPlusTree usando recorrido de hojas enlazadas.
     * Precondicion: category no vacio; results tiene capacidad MAX_RESULTS.
     * Postcondicion: results contiene punteros a productos de la categoria;
     *                count indica cuantos se encontraron.
     * Complejidad: O(log n + k)
     */
    void findByCategory(const std::string& category, Product** results, int& count) const;

    /*
     * findByDateRange
     * Busqueda por rango en BTree.
     * Precondicion: d1 <= d2; ambos formato "YYYY-MM-DD".
     * Postcondicion: results contiene punteros a productos en el rango.
     * Complejidad: O(log n + k)
     */
    void findByDateRange(const std::string& d1, const std::string& d2, Product** results, int& count) const;

    /*
     * listAllByName
     * Recorrido inorder del AVLTree: imprime todos los productos en orden
     * alfabetico por nombre.
     * Precondicion: ninguna.
     * Postcondicion: productos impresos en stdout.
     * Complejidad: O(n)
     */
    void listAllByName() const;

    /*
     * benchmarkSearch
     * Mide y compara tiempos de busqueda entre LinkedList, SortedLinkedList
     * y AVLTree para N consultas aleatorias, repetidas M veces.
     * Precondicion: N > 0; M > 0; catalogo con productos cargados.
     * Postcondicion: tabla comparativa impresa en stdout con tiempos en us.
     * Complejidad: O(N * M * costo_busqueda)
     */
    void benchmarkSearch(int N = 20, int M = 5) const;

    /*
     * generateDotFiles
     * Genera archivos .dot para AVLTree, BTree y BPlusTree en el directorio output/.
     * Precondicion: directorio output/ existe o puede crearse.
     * Postcondicion: archivos output/avl.dot, output/btree.dot,
     *                output/bplustree.dot creados o sobreescritos.
     * Complejidad: O(n)
     */
    void generateDotFiles() const;

    LinkedList&       getList();
    SortedLinkedList& getSortedList();
    AVLTree&          getAVL();
    Logger&           getLogger();

private:
    LinkedList       _list;        // lista no ordenada
    SortedLinkedList _sortedList;  // lista ordenada por nombre
    AVLTree          _avl;         // arbol AVL por nombre
    BTree            _btree;       // arbol B por fecha de caducidad
    BPlusTree        _bplus;       // arbol B+ por categoria
    Logger           _logger;      // registro de errores

    // Rollback: elimina p de las estructuras donde ya fue insertado.
    // insertedList, insertedSorted, insertedAVL, insertedBTree, insertedBPlus
    // indican cuales ya procesaron la insercion.
    void rollback(const Product& p, bool insertedList, bool insertedSorted, bool insertedAVL, bool insertedBTree, bool insertedBPlus);
};

#endif