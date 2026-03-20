#ifndef BTREE_H
#define BTREE_H

#include "Product.h"
#include <fstream>
#include <string>

/* Grado minimo del arbol B.
 * Cada nodo (excepto la raiz) tiene entre T-1 y 2T-1 claves.
 * Con T=4: nodo lleno tiene 7 claves y hasta 8 hijos.
 * Elegido porque maximiza la reduccion de altura antes de rendimientos
 * decrecientes: altura maxima = 5 para n=1000, con .dot legible (7 claves/nodo).
 */
static const int BTREE_T = 4;

/*
 * BNode
 * Nodo interno del arbol B.
 * Capacidad maxima: 2*BTREE_T - 1 claves.
 * Los datos (punteros a Product) son paralelos al arreglo de claves.
 */
struct BNode {
    std::string keys[2 * BTREE_T - 1];     // claves (fechas YYYY-MM-DD)
    Product*    data[2 * BTREE_T - 1];     // punteros a productos (paralelo a keys)
    BNode*      children[2 * BTREE_T];     // punteros a hijos
    int         n;                          // cantidad actual de claves
    bool        isLeaf;                     // true si el nodo no tiene hijos

    BNode();
    ~BNode();
};

/*
 * BTree
 * Arbol B de grado minimo BTREE_T.
 * Clave de busqueda: expiry_date en formato "YYYY-MM-DD" (comparable como string).
 * Soporta busqueda puntual y por rango de fechas.
 *
 * Complejidades:
 *   insert      : O(log n)
 *   search      : O(log n)
 *   remove      : O(log n)
 *   rangeSearch : O(log n + k)  donde k = elementos en el rango
 *   toDot       : O(n)
 */
class BTree {
public:
    BTree();
    ~BTree();

    BTree(const BTree&)            = delete;
    BTree& operator=(const BTree&) = delete;

    /*
     * insert
     * Precondicion: p.expiry_date tiene formato valido "YYYY-MM-DD".
     * Postcondicion: p insertado en la posicion correcta por fecha;
     *                nodo dividido (split) si estaba lleno; arbol valido.
     * Complejidad: O(log n)
     */
    void insert(const Product& p);

    /*
     * search
     * Precondicion: date tiene formato "YYYY-MM-DD".
     * Postcondicion: retorna puntero al primer Product con esa fecha o nullptr.
     * Complejidad: O(log n)
     */
    Product* search(const std::string& date);

    /*
     * rangeSearch
     * Busca todos los productos cuya fecha este en el intervalo [d1, d2].
     * Precondicion: d1 <= d2; ambos con formato "YYYY-MM-DD".
     * Postcondicion: results contiene punteros a todos los productos del rango.
     * Complejidad: O(log n + k)
     */
    void rangeSearch(const std::string& d1, const std::string& d2, Product** results, int& count, int maxResults) const;

    /*
     * remove
     * Precondicion: date puede o no existir.
     * Postcondicion: primer producto con esa fecha eliminado (si existia);
     *                merges o redistribucion aplicados para mantener invariantes.
     * Complejidad: O(log n)
     */
    void remove(const std::string& date);

    /*
     * toDot
     * Genera representacion Graphviz del arbol B.
     * Precondicion: out esta abierto para escritura.
     * Postcondicion: archivo .dot con paginas (nodos) y claves representadas.
     * Complejidad: O(n)
     */
    void toDot(std::ofstream& out) const;

    /*
     * isEmpty
     * Precondicion: ninguna.
     * Postcondicion: retorna true si el arbol no tiene claves.
     * Complejidad: O(1)
     */
    bool isEmpty() const;

private:
    BNode* _root; // puntero a la raiz del arbol
    int    _t;    // grado minimo (= BTREE_T)

    // Insercion cuando la raiz no esta llena
    void insertNonFull(BNode* node, const Product& p);

    // Division del hijo i-esimo de node cuando esta lleno
    void splitChild(BNode* parent, int i, BNode* child);

    // Busqueda recursiva
    Product* searchNode(BNode* node, const std::string& date) const;

    // Recorrido en orden para rangeSearch
    void rangeNode(BNode* node, const std::string& d1, const std::string& d2, Product** results, int& count, int maxResults) const;

    // Eliminacion recursiva
    void removeNode(BNode* node, const std::string& date);

    // Helpers de eliminacion: merge y redistribucion
    void    mergeChildren(BNode* node, int idx);
    void    redistributeFromLeft(BNode* node, int idx);
    void    redistributeFromRight(BNode* node, int idx);
    Product* getPredecessor(BNode* node, int idx) const;
    Product* getSuccessor(BNode* node, int idx)   const;

    // Generacion de DOT recursiva
    void toDotNode(BNode* node, std::ofstream& out, int& nodeId) const;

    // Destruccion recursiva
    void destroyNode(BNode* node);
};

#endif