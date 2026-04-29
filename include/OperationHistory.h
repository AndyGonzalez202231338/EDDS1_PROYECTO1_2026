#ifndef OPERATION_HISTORY_H
#define OPERATION_HISTORY_H

#include "Product.h"
#include <string>

enum class OpType { INSERT, REMOVE };

struct HistoryEntry {
    OpType      type;
    int         branchId;
    Product     product;      // copia completa del producto
    std::string description;  // texto legible para mostrar en UI

    HistoryEntry(OpType t, int bid, const Product& p, const std::string& desc)
        : type(t), branchId(bid), product(p), description(desc) {}
};

#endif // OPERATION_HISTORY_H
