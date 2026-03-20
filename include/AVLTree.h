#ifndef AVLTREE_H
#define AVLTREE_H

#include "Product.h"
#include <fstream>
#include <functional>

/*
 * AVLNode
 * Nodo interno del arbol AVL.
 * Almacena una copia del producto y mantiene la altura
 * para calcular el factor de balance en cada operacion.
 */
struct AVLNode {
    Product  data;   // producto almacenado en este nodo
    int      height; // altura del nodo (hoja = 1)
    AVLNode* left;   // hijo izquierdo
    AVLNode* right;  // hijo derecho

    explicit AVLNode(const Product& p);
};

/*
 * AVLTree
 * Arbol AVL ordenado por Product.name.
 * Garantiza altura O(log n) mediante rotaciones tras cada insercion
 * o eliminacion, manteniendo |balance| <= 1 en cada nodo.
 *
 * Clave de busqueda: name (string, comparacion lexicografica).
 * Uso principal: busqueda por nombre y listado alfabetico.
 */
class AVLTree {
public:
    AVLTree();
    ~AVLTree();

    AVLTree(const AVLTree&)            = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    /*
     * insert
     * Precondicion: p.isValid() == true; p.name no duplicado en el arbol.
     * Postcondicion: p insertado; arbol rebalanceado; |balance| <= 1 en todo nodo.
     * Complejidad: O(log n)
     */
    void insert(const Product& p);

    /*
     * remove
     * Precondicion: name puede o no existir en el arbol.
     * Postcondicion: nodo con data.name == name eliminado (si existia);
     *                arbol rebalanceado.
     * Complejidad: O(log n)
     */
    void remove(const std::string& name);

    /*
     * search
     * Precondicion: name no vacio.
     * Postcondicion: retorna puntero al Product encontrado o nullptr.
     * Complejidad: O(log n)
     */
    Product* search(const std::string& name);

    /*
     * inorder
     * Recorre el arbol en orden (izquierda - raiz - derecha),
     * que equivale a orden alfabetico por name.
     * Precondicion: cb es valido.
     * Postcondicion: cb invocado exactamente una vez por cada nodo en orden.
     * Complejidad: O(n)
     */
    void inorder(std::function<void(const Product&)> cb) const;

    /*
     * toDot
     * Genera representacion Graphviz del arbol en formato DOT.
     * Precondicion: out esta abierto para escritura.
     * Postcondicion: archivo .dot valido escrito en out con nodos y aristas.
     */
    void toDot(std::ofstream& out) const;

    /*
     * isEmpty
     * Postcondicion: retorna true si el arbol no tiene nodos.
     */
    bool isEmpty() const;

private:
    AVLNode* _root; // puntero a la raiz del arbol

    // Helpers recursivos internos
    AVLNode* insertNode(AVLNode* node, const Product& p);
    AVLNode* removeNode(AVLNode* node, const std::string& name);
    Product* searchNode(AVLNode* node, const std::string& name) const;
    void     inorderNode(AVLNode* node, std::function<void(const Product&)> cb) const;
    void     toDotNode(AVLNode* node, std::ofstream& out) const;
    void     destroyNode(AVLNode* node);

    // Utilidades de balance
    int      height(AVLNode* node) const;
    int      balanceFactor(AVLNode* node) const;
    void     updateHeight(AVLNode* node);

    // Rotaciones
    AVLNode* rotateLeft(AVLNode* node);
    AVLNode* rotateRight(AVLNode* node);

    // Rebalanceo tras insercion o eliminacion
    AVLNode* rebalance(AVLNode* node);

    // Nodo con el valor minimo en el subarbol (usado en eliminacion)
    AVLNode* minNode(AVLNode* node) const;
};

#endif