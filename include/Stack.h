#ifndef STACK_H
#define STACK_H

/**
 * Una pila (stack) es una estructura de datos que sigue el principio LIFO (Last In, First Out), donde el último elemento en entrar es el primero en salir.
 * En esta implementación, se utiliza una lista enlazada para mantener la eficiencia de las operaciones de inserción y eliminación, ambas con complejidad O(1). La pila tiene un puntero
 * al nodo superior y un contador de elementos para facilitar la consulta del tamaño.
 */
template<typename T>
struct SNode {
    T      data;
    SNode* next;
};

template<typename T>
class Stack {
public:
    Stack() : _top(nullptr), _size(0) {}

    ~Stack() { clear(); }

    void push(const T& item) {
        SNode<T>* node = new SNode<T>{item, _top};
        _top = node;
        ++_size;
    }

    T pop() {
        SNode<T>* tmp = _top;
        T val = tmp->data;
        _top = _top->next;
        delete tmp;
        --_size;
        return val;
    }

    T& top() const { return _top->data; }
    bool isEmpty() const { return _size == 0; }
    int getSize() const { return _size; }
    SNode<T>* getTopNode() const { return _top; }

    void clear() {
        while (_top) {
            SNode<T>* tmp = _top;
            _top = _top->next;
            delete tmp;
        }
        _size = 0;
    }

private:
    SNode<T>* _top;
    int       _size;
};

#endif // STACK_H
