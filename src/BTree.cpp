#include "BTree.h"
#include <stdexcept>
#include <algorithm>

BNode::BNode() : n(0), isLeaf(true) {
    for (int i = 0; i < 2 * BTREE_T;     ++i) children[i] = nullptr;
    for (int i = 0; i < 2 * BTREE_T - 1; ++i) data[i]     = nullptr;
}

BNode::~BNode() {
    // Solo libera los punteros a Product que este nodo posee. Los hijos se liberan desde destroyNode en BTree.
    for (int i = 0; i < n; ++i) {
        delete data[i];
        data[i] = nullptr;
    }
}

BTree::BTree() : _root(nullptr), _t(BTREE_T) {
    _root         = new BNode(); // iniciar con una raiz vacia
    _root->isLeaf = true; // la raiz inicia como hoja
    _root->n      = 0; //no tiene clave iniciando
}

BTree::~BTree() {
    destroyNode(_root);
}

bool BTree::isEmpty() const {
    return _root == nullptr || _root->n == 0;
}

void BTree::insert(const Product& p) {
    BNode* root = _root;

    // Si la raiz esta llena, se divide antes de insertar
    if (root->n == 2 * _t - 1) {
        BNode* newRoot   = new BNode();
        newRoot->isLeaf  = false; // la nueva raiz no es hoja
        newRoot->n       = 0;
        newRoot->children[0] = root; // el hijo 0 de la nueva raiz es la raiz vieja
        _root = newRoot; //la nueva raiz es ahora el nodo creado

        splitChild(newRoot, 0, root); // dividir la raiz vieja y subir la clave media a la nueva raiz
        insertNonFull(newRoot, p); // insertar el producto en la nueva raiz (que ahora tiene espacio)
    } else {
        insertNonFull(root, p); // insertar directamente en la raiz si no esta llena
    }
}

/**
 * insertNonFull
 * Inserta en un nodo que garantizadamente no esta lleno.
 * Si es hoja, inserta directamente.
 * Si es interno, desciende al hijo correcto, dividendolo si esta lleno.
 */
void BTree::insertNonFull(BNode* node, const Product& p) {
    int i = node->n - 1;

    if (node->isLeaf) {
        // Desplazar claves mayores una posicion a la derecha
        while (i >= 0 && p.expiry_date < node->keys[i]) { // mover claves y datos a la derecha para hacer espacio
            node->keys[i + 1] = node->keys[i];
            node->data[i + 1] = node->data[i];
            --i;
        }
        node->keys[i + 1] = p.expiry_date;
        node->data[i + 1] = new Product(p);
        ++node->n; // incrementar el numero de claves en este nodo
    } else {
        // Encontrar el hijo correcto
        while (i >= 0 && p.expiry_date < node->keys[i]) --i; // i termina en la posicion del hijo a descender - 1
        ++i; // corregir para que i sea el indice del hijo a descender

        // Dividir hijo si esta lleno
        if (node->children[i]->n == 2 * _t - 1) { // el hijo esta lleno, dividirlo
            splitChild(node, i, node->children[i]); // dividir el hijo i y subir la clave media a node
            if (p.expiry_date > node->keys[i]) ++i; // determinar a cual de los dos hijos resultantes descender
        }
        insertNonFull(node->children[i], p); // insertar recursivamente en el hijo que ahora garantizadamente no esta lleno
    }
}

/**
 * splitChild
 * Divide el hijo lleno en dos y sube la clave mediana al padre.
 *
 *   parent
 *     |
 *   child (lleno, 2t-1 claves)
 *
 * Resultado: parent queda con child a la izq y newChild a la der.
 * child y newChild tienen t-1 claves cada uno.
 * La clave mediana sube a parent.
 */
void BTree::splitChild(BNode* parent, int i, BNode* child) {
    BNode* newChild   = new BNode();
    newChild->isLeaf  = child->isLeaf;
    newChild->n       = _t - 1;

    // Copiar la mitad derecha de child a newChild
    for (int j = 0; j < _t - 1; ++j) {
        newChild->keys[j] = child->keys[j + _t]; // copiar claves
        newChild->data[j] = child->data[j + _t];
        child->data[j + _t] = nullptr; // evitar doble liberacion
    }
    if (!child->isLeaf) {
        for (int j = 0; j < _t; ++j) {
            newChild->children[j] = child->children[j + _t]; // copiar hijos
            child->children[j + _t] = nullptr;
        }
    }
    child->n = _t - 1; // el hijo original ahora tiene solo t-1 claves

    // Desplazar hijos de parent a la derecha para hacer espacio
    for (int j = parent->n; j >= i + 1; --j) {
        parent->children[j + 1] = parent->children[j];
    }
    parent->children[i + 1] = newChild; // el nuevo hijo va a la derecha del hijo original

    // Desplazar claves de parent a la derecha
    for (int j = parent->n - 1; j >= i; --j) {
        parent->keys[j + 1] = parent->keys[j];
        parent->data[j + 1] = parent->data[j];
    }

    // Subir la clave mediana al padre
    parent->keys[i] = child->keys[_t - 1];
    parent->data[i] = child->data[_t - 1];
    child->data[_t - 1] = nullptr; // el padre contiene ahora este Product
    ++parent->n;
}


Product* BTree::search(const std::string& date) {
    return searchNode(_root, date);
}

Product* BTree::searchNode(BNode* node, const std::string& date) const {
    if (node == nullptr) return nullptr;

    // Busqueda lineal dentro del nodo
    int i = 0;
    while (i < node->n && date > node->keys[i]) ++i;

    if (i < node->n && date == node->keys[i]) {
        return node->data[i];
    }
    if (node->isLeaf) return nullptr;

    return searchNode(node->children[i], date);
}


void BTree::rangeSearch(const std::string& d1, const std::string& d2, Product** results,int& count,int maxResults) const {
    count = 0;
    rangeNode(_root, d1, d2, results, count, maxResults);
}

/**
 * rangeNode
 * Recorre el arbol en orden y recolecta todos los productos
 * cuya clave este en el intervalo [d1, d2].
 *  
 * */
void BTree::rangeNode(BNode* node,const std::string& d1,const std::string& d2,Product** results,int& count,int maxResults) const {
    if (node == nullptr || count >= maxResults) return;

    for (int i = 0; i < node->n && count < maxResults; ++i) {
        // Descender al hijo izquierdo antes de procesar la clave i
        if (!node->isLeaf) {
            rangeNode(node->children[i], d1, d2, results, count, maxResults);
        }
        // Procesar clave i si esta en el rango
        if (node->keys[i] >= d1 && node->keys[i] <= d2) {
            results[count++] = node->data[i];
        }
    }
    // Descender al ultimo hijo
    if (!node->isLeaf && count < maxResults) {
        rangeNode(node->children[node->n], d1, d2, results, count, maxResults);
    }
}


void BTree::remove(const std::string& date) {
    if (!_root || _root->n == 0) return;
    removeNode(_root, date);

    // Si la raiz quedo vacia y tiene un hijo, ese hijo es la nueva raiz
    if (_root->n == 0 && !_root->isLeaf) {
        BNode* oldRoot = _root;
        _root          = _root->children[0];
        oldRoot->n     = 0; // evitar que el destructor libere data[0..n-1]
        delete oldRoot;
    }
}

/**
 * removeNode
 * Elimina la clave date del subarbol con raiz node.
 * Garantiza que node tenga al menos t claves antes de descender.
 *  
 * */ 
void BTree::removeNode(BNode* node, const std::string& date) {
    int i = 0;
    while (i < node->n && date > node->keys[i]) ++i; // i termina en la posicion del hijo a descender o en la clave que se quiere eliminar

    if (i < node->n && date == node->keys[i]) { // clave encontrada en este nodo
        // Caso A: clave encontrada en este nodo
        if (node->isLeaf) {
            // Caso A1: nodo hoja - eliminar directamente
            delete node->data[i];
            for (int j = i; j < node->n - 1; ++j) { // desplazar claves y datos a la izquierda para llenar el espacio
                node->keys[j] = node->keys[j + 1];
                node->data[j] = node->data[j + 1];
            }
            node->data[node->n - 1] = nullptr; 
            --node->n; // decrementar el numero de claves en este nodo
        } else {
            // Caso A2: nodo interno
            BNode* leftChild  = node->children[i]; // hijo izquierdo del indice i
            BNode* rightChild = node->children[i + 1]; // hijo derecho del indice i

            if (leftChild->n >= _t) {
                // Caso A2a: el hijo izquierdo tiene >= t claves
                // Reemplazar con el predecesor
                Product* pred = getPredecessor(node, i);
                delete node->data[i];
                node->keys[i] = pred->expiry_date;
                node->data[i] = new Product(*pred);
                removeNode(leftChild, pred->expiry_date); // eliminar recursivamente el predecesor en el hijo izquierdo
            } else if (rightChild->n >= _t) {
                // Caso A2b: el hijo derecho tiene >= t claves
                // Reemplazar con el sucesor
                Product* succ = getSuccessor(node, i);
                delete node->data[i];
                node->keys[i] = succ->expiry_date;
                node->data[i] = new Product(*succ);
                removeNode(rightChild, succ->expiry_date); // eliminar recursivamente el sucesor en el hijo derecho
            } else {
                // Caso A2c: ambos hijos tienen t-1 claves - merge
                mergeChildren(node, i); // fusiona leftChild, la clave i y rightChild en un solo nodo con 2t-1 claves
                removeNode(leftChild, date);
            }
        }
    } else {
        // Caso B: clave no esta en este nodo
        if (node->isLeaf) return; // no existe en el arbol

        bool lastChild = (i == node->n); // si i == n, el hijo a descender es el ultimo (hijo derecho del ultimo indice)
        BNode* child   = node->children[i];

        if (child->n < _t) { // el hijo a descender tiene solo t-1 claves, no es seguro descender
            // Garantizar que el hijo tenga al menos t claves
            if (i > 0 && node->children[i - 1]->n >= _t) {
                redistributeFromLeft(node, i); // el hermano izquierdo tiene suficientes claves para ceder una
            } else if (!lastChild && i < node->n && node->children[i + 1]->n >= _t) {
                redistributeFromRight(node, i); // el hermano derecho tiene suficientes claves para ceder una
            } else {
                if (lastChild) --i; // si es el ultimo hijo, merge con el hermano izquierdo
                mergeChildren(node, i);
                child = node->children[i]; // después del merge, el hijo a descender es el resultado del merge (que tiene 2t-1 claves)
            }
        }
        removeNode(child, date);
    }
}

/**
 * getPredecessor
 * Retorna el producto con la clave maxima en el subarbol
 * del hijo izquierdo del indice idx.
 * */ 
Product* BTree::getPredecessor(BNode* node, int idx) const {
    BNode* curr = node->children[idx];
    while (!curr->isLeaf) curr = curr->children[curr->n];
    return curr->data[curr->n - 1];
}

/**
 * getSuccessor
 * Retorna el producto con la clave minima en el subarbol
 * del hijo derecho del indice idx.
 * */ 
Product* BTree::getSuccessor(BNode* node, int idx) const {
    BNode* curr = node->children[idx + 1];
    while (!curr->isLeaf) curr = curr->children[0];
    return curr->data[0];
}

/**
 * mergeChildren
 * Fusiona el hijo idx y el hijo idx+1 a traves de la clave idx del padre.
 * Resultado: el hijo idx queda con 2t-1 claves, el padre pierde una clave.
 * */ 
void BTree::mergeChildren(BNode* node, int idx) {
    BNode* left  = node->children[idx];
    BNode* right = node->children[idx + 1];

    // Bajar la clave del padre al hijo izquierdo
    left->keys[_t - 1] = node->keys[idx];
    left->data[_t - 1] = node->data[idx];
    node->data[idx]    = nullptr;

    // Copiar las claves del hijo derecho al izquierdo
    for (int i = 0; i < right->n; ++i) {
        left->keys[_t + i] = right->keys[i];
        left->data[_t + i] = right->data[i];
        right->data[i]     = nullptr; // evitar doble liberacion
    }
    if (!right->isLeaf) {
        for (int i = 0; i <= right->n; ++i) {
            left->children[_t + i] = right->children[i];
        }
    }
    left->n = 2 * _t - 1;

    // Eliminar la clave del padre y ajustar sus hijos
    for (int i = idx; i < node->n - 1; ++i) {
        node->keys[i]         = node->keys[i + 1];
        node->data[i]         = node->data[i + 1];
        node->children[i + 1] = node->children[i + 2];
    }
    node->data[node->n - 1]     = nullptr;
    node->children[node->n]     = nullptr;
    --node->n;

    right->n = 0; // evitar que el destructor libere sus datos
    delete right;
}

/**
 * redistributeFromLeft
 * El hermano izquierdo del hijo idx cede su clave maxima
 * a traves del padre hacia el hijo idx.
 * */ 
void BTree::redistributeFromLeft(BNode* node, int idx) {
    BNode* child  = node->children[idx]; // hijo que necesita claves
    BNode* sibling = node->children[idx - 1]; // hermano izquierdo que cede una clave

    // Desplazar claves del hijo a la derecha
    for (int i = child->n; i > 0; --i) {
        child->keys[i] = child->keys[i - 1];
        child->data[i] = child->data[i - 1];
    }
    if (!child->isLeaf) {
        for (int i = child->n + 1; i > 0; --i) {
            child->children[i] = child->children[i - 1];
        }
    }

    // Bajar clave del padre al hijo
    child->keys[0] = node->keys[idx - 1];
    child->data[0] = node->data[idx - 1];

    // Subir la clave maxima del hermano al padre
    node->keys[idx - 1] = sibling->keys[sibling->n - 1];
    node->data[idx - 1] = sibling->data[sibling->n - 1];
    sibling->data[sibling->n - 1] = nullptr;

    if (!sibling->isLeaf) {
        child->children[0] = sibling->children[sibling->n];
        sibling->children[sibling->n] = nullptr;
    }
    ++child->n;
    --sibling->n;
}

/**
 * redistributeFromRight
 * El hermano derecho del hijo idx cede su clave minima
 * a traves del padre hacia el hijo idx.
 * */ 
void BTree::redistributeFromRight(BNode* node, int idx) {
    BNode* child   = node->children[idx]; // hijo que necesita claves
    BNode* sibling = node->children[idx + 1]; // hermano derecho que cede una clave

    // Bajar clave del padre al final del hijo
    child->keys[child->n] = node->keys[idx];
    child->data[child->n] = node->data[idx];

    if (!child->isLeaf) {
        child->children[child->n + 1] = sibling->children[0];
    }

    // Subir la clave minima del hermano al padre
    node->keys[idx] = sibling->keys[0];
    node->data[idx] = sibling->data[0];
    sibling->data[0] = nullptr;

    // Desplazar claves del hermano a la izquierda
    for (int i = 0; i < sibling->n - 1; ++i) {
        sibling->keys[i] = sibling->keys[i + 1];
        sibling->data[i] = sibling->data[i + 1];
    }
    if (!sibling->isLeaf) {
        for (int i = 0; i < sibling->n; ++i) {
            sibling->children[i] = sibling->children[i + 1];
        }
    }
    sibling->data[sibling->n - 1] = nullptr;
    ++child->n;
    --sibling->n;
}


void BTree::toDot(std::ofstream& out) const {
    out << "digraph BTree {\n";
    out << "    node [shape=record fontname=\"Arial\" fontsize=10];\n";
    int nodeId = 0;
    if (_root) toDotNode(_root, out, nodeId);
    out << "}\n";
}

/**
 * toDotNode
 * Genera la representacion de cada pagina (nodo) del arbol B.
 * Cada nodo se muestra como un record con sus claves.
 * */ 
void BTree::toDotNode(BNode* node, std::ofstream& out, int& nodeId) const {
    if (node == nullptr) return;

    int myId = nodeId++;

    // Construir etiqueta con todas las claves del nodo
    out << "    node" << myId << " [label=\"";
    for (int i = 0; i < node->n; ++i) {
        if (i > 0) out << "|";
        out << "<f" << i << "> " << node->keys[i];
    }
    out << "\"];\n";

    if (!node->isLeaf) {
        for (int i = 0; i <= node->n; ++i) {
            int childId = nodeId;
            toDotNode(node->children[i], out, nodeId);
            out << "    node" << myId << " -> node" << childId << ";\n";
        }
    }
}

void BTree::destroyNode(BNode* node) {
    if (node == nullptr) return;
    if (!node->isLeaf) {
        for (int i = 0; i <= node->n; ++i) {
            destroyNode(node->children[i]);
        }
    }
    delete node;
}