#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "Product.h"

/*
 * ListNode
 * Nodo interno de la lista enlazada.
 * Almacena una copia del producto y un puntero al siguiente nodo.
 */
struct ListNode {
    Product  data;  // copia del producto almacenado
    ListNode* next; // puntero al siguiente nodo, nullptr si es el ultimo

    explicit ListNode(const Product& p);
};

/*
 * LinkedList
 * Lista enlazada simple no ordenada.
 * Insercion al frente en O(1).
 * Busqueda y eliminacion secuencial en O(n).
 * Usada como base comparativa para el benchmark de rendimiento.
 */
class LinkedList {
public:
    LinkedList();
    ~LinkedList();

    LinkedList(const LinkedList&)            = delete;
    LinkedList& operator=(const LinkedList&) = delete;

    /*
     * insertFront
     * Precondicion: p.isValid() == true.
     * Postcondicion: p insertado al frente; head apunta al nuevo nodo; _size++.
     * Complejidad: O(1)
     */
    void insertFront(const Product& p);

    /*
     * remove
     * Precondicion: lista no necesariamente vacia.
     * Postcondicion: primer nodo con data.barcode == barcode eliminado;
     *                punteros reenlazados; _size--. Si no existe, no hace nada.
     * Complejidad: O(n)
     */
    void remove(const std::string& barcode);

    /*
     * searchSequential
     * Busca por barcode recorriendo la lista nodo a nodo.
     * Precondicion: llave no vacia.
     * Postcondicion: retorna puntero al Product encontrado o nullptr.
     * Complejidad: O(n)
     */
    Product* searchSequential(const std::string& key);

    /*
     * searchByName
     * Busca por nombre recorriendo la lista nodo a nodo.
     * Precondicion: name no vacio.
     * Postcondicion: retorna puntero al Product encontrado o nullptr.
     * Complejidad: O(n)
     */
    Product* searchByName(const std::string& name);

    int size() const;

    /*
     * isEmpty
     * Postcondicion: retorna true si la lista no tiene nodos.
     */
    bool isEmpty() const;

    /*
     * getHead
     * Acceso de solo lectura al primer nodo (para recorridos externos).
     * Postcondicion: retorna puntero constante a head.
     */
    const ListNode* getHead() const;

    private:
        ListNode* _head; // puntero al primer nodo
        int       _size; // cantidad de nodos

        // Libera todos los nodos de la lista
        void clear();
    };

#endif