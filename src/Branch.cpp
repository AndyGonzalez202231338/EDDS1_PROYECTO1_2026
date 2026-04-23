#include "Branch.h"

Branch::Branch(int id, const std::string& name, const std::string& location,
               int t_ingreso, int t_traspaso, int t_despacho)
    : id(id), name(name), location(location),
      t_ingreso(t_ingreso), t_traspaso(t_traspaso), t_despacho(t_despacho),
      _catalog("errors_branch_" + std::to_string(id) + ".log") {}

Catalog& Branch::getCatalog() { return _catalog; }

bool Branch::addProduct(const Product& p) { return _catalog.addProduct(p); }

bool Branch::removeProduct(const std::string& barcode) { return _catalog.removeProduct(barcode); }

Product* Branch::findByBarcode(const std::string& barcode) { return _catalog.findByBarcode(barcode); }

void Branch::receiveProduct(Product* p)  { _colaIngreso.enqueue(p); }

void Branch::prepareTransfer(Product* p) { _colaTraspaso.enqueue(p); }

void Branch::dispatchProduct() {
    if (!_colaTraspaso.isEmpty())
        _colaSalida.enqueue(_colaTraspaso.dequeue());
}

Product* Branch::getNextDispatch() {
    if (_colaSalida.isEmpty()) return nullptr;
    return _colaSalida.dequeue();
}

void Branch::pushRollback(const TransferRecord& r) { _rollbackStack.push(r); }

TransferRecord Branch::popRollback() { return _rollbackStack.pop(); }

bool Branch::hasRollback() const { return !_rollbackStack.isEmpty(); }
