#ifndef INVENTORY_SERVICE_H
#define INVENTORY_SERVICE_H

#include <string>
#include <vector>
#include "BranchManager.h"
#include "Product.h"

class InventoryService {
public:
    explicit InventoryService(BranchManager& branchManager);

    // Requeridos
    bool insertProduct(int branchId, const Product& p);
    bool removeProduct(int branchId, const std::string& barcode);
    Product* searchByBarcode(int branchId, const std::string& barcode);
    Product* searchByName(int branchId, const std::string& name);

    // Útiles para API (error explícito)
    bool insertProduct(int branchId, const Product& p, std::string& error);
    bool removeProduct(int branchId, const std::string& barcode, std::string& error);
    Product* searchByBarcode(int branchId, const std::string& barcode, std::string& error);
    Product* searchByName(int branchId, const std::string& name, std::string& error);
    std::vector<Product> listProducts(int branchId, std::string& error);

private:
    BranchManager& _branchManager;
};

#endif