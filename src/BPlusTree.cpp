#include "BPlusTree.h"
#include <algorithm>


BPlusNode::BPlusNode(BPlusNodeType t)
    : type(t), n(0) {}


BPlusLeaf::BPlusLeaf()
    : BPlusNode(BPlusNodeType::LEAF), next(nullptr) {
    for (int i = 0; i < BPLUS_MAX_KEYS; ++i) data[i] = nullptr;
}

BPlusInternal::BPlusInternal()
    : BPlusNode(BPlusNodeType::INTERNAL) {
    for (int i = 0; i <= BPLUS_MAX_KEYS; ++i) children[i] = nullptr;
}

BPlusTree::BPlusTree()
    : _root(nullptr), _firstLeaf(nullptr), _t(BPLUS_T) {
    BPlusLeaf* leaf = new BPlusLeaf();
    _root           = leaf;
    _firstLeaf      = leaf;
}

BPlusTree::~BPlusTree() {
    destroyNode(_root);
}

bool BPlusTree::isEmpty() const {
    return _root == nullptr || _root->n == 0;
}

/**
 * findLeaf
 * Desciende desde la raiz hasta la hoja correcta
 * y registra el camino de padres en el stack.
 */
BPlusLeaf* BPlusTree::findLeaf(const std::string& category) const {
    BPlusNode* curr = _root;
    while (curr->type == BPlusNodeType::INTERNAL) {
        BPlusInternal* internal = static_cast<BPlusInternal*>(curr);
        int i = 0;
        while (i < internal->n && category >= internal->keys[i]) ++i;
        curr = internal->children[i]; // i es el indice del hijo a descender: el primero con clave mayor o igual a category, o el ultimo si category es mayor que todas las claves
    }
    return static_cast<BPlusLeaf*>(curr);
}


/**
 * insert
 * Usa un stack de padres construido durante el descenso
 * para propagar splits correctamente hacia arriba.
 */
void BPlusTree::insert(const Product& p) {
    // Stack de padres en el camino de descenso
    // Maximo de niveles para n=1000 con t=4: 5 niveles
    static const int MAX_DEPTH = 32;
    BPlusInternal* parentStack[MAX_DEPTH];
    int childIdxStack[MAX_DEPTH]; // indice del hijo por el que se descendio en cada nivel
    int depth = 0;

    // Descender registrando el camino
    BPlusNode* curr = _root;
    while (curr->type == BPlusNodeType::INTERNAL) {
        BPlusInternal* internal = static_cast<BPlusInternal*>(curr);
        int i = 0;
        while (i < internal->n && p.category >= internal->keys[i]) ++i;
        parentStack[depth] = internal;
        childIdxStack[depth] = i;
        ++depth;
        curr = internal->children[i];
    }
    BPlusLeaf* leaf = static_cast<BPlusLeaf*>(curr);

    // Insertar en la hoja manteniendo orden
    insertInLeaf(leaf, p);

    // Si la hoja no se lleno, terminamos
    if (leaf->n < BPLUS_MAX_KEYS) return;

    // La hoja esta llena: dividirla
    BPlusLeaf* newLeaf = new BPlusLeaf();
    int mid = _t - 1;

    // Copiar mitad derecha a newLeaf
    for (int i = mid; i < BPLUS_MAX_KEYS; ++i) {
        newLeaf->keys[i - mid] = leaf->keys[i];
        newLeaf->data[i - mid] = leaf->data[i];
        leaf->data[i]          = nullptr;
    }
    newLeaf->n = BPLUS_MAX_KEYS - mid;
    leaf->n    = mid;

    // Mantener enlace de hojas
    newLeaf->next = leaf->next;
    leaf->next    = newLeaf;

    // Si _firstLeaf era esta hoja y la nueva va antes (no aplica aqui
    // porque insertamos en orden), no necesita actualizar _firstLeaf.
    // _firstLeaf solo cambia si se divide la primera hoja y la nueva
    // queda a la izquierda, lo cual no ocurre en este esquema.

    // Clave que sube al padre
    std::string pushKey = newLeaf->keys[0]; // la primera clave de la nueva hoja es la clave guia para el padre
    BPlusNode*  pushRight = newLeaf;
    BPlusNode*  pushLeft  = leaf;

    // Propagar el split hacia arriba usando el stack
    while (depth > 0) {
        --depth;
        BPlusInternal* parent = parentStack[depth];
        int pos = childIdxStack[depth]; // posicion donde pushKey debe ser insertada en parent

        // Insertar pushKey y pushRight en parent en la posicion pos
        for (int i = parent->n; i > pos; --i) {
            parent->keys[i] = parent->keys[i - 1];
            parent->children[i + 1] = parent->children[i];
        }
        parent->keys[pos] = pushKey;
        parent->children[pos + 1] = pushRight;
        ++parent->n;

        // Si parent no se lleno, terminamos
        if (parent->n < BPLUS_MAX_KEYS) return;

        // parent se lleno: dividirlo
        BPlusInternal* newInternal = new BPlusInternal();
        int imid = _t - 1;

        // La clave media sube; no se queda en ninguno de los dos
        std::string midKey = parent->keys[imid];

        int newN = parent->n - imid - 1;
        for (int i = 0; i < newN; ++i) { // copiar mitad derecha a newInternal
            newInternal->keys[i]     = parent->keys[imid + 1 + i];
            newInternal->children[i] = parent->children[imid + 1 + i];
        }
        newInternal->children[newN] = parent->children[parent->n];
        newInternal->n              = newN;

        // Limpiar la mitad derecha del padre
        for (int i = imid; i < parent->n; ++i) {
            parent->children[i + 1] = nullptr;
        }
        parent->n = imid;

        // Preparar para el siguiente nivel
        pushKey   = midKey;
        pushLeft  = parent;
        pushRight = newInternal;
    }

    BPlusInternal* newRoot = new BPlusInternal();
    newRoot->keys[0]     = pushKey;
    newRoot->children[0] = pushLeft;
    newRoot->children[1] = pushRight;
    newRoot->n           = 1;
    _root = newRoot;
}

/**
 * insertInLeaf
 * Inserta el producto en la hoja manteniendo el orden por category.
 */
void BPlusTree::insertInLeaf(BPlusLeaf* leaf, const Product& p) {
    int i = leaf->n - 1;
    while (i >= 0 && p.category < leaf->keys[i]) {
        leaf->keys[i + 1] = leaf->keys[i];
        leaf->data[i + 1] = leaf->data[i];
        --i;
    }
    leaf->keys[i + 1] = p.category;
    leaf->data[i + 1] = new Product(p);
    ++leaf->n;
}

void BPlusTree::searchCategory(const std::string& category, Product** results, int& count, int maxResults) const {
    count = 0;

    /**Encontrar la hoja donde empieza la categoria.
    * Con claves duplicadas, findLeaf puede apuntar a una hoja intermedia,
    * dejando entradas anteriores sin recorrer. Para garantizar que no se
    * pierda ninguna, localizamos la primera hoja que contenga la categoria
    * usando el enlace _firstLeaf y avanzando hasta encontrarla.
    */
    BPlusLeaf* leaf = _firstLeaf;
    while (leaf != nullptr) {
        // Verificar si esta hoja contiene la categoria
        bool hasCategory = false;
        for (int i = 0; i < leaf->n; ++i) {
            if (leaf->keys[i] == category) { hasCategory = true; break; }
        }
        // Early stop: si la primera clave ya supera la categoria, no existe
        if (leaf->n > 0 && leaf->keys[0] > category) break;
        if (hasCategory) break;
        leaf = leaf->next;
    }

    // Recorrer desde esa hoja mientras encontremos entradas con la categoria
    while (leaf != nullptr && count < maxResults) {
        for (int i = 0; i < leaf->n && count < maxResults; ++i) {
            if (leaf->keys[i] == category) {
                results[count++] = leaf->data[i];
            }
        }
        // Avanzar solo si la siguiente hoja puede tener la misma categoria
        if (leaf->next == nullptr) break;
        // Si todas las claves de la siguiente hoja son mayores, parar
        if (leaf->next->n > 0 && leaf->next->keys[0] > category) break;
        leaf = leaf->next;
    }
}


void BPlusTree::leafTraversal(Product** results, int& count, int maxResults) const {
    count = 0;
    BPlusLeaf* leaf = _firstLeaf;
    while (leaf != nullptr && count < maxResults) {
        for (int i = 0; i < leaf->n && count < maxResults; ++i) {
            if (leaf->data[i] != nullptr) {
                results[count++] = leaf->data[i];
            }
        }
        leaf = leaf->next;
    }
}

void BPlusTree::remove(const std::string& category) {
    BPlusLeaf* leaf = findLeaf(category);

    int pos = -1;
    for (int i = 0; i < leaf->n; ++i) {
        if (leaf->keys[i] == category) { pos = i; break; }
    }
    if (pos == -1) return;

    delete leaf->data[pos];
    for (int i = pos; i < leaf->n - 1; ++i) {
        leaf->keys[i] = leaf->keys[i + 1];
        leaf->data[i] = leaf->data[i + 1];
    }
    leaf->data[leaf->n - 1] = nullptr;
    --leaf->n;
}

// ─────────────────────────────────────────────
// toDot
// ─────────────────────────────────────────────

void BPlusTree::toDot(std::ofstream& out) const {
    out << "digraph BPlusTree {\n";
    out << "    node [shape=record fontname=\"Arial\" fontsize=10];\n";
    out << "    edge [fontsize=9];\n";
    int nodeId = 0;
    if (_root) toDotNode(_root, out, nodeId);

    // Enlazar hojas con aristas punteadas
    BPlusLeaf* leaf = _firstLeaf;
    int leafId = 0;
    while (leaf != nullptr && leaf->next != nullptr) {
        out << "    leaf" << leafId
            << " -> leaf" << leafId + 1
            << " [style=dashed color=gray];\n";
        leaf = leaf->next;
        ++leafId;
    }
    out << "}\n";
}

void BPlusTree::toDotNode(BPlusNode* node, std::ofstream& out,
                          int& nodeId) const {
    if (node == nullptr) return;
    int myId = nodeId++;

    if (node->type == BPlusNodeType::LEAF) {
        BPlusLeaf* leaf = static_cast<BPlusLeaf*>(node);
        out << "    leaf" << myId << " [label=\"";
        for (int i = 0; i < leaf->n; ++i) {
            if (i > 0) out << "|";
            out << leaf->keys[i];
        }
        out << "\" style=filled fillcolor=lightblue];\n";
    } else {
        BPlusInternal* internal = static_cast<BPlusInternal*>(node);
        out << "    node" << myId << " [label=\"";
        for (int i = 0; i < internal->n; ++i) {
            if (i > 0) out << "|";
            out << "<f" << i << "> " << internal->keys[i];
        }
        out << "\"];\n";
        for (int i = 0; i <= internal->n; ++i) {
            if (internal->children[i] == nullptr) continue;
            int childId = nodeId;
            toDotNode(internal->children[i], out, nodeId);
            out << "    node" << myId << " -> ";
            if (internal->children[i]->type == BPlusNodeType::LEAF)
                out << "leaf" << childId;
            else
                out << "node" << childId;
            out << ";\n";
        }
    }
}

void BPlusTree::destroyNode(BPlusNode* node) {
    if (node == nullptr) return;
    if (node->type == BPlusNodeType::INTERNAL) {
        BPlusInternal* internal = static_cast<BPlusInternal*>(node);
        for (int i = 0; i <= internal->n; ++i) {
            destroyNode(internal->children[i]);
        }
        delete internal;
    } else {
        BPlusLeaf* leaf = static_cast<BPlusLeaf*>(node);
        for (int i = 0; i < leaf->n; ++i) {
            delete leaf->data[i];
            leaf->data[i] = nullptr;
        }
        delete leaf;
    }
}