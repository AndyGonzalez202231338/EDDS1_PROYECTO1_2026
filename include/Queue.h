#ifndef QUEUE_H
#define QUEUE_H

/**
 * Una cola (queue) es una estructura de datos que sigue el principio FIFO (First In, First Out), donde el primer elemento en entrar es el primero en salir.
 * En esta implementación, se utiliza una lista enlazada para mantener la eficiencia de las operaciones de inserción y eliminación, ambas con complejidad O(1). La cola tiene un puntero
 */
template<typename T>
struct QNode {
    T      data;
    QNode* next;
    explicit QNode(const T& d) : data(d), next(nullptr) {}
};

template<typename T>
class Queue {
public:
    Queue() : _head(nullptr), _tail(nullptr), _size(0) {}

    ~Queue() {
        while (_head) {
            QNode<T>* tmp = _head;
            _head = _head->next;
            delete tmp;
        }
    }

    void enqueue(const T& item) {
        QNode<T>* node = new QNode<T>(item);
        if (_tail) _tail->next = node;
        else _head = node;
        _tail = node;
        ++_size;
    }

    T dequeue() {
        QNode<T>* tmp = _head;
        T val = tmp->data;
        _head = _head->next;
        if (!_head) _tail = nullptr;
        delete tmp;
        --_size;
        return val;
    }

    T& front() const { return _head->data; }
    bool isEmpty() const { return _size == 0; }
    int  size() const { return _size; }

private:
    QNode<T>* _head;
    QNode<T>* _tail;
    int       _size;
};

#endif // QUEUE_H
