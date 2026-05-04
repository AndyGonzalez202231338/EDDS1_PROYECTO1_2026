#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "Product.h"
#include <string>
#include <fstream>
/**
 * Para un codigo de 10 caracteres, el numero de combinaciones posibles es 36^10 (26 letras + 10 digitos).
 * Esto da un total de 3.656.158.440.062.976 combinaciones posibles, lo cual es un espacio de direcciones
 * suficientemente grande para evitar colisiones frecuentes en una tabla hash con un tamaño de 1024.
 * Sin embargo, es importante implementar una buena función hash para distribuir uniformemente las claves
 * y minimizar las colisiones.
 */
static const int HT_SIZE = 1024;  // tamaño de la tabla


struct HTNode {
    std::string barcode;   // clave
    Product*    data;      // puntero al producto
    HTNode*     next;      // encadenamiento para colisiones
    HTNode(const std::string& bc, Product* p);
};

class HashTable {
public:
    HashTable();
    ~HashTable();

    void     insert(const Product& p);
    Product* search(const std::string& barcode) const;
    void     remove(const std::string& barcode);
    bool     isEmpty() const;
    void     toDot(std::ofstream& out) const;

private:
    HTNode* _buckets[HT_SIZE];  // arreglo de punteros a listas de colisiones
    int     _size;

    int hashFunction(const std::string& barcode) const;
};

#endif // HASH_TABLE_H