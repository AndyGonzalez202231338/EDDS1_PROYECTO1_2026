#include "AVLTree.h"
#include <algorithm>
#include <stdexcept>


AVLNode::AVLNode(const Product& p)
    : data(p), height(1), left(nullptr), right(nullptr) {}


AVLTree::AVLTree()
    : _root(nullptr) {}

AVLTree::~AVLTree() {
    destroyNode(_root);
}

void AVLTree::insert(const Product& p) {
    _root = insertNode(_root, p);
}

void AVLTree::remove(const std::string& name) {
    _root = removeNode(_root, name);
}

Product* AVLTree::search(const std::string& name) {
    return searchNode(_root, name);
}

void AVLTree::inorder(std::function<void(const Product&)> cb) const {
    inorderNode(_root, cb);
}

void AVLTree::toDot(std::ofstream& out) const {
    out << "digraph AVL {\n";
    out << "    node [shape=box fontname=\"Arial\" fontsize=10];\n";
    if (_root) toDotNode(_root, out);
    out << "}\n";
}

bool AVLTree::isEmpty() const {
    return _root == nullptr;
}


/**
 * height
 * Retorna la altura del nodo, 0 si es nullptr.
 */
int AVLTree::height(AVLNode* node) const {
    return node ? node->height : 0;
}

/**
 * balanceFactor
 * Factor de balance = altura(izq) - altura(der).
 * Un arbol AVL mantiene este valor en {-1, 0, 1}.
 */
int AVLTree::balanceFactor(AVLNode* node) const {
    return node ? height(node->left) - height(node->right) : 0;
}

/**
 * updateHeight
 * Recalcula la altura de un nodo a partir de sus hijos.
 */
void AVLTree::updateHeight(AVLNode* node) {
    if (node) {
        node->height = 1 + std::max(height(node->left), height(node->right));
    }
}

/*      ROTACIONES      */

/*
 * rotateLeft
 * Rotacion simple a la izquierda. Se usa cuando el subarbol derecho
 * esta desbalanceado (balance < -1, hijo derecho pesado a la derecha).
 *
 *     node                  right
 *    /    \                /     \
 *   A     right   =>    node      C
 *        /    \        /    \
 *       B      C      A      B
 */
AVLNode* AVLTree::rotateLeft(AVLNode* node) {
    // right es el nuevo nodo raiz del subarbol
    AVLNode* right = node->right;
    // rightLeft es el hijo izquierdo de right (B) que se va a mover a ser el hijo derecho de node
    AVLNode* rightLeft  = right->left;

    // Realizar la rotacion

    // right se convierte en la nueva raiz del subarbol, node se convierte en el hijo izquierdo de right
    right->left  = node;
    // rightLeft (B) se convierte en el hijo derecho de node
    node->right  = rightLeft;

    updateHeight(node);
    updateHeight(right);

    return right;
}

/*
 * rotateRight
 * Rotacion simple a la derecha. Se usa cuando el subarbol izquierdo
 * esta desbalanceado (balance > 1, hijo izquierdo pesado a la izquierda).
 *
 *       node              left
 *      /    \            /    \
 *    left    C   =>     A     node
 *   /    \                   /    \
 *  A      B                 B      C
 */
AVLNode* AVLTree::rotateRight(AVLNode* node) {
    // left es el nuevo nodo raiz del subarbol
    AVLNode* left      = node->left;
    // leftRight es el hijo derecho de left (B) que se va a mover a ser el hijo izquierdo de node
    AVLNode* leftRight = left->right;

    // Realizar la rotacion
    // left se convierte en la nueva raiz del subarbol, node se convierte en el hijo derecho de left
    left->right = node;
    // leftRight (B) se convierte en el hijo izquierdo de node
    node->left  = leftRight;

    updateHeight(node);
    updateHeight(left);

    return left;
}

/*      REBALANCEO      */

/*
 * rebalance
 * Detecta el caso de desbalance y aplica las rotaciones necesarias.
 * Hay 4 casos posibles:
 *
 *   Caso LL (balance > 1, hijo izq pesado a la izq): rotacion simple derecha
 *   Caso LR (balance > 1, hijo izq pesado a la der): rotacion doble izq-der
 *   Caso RR (balance < -1, hijo der pesado a la der): rotacion simple izquierda
 *   Caso RL (balance < -1, hijo der pesado a la izq): rotacion doble der-izq
 */
AVLNode* AVLTree::rebalance(AVLNode* node) {
    updateHeight(node);
    int bal = balanceFactor(node);

    // Caso LL
    if (bal > 1 && balanceFactor(node->left) >= 0) {
        return rotateRight(node);
    }
    // Caso LR
    if (bal > 1 && balanceFactor(node->left) < 0) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    // Caso RR
    if (bal < -1 && balanceFactor(node->right) <= 0) {
        return rotateLeft(node);
    }
    // Caso RL
    if (bal < -1 && balanceFactor(node->right) > 0) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node; // ya estaba balanceado
}



/**
 * insertNode
 * Desciende recursivamente por el arbol comparando nombres,
 * inserta el nodo en la posicion correcta y rebalancea en el
 * camino de retorno.
 *  */
AVLNode* AVLTree::insertNode(AVLNode* node, const Product& p) {
    // Caso base: posicion encontrada
    if (node == nullptr) {
        return new AVLNode(p);
    }

    if (p.name < node->data.name) {
        node->left  = insertNode(node->left, p);
    } else if (p.name > node->data.name) {
        node->right = insertNode(node->right, p);
    } else {
        // Nombre duplicado: no se inserta (barcode es la clave unica,
        // pero el AVL indexa por nombre)
        return node;
    }

    return rebalance(node);
}

/**
 * minNode
 * Retorna el nodo con la clave minima en el subarbol.
 * Usado en eliminacion para encontrar el sucesor inorder.
 */
AVLNode* AVLTree::minNode(AVLNode* node) const {
    AVLNode* curr = node;
    while (curr->left != nullptr) {
        curr = curr->left;
    }
    return curr;
}

/**
 * removeNode
 * Busca el nodo con ese nombre, lo elimina segun el caso
 * (hoja, un hijo, dos hijos) y rebalancea en el camino de retorno.
 */
AVLNode* AVLTree::removeNode(AVLNode* node, const std::string& name) {
    if (node == nullptr) return nullptr;

    if (name < node->data.name) {
        node->left  = removeNode(node->left, name);
    } else if (name > node->data.name) {
        node->right = removeNode(node->right, name);
    } else {
        // Nodo encontrado
        if (node->left == nullptr || node->right == nullptr) {
            // Caso hoja o un solo hijo
            AVLNode* child = node->left ? node->left : node->right;
            delete node;
            return child;
        }

        // Caso dos hijos: reemplazar con el sucesor inorder (minimo del subarbol derecho)
        AVLNode* successor = minNode(node->right);
        // Copiar los datos del sucesor al nodo actual
        node->data = successor->data;
        // Eliminar el nodo sucesor (que ahora esta duplicado)
        node->right = removeNode(node->right, successor->data.name);
    }

    return rebalance(node);
}

/**
 * searchNode
 * Busqueda binaria recursiva por nombre.
 */
Product* AVLTree::searchNode(AVLNode* node, const std::string& name) const {
    if (node == nullptr)         return nullptr;
    if (name == node->data.name) return &node->data;
    if (name < node->data.name)  return searchNode(node->left, name);
    return searchNode(node->right, name);
}

/**
 * inorderNode
 * Recorrido izquierda - raiz - derecha.
 * Produce los productos en orden alfabetico por nombre.
 */
void AVLTree::inorderNode(AVLNode* node, std::function<void(const Product&)> cb) const {
    if (node == nullptr) return;
    inorderNode(node->left, cb);
    cb(node->data);
    inorderNode(node->right, cb);
}

/**
 * toDotNode
 * Genera los nodos y aristas del grafo DOT de forma recursiva.
 * Cada nodo muestra el nombre del producto y su altura.
 */
void AVLTree::toDotNode(AVLNode* node, std::ofstream& out) const {
    if (node == nullptr) return;

    // Etiqueta del nodo: nombre + altura entre corchetes
    out << "    \"" << node->data.name << "\" "
        << "[label=\"" << node->data.name
        << "\\nh=" << node->height << "\"];\n";

    if (node->left) {
        out << "    \"" << node->data.name << "\" -> \""
            << node->left->data.name  << "\" [label=\"L\"];\n";
        toDotNode(node->left, out);
    }
    if (node->right) {
        out << "    \"" << node->data.name << "\" -> \""
            << node->right->data.name << "\" [label=\"R\"];\n";
        toDotNode(node->right, out);
    }
}

void AVLTree::destroyNode(AVLNode* node) {
    if (node == nullptr) return;
    destroyNode(node->left);
    destroyNode(node->right);
    delete node;
}