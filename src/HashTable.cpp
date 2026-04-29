#include "HashTable.h"
#include <cstdio>

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

void HashTable::toDot(std::ofstream& out) const {
    out << "digraph HashTable {\n";
    out << "  rankdir=TB;\n";
    out << "  node [shape=box, style=\"rounded,filled\"];\n";
    out << "  edge [fontsize=10];\n\n";

    // Contadores para estadísticas
    int totalElements = 0;
    int occupiedBuckets = 0;
    int collisions = 0;

    // Recorrer todos los buckets
    for (int i = 0; i < HT_SIZE; ++i) {
        if (_buckets[i] != nullptr) {
            occupiedBuckets++;

            // Crear nodo para el bucket
            out << "  bucket_" << i << " [label=\"Bucket " << i << "\", fillcolor=\"#4A90E2\", fontcolor=white, fontweight=bold];\n";

            // Recorrer la cadena de colisiones
            HTNode* current = _buckets[i];
            int position = 0;

            while (current != nullptr) {
                totalElements++;

                // Determinar si hay colisión (si next != nullptr)
                bool hasCollision = (current->next != nullptr);
                if (position > 0) {
                    collisions++;  // Cuenta cada producto adicional como colisión
                }

                // Color del nodo: verde si no hay colisión, naranja si la hay
                std::string color = hasCollision || position > 0 ? "#FF9500" : "#28A745";
                std::string fontColor = hasCollision || position > 0 ? "white" : "white";

                // Crear ID único para cada nodo
                std::string nodeId = "bucket_" + std::to_string(i) + "_node_" + std::to_string(position);

                // Escapar caracteres especiales en el nombre del producto
                std::string productName = current->data->name;
                std::string safeName = "";
                for (char c : productName) {
                    if (c == '"' || c == '\\') {
                        safeName += '\\';
                    }
                    safeName += c;
                }

                // Crear etiqueta del producto
                out << "  " << nodeId << " [label=\"" << current->barcode << "\\n" << safeName
                    << "\", fillcolor=\"" << color << "\", fontcolor=" << fontColor << "];\n";

                // Conectar bucket con primer producto
                if (position == 0) {
                    out << "  bucket_" << i << " -> " << nodeId << " [label=\"head\"];\n";
                } else {
                    // Conectar con el producto anterior
                    std::string prevNodeId = "bucket_" + std::to_string(i) + "_node_" + std::to_string(position - 1);
                    out << "  " << prevNodeId << " -> " << nodeId << " [style=dashed];\n";
                }

                current = current->next;
                position++;
            }

            out << "\n";
        }
    }

    // Nodo de estadísticas
    out << "  stats [shape=box, label=\"";
    out << "Total de elementos: " << totalElements << "\\n";
    out << "Buckets ocupados: " << occupiedBuckets << "\\n";
    out << "Factor de carga: " << std::fixed;

    // Calcular y mostrar factor de carga con 4 decimales
    double loadFactor = static_cast<double>(totalElements) / HT_SIZE;
    char loadFactorStr[20];
    snprintf(loadFactorStr, sizeof(loadFactorStr), "%.4f", loadFactor);
    out << loadFactorStr << "\\n";

    out << "Colisiones: " << collisions << "\", fillcolor=\"#D3D3D3\", fontname=monospace];\n";

    out << "}\n";
}

