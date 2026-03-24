#include "SortedLinkedList.h"

SortedListNode::SortedListNode(const Product& p)
    : data(p), next(nullptr) {}

SortedLinkedList::SortedLinkedList()
    : _head(nullptr), _size(0) {}

SortedLinkedList::~SortedLinkedList() {
    clear();
}

/**
 * insertSorted
 * Busca la posicion correcta comparando nombres lexicograficamente
 * e inserta el nodo ahi para mantener el orden ascendente.
 *  */ 
void SortedLinkedList::insertSorted(const Product& p) {
    SortedListNode* newNode = new SortedListNode(p);

    // lista vacia o el nuevo nodo va al frente
    if (_head == nullptr || p.name <= _head->data.name) {
        newNode->next = _head;
        _head         = newNode;
        ++_size;
        return;
    }

    // buscar la posicion donde p.name > curr.name
    // y p.name <= curr->next.name
    SortedListNode* curr = _head;
    while (curr->next != nullptr && curr->next->data.name < p.name) {
        curr = curr->next;
    }

    newNode->next = curr->next;
    curr->next    = newNode;
    ++_size;
}

/**
 * remove
 * Busca el primer nodo cuyo name coincida y lo elimina.
 * Aprovecha el orden para detenerse en cuanto supera la clave (early stop).
 *  */ 
void SortedLinkedList::remove(const std::string& name) {
    SortedListNode* prev = nullptr;
    SortedListNode* curr = _head;

    while (curr != nullptr) {
        // Early stop: si el nombre actual ya supera la clave buscada,
        // el elemento no puede estar mas adelante (lista ordenada).
        if (curr->data.name > name) return;

        if (curr->data.name == name) {
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

/**
 * searchSequential
 * Busqueda lineal por nombre con early stop:
 * si el nodo actual supera lexicograficamente la clave, el elemento
 * no puede existir mas adelante en la lista ordenada.
 * Complejidad: O(n) peor caso, mejor promedio que LinkedList::searchSequential
 */
Product* SortedLinkedList::searchSequential(const std::string& name) {
    SortedListNode* curr = _head;
    while (curr != nullptr) {
        if (curr->data.name == name) return &curr->data;

        // Early stop: lista ordenada, no tiene sentido seguir
        if (curr->data.name > name) return nullptr;

        curr = curr->next;
    }
    return nullptr;
}

/**
 * size
 * Retorna el numero de nodos en la lista.
 */
int SortedLinkedList::size() const {
    return _size;
}

/**
 * isEmpty
 * Retorna true si la lista esta vacia, false en caso contrario.
 */
bool SortedLinkedList::isEmpty() const {
    return _head == nullptr;
}

/**
 * getHead
 * Acceso de solo lectura al primer nodo.
 */
const SortedListNode* SortedLinkedList::getHead() const {
    return _head;
}

/**
 * clear
 * Libera todos los nodos. Llamado solo por el destructor.
 */
void SortedLinkedList::clear() {
    SortedListNode* curr = _head;
    while (curr != nullptr) {
        SortedListNode* next = curr->next;
        delete curr;
        curr = next;
    }
    _head = nullptr;
    _size = 0;
}