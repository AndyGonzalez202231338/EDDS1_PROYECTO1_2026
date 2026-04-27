#include "InventoryService.h"
#include <stdexcept>

InventoryService::InventoryService(BranchManager& branchManager)
    : _branchManager(branchManager) {}

bool InventoryService::insertProduct(int branchId, const Product& p) {
    std::string error;
    return insertProduct(branchId, p, error);
}

bool InventoryService::removeProduct(int branchId, const std::string& barcode) {
    std::string error;
    return removeProduct(branchId, barcode, error);
}

Product* InventoryService::searchByBarcode(int branchId, const std::string& barcode) {
    std::string error;
    return searchByBarcode(branchId, barcode, error);
}

Product* InventoryService::searchByName(int branchId, const std::string& name) {
    std::string error;
    return searchByName(branchId, name, error);
}

bool InventoryService::insertProduct(int branchId, const Product& p, std::string& error) {
    Branch* branch = _branchManager.findBranch(branchId);
    if (!branch) {
        error = "Branch no existe";
        return false;
    }
    if (!branch->insertProduct(p)) {
        error = "Producto duplicado o error al insertar";
        return false;
    }
    return true;
}

bool InventoryService::removeProduct(int branchId, const std::string& barcode, std::string& error) {
    Branch* branch = _branchManager.findBranch(branchId);
    if (!branch) {
        error = "Branch no existe";
        return false;
    }
    if (!branch->removeProduct(barcode)) {
        error = "Producto no encontrado";
        return false;
    }
    return true;
}

Product* InventoryService::searchByBarcode(int branchId, const std::string& barcode, std::string& error) {
    Branch* branch = _branchManager.findBranch(branchId);
    if (!branch) {
        error = "Branch no existe";
        return nullptr;
    }
    return branch->searchByBarcode(barcode);
}

Product* InventoryService::searchByName(int branchId, const std::string& name, std::string& error) {
    Branch* branch = _branchManager.findBranch(branchId);
    if (!branch) {
        error = "Branch no existe";
        return nullptr;
    }
    return branch->searchByName(name);
}

