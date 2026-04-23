#include "HashTable.h"

HTNode::HTNode(const std::string& bc, Product* p)
    : barcode(bc), data(p), next(nullptr) {}

HashTable::HashTable() : _size(0) {
    for (int i = 0; i < HT_SIZE; ++i) {
        _buckets[i] = nullptr;
    }
}

HashTable::~HashTable() {
    for (int i = 0; i < HT_SIZE; ++i) {
        HTNode* current = _buckets[i];
        while (current != nullptr) {
            HTNode* toDelete = current;
            current = current->next;

            delete toDelete->data;
            delete toDelete;
        }
        _buckets[i] = nullptr;
    }
    _size = 0;
}

void HashTable::insert(const Product& p) {
    const std::string barcode = p.getBarcode();
    int index = hashFunction(barcode);

    // Si ya existe, reemplaza el producto almacenado
    HTNode* current = _buckets[index];
    while (current != nullptr) {
        if (current->barcode == barcode) {
            delete current->data;
            current->data = new Product(p);
            return;
        }
        current = current->next;
    }

    // Inserción al inicio de la lista (encadenamiento)
    HTNode* newNode = new HTNode(barcode, new Product(p));
    newNode->next = _buckets[index];
    _buckets[index] = newNode;
    ++_size;
}

Product* HashTable::search(const std::string& barcode) const {
    int index = hashFunction(barcode);
    HTNode* current = _buckets[index];

    while (current != nullptr) {
        if (current->barcode == barcode) {
            return current->data;
        }
        current = current->next;
    }
    return nullptr;
}

void HashTable::remove(const std::string& barcode) {
    int index = hashFunction(barcode);
    HTNode* current = _buckets[index];
    HTNode* prev = nullptr;

    while (current != nullptr) {
        if (current->barcode == barcode) {
            if (prev == nullptr) {
                _buckets[index] = current->next;
            } else {
                prev->next = current->next;
            }

            delete current->data;
            delete current;
            --_size;
            return;
        }
        prev = current;
        current = current->next;
    }
}

bool HashTable::isEmpty() const {
    return _size == 0;
}

int HashTable::hashFunction(const std::string& barcode) const {
    // djb2
    unsigned long hash = 5381;
    for (char c : barcode) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(c); // hash * 33 + c
    }
    return static_cast<int>(hash % HT_SIZE);
}
