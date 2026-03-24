#include "LinkedList.h"


ListNode::ListNode(const Product& p)
    : data(p), next(nullptr) {}


LinkedList::LinkedList()
    : _head(nullptr), _size(0) {}

LinkedList::~LinkedList() {
    clear();
}

// insertFront
// Crea un nuevo nodo y lo enlaza al frente de la lista.
void LinkedList::insertFront(const Product& p) {
    ListNode* node = new ListNode(p);
    node->next = _head;
    _head      = node;
    ++_size;
}

// remove
// Recorre la lista buscando el primer nodo cuyo barcode coincida.
// Reenlaza los punteros y libera el nodo encontrado.
// Si no existe, no modifica nada.
void LinkedList::remove(const std::string& barcode) {
    ListNode* prev = nullptr;
    ListNode* curr = _head;

    while (curr != nullptr) {
        if (curr->data.barcode == barcode) {
            if (prev == nullptr) {
                _head = curr->next;
            } else {
                prev->next = curr->next;
            }
            delete curr;
            --_size;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

// searchSequential
// Busqueda lineal por barcode.
// Retorna puntero al Product dentro del nodo (no una copia).
Product* LinkedList::searchSequential(const std::string& key) {
    ListNode* curr = _head;
    while (curr != nullptr) {
        if (curr->data.barcode == key) {
            return &curr->data;
        }
        curr = curr->next;
    }
    return nullptr;
}

// searchByName
// Busqueda lineal por nombre del producto.
// Necesaria para que Catalog pueda hacer rollback por nombre.
Product* LinkedList::searchByName(const std::string& name) {
    ListNode* curr = _head;
    while (curr != nullptr) {
        if (curr->data.name == name) {
            return &curr->data;
        }
        curr = curr->next;
    }
    return nullptr;
}


int LinkedList::size() const {
    return _size;
}

bool LinkedList::isEmpty() const {
    return _head == nullptr;
}

// getHead
// Acceso de solo lectura al primer nodo para recorridos externos.
const ListNode* LinkedList::getHead() const {
    return _head;
}

// clear (privado)
// Libera todos los nodos. Llamado solo por el destructor.
void LinkedList::clear() {
    ListNode* curr = _head;
    while (curr != nullptr) {
        ListNode* next = curr->next;
        delete curr;
        curr = next;
    }
    _head = nullptr;
    _size = 0;
}