#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include "Product.h"
#include <fstream>
#include <string>

/* Grado minimo del arbol B+.
 * Hojas: entre T-1 y 2*T-1 entradas. Con T=4: entre 3 y 7 entradas por hoja.
 * Nodos internos: entre T-1 y 2*T-1 claves y entre T y 2*T hijos.
 * Misma justificacion que BTree: altura max 5 para n=1000, .dot legible.
 */
static const int BPLUS_T = 4;

// Capacidad maxima de claves en un nodo
static const int BPLUS_MAX_KEYS = 2 * BPLUS_T - 1;

/*
 * Tipo de nodo (hoja o interno).
 * Permite distinguir en tiempo de ejecucion sin dynamic_cast.
 */
enum class BPlusNodeType { LEAF, INTERNAL };

/*
 * BPlusNode
 * Clase base para nodos del arbol B+.
 */
struct BPlusNode {
    BPlusNodeType type; // LEAF o INTERNAL
    int           n;    // cantidad actual de claves/entradas

    explicit BPlusNode(BPlusNodeType t);
    virtual ~BPlusNode() = default;
};

/*
 * BPlusLeaf
 * Nodo hoja del arbol B+.
 * Almacena los datos reales (punteros a Product) y enlaza con el
 * siguiente nodo hoja para permitir recorrido eficiente por nivel.
 *
 * Invariante: keys[i] < keys[i+1] para todo i valido.
 */
struct BPlusLeaf : public BPlusNode {
    std::string  keys[BPLUS_MAX_KEYS];  // claves (categorias)
    Product*     data[BPLUS_MAX_KEYS];  // punteros a productos (paralelo a keys)
    BPlusLeaf*   next;                  // enlace al siguiente nodo hoja

    BPlusLeaf();
};

/*
 * BPlusInternal
 * Nodo interno del arbol B+.
 * Solo contiene claves guia y punteros a hijos.
 * Los datos reales estan exclusivamente en las hojas.
 */
struct BPlusInternal : public BPlusNode {
    std::string  keys[BPLUS_MAX_KEYS];      // claves separadoras
    BPlusNode*   children[BPLUS_MAX_KEYS + 1]; // punteros a hijos (n+1 como maximo)

    BPlusInternal();
};

/*
 * BPlusTree
 * Arbol B+ de grado minimo BPLUS_T.
 * Clave de busqueda: category (string, comparacion lexicografica).
 * Los datos estan SOLO en las hojas; los nodos internos son indices.
 * Las hojas estan enlazadas entre si (puntero next) para recorrido
 * eficiente de todos los productos de una categoria.
 *
 * Complejidades:
 *   insert         : O(log n)
 *   remove         : O(log n)
 *   searchCategory : O(log n + k)  donde k = productos en la categoria
 *   leafTraversal  : O(n)
 *   toDot          : O(n)
 */
class BPlusTree {
public:
    BPlusTree();
    ~BPlusTree();

    BPlusTree(const BPlusTree&)            = delete;
    BPlusTree& operator=(const BPlusTree&) = delete;

    /*
     * insert
     * Precondicion: p.category no vacio.
     * Postcondicion: p insertado en la hoja correcta ordenado por category;
     *                splits propagados hacia la raiz si fue necesario.
     * Complejidad: O(log n)
     */
    void insert(const Product& p);

    /*
     * remove
     * Precondicion: category puede o no existir.
     * Postcondicion: primera entrada con esa category eliminada de su hoja;
     *                merge o redistribucion aplicados si hay underflow.
     * Complejidad: O(log n)
     */
    void remove(const std::string& category);

    /*
     * searchCategory
     * Busca todos los productos de una categoria usando el enlace next de hojas.
     * Precondicion: category no vacio; results tiene capacidad >= maxResults.
     * Postcondicion: results contiene punteros a los productos encontrados;
     *                count indica cuantos se encontraron.
     * Complejidad: O(log n + k)
     */
    void searchCategory(const std::string& category, Product** results, int& count, int maxResults) const;

    /*
     * leafTraversal
     * Recorre todas las hojas enlazadas desde firstLeaf.
     * Precondicion: results tiene capacidad >= maxResults.
     * Postcondicion: results contiene punteros a todos los productos en orden
     *                de categoria; count indica el total.
     * Complejidad: O(n)
     */
    void leafTraversal(Product** results, int& count, int maxResults) const;

    /*
     * toDot
     * Genera representacion Graphviz del arbol B+.
     * Precondicion: out esta abierto para escritura.
     * Postcondicion: archivo .dot con nodos internos y hojas diferenciados;
     *                enlaces entre hojas representados con aristas punteadas.
     * Complejidad: O(n)
     */
    void toDot(std::ofstream& out) const;

    /*
     * isEmpty
     * Precondicion: ninguna.
     * Postcondicion: retorna true si no hay entradas en el arbol.
     * Complejidad: O(1)
     */
    bool isEmpty() const;

private:
    BPlusNode* _root;       // raiz del arbol (puede ser hoja o interno)
    BPlusLeaf* _firstLeaf;  // primera hoja (inicio del nivel de hojas)
    int        _t;          // grado minimo (= BPLUS_T)

    // Busca la hoja donde deberia estar category
    BPlusLeaf* findLeaf(const std::string& category) const;

    // Insercion en hoja con posible split
    void insertInLeaf(BPlusLeaf* leaf, const Product& p);
    void splitLeaf(BPlusLeaf* leaf, BPlusInternal* parent, int childIdx);
    void splitInternal(BPlusInternal* node, BPlusInternal* parent, int childIdx);
    void insertInParent(BPlusNode* left, const std::string& key, BPlusNode* right);

    // Generacion de DOT recursiva
    void toDotNode(BPlusNode* node, std::ofstream& out, int& nodeId) const;

    // Destruccion recursiva
    void destroyNode(BPlusNode* node);
};

#endif // BPLUSTREE_H