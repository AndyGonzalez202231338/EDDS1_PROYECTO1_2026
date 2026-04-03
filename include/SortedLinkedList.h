#ifndef SORTEDLINKEDLIST_H
#define SORTEDLINKEDLIST_H

#include "Product.h"

/*
 * SortedListNode
 * Nodo interno de la lista enlazada ordenada.
 * Estructura identica a ListNode pero es una clase independiente de LinkedList.
 */
struct SortedListNode {
    Product       data; // copia del producto almacenado
    SortedListNode* next; // puntero al siguiente nodo

    explicit SortedListNode(const Product& p);
};

/*
 * SortedLinkedList
 * Lista enlazada simple ordenada alfabeticamente por Product.name.
 * Clase independiente de LinkedList (sin herencia).
 * La insercion mantiene el orden en O(n).
 * La busqueda puede detener el recorrido antes (early stop) si
 * el nodo actual supera lexicograficamente la clave buscada,
 * aunque el peor caso sigue siendo O(n).
 */
class SortedLinkedList {
public:
    SortedLinkedList();
    ~SortedLinkedList();

    SortedLinkedList(const SortedLinkedList&)            = delete;
    SortedLinkedList& operator=(const SortedLinkedList&) = delete;

    /*
     * insertSorted
     * Precondicion: p.isValid() == true.
     * Postcondicion: p insertado en la posicion correcta para mantener
     *                el orden alfabetico por name; _size++.
     * Complejidad: O(n)
     */
    void insertSorted(const Product& p);

    /*
     * remove
     * Precondicion: lista no necesariamente vacia.
     * Postcondicion: primer nodo con data.name == name eliminado;
     *                orden conservado; _size--. Si no existe, no hace nada.
     * Complejidad: O(n)
     */
    void remove(const std::string& name);

    /*
     * searchSequential
     * Busca por nombre con early stop: si nodo.name > name, no existe.
     * Precondicion: name no vacio.
     * Postcondicion: retorna puntero al Product encontrado o nullptr.
     * Complejidad: O(n) peor caso, mejor en listas casi completas.
     */
    Product* searchSequential(const std::string& name);

    int size() const;

    /*
     * isEmpty
     * Postcondicion: retorna true si la lista no tiene nodos.
     */
    bool isEmpty() const;

    /*
     * getHead
     * Acceso de solo lectura al primer nodo.
     * Postcondicion: retorna puntero constante a head.
     * Complejidad: O(1)
     */
    const SortedListNode* getHead() const;
    void insertFront(const Product& p);
    void sortInPlace();

private:
    SortedListNode* _head; // puntero al primer nodo (menor alfabeticamente)
    int             _size; // cantidad de nodos

    // Libera todos los nodos de la lista
    void clear();
};

#endif