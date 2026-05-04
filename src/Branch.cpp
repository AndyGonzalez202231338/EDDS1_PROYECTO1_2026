#include "Branch.h"

Branch::Branch()
    : _id(-1), _nombre(""), _ubicacion(""),
      _tiempoIngreso(0), _tiempoPreparacion(0), _intervaloDespacho(0) {}

Branch::Branch(int id,
               const std::string& nombre,
               const std::string& ubicacion,
               int tiempoIngreso,
               int tiempoPreparacion,
               int intervaloDespacho)
    : _id(id), _nombre(nombre), _ubicacion(ubicacion),
      _tiempoIngreso(tiempoIngreso),
      _tiempoPreparacion(tiempoPreparacion),
      _intervaloDespacho(intervaloDespacho) {}

int Branch::getId() const { return _id; }
const std::string& Branch::getNombre() const { return _nombre; }
const std::string& Branch::getUbicacion() const { return _ubicacion; }
int Branch::getTiempoIngreso() const { return _tiempoIngreso; }
int Branch::getTiempoPreparacion() const { return _tiempoPreparacion; }
int Branch::getIntervaloDespacho() const { return _intervaloDespacho; }

bool Branch::insertProduct(const Product& p) {
    std::lock_guard<std::mutex> lk(_mtx);
    if (!p.isValid()) return false;
    if (_hash.search(p.barcode) != nullptr) return false;

    Product local = p;
    local.branchId = _id;

    bool inList=false, inSorted=false, inAVL=false;
    bool inBTree=false, inBPlus=false, inHash=false;

    try {
        _list.insertFront(local);        inList = true;
        _sortedList.insertSorted(local); inSorted = true;
        _avl.insert(local);              inAVL = true;
        _btree.insert(local);            inBTree = true;
        _bplus.insert(local);            inBPlus = true;
        _hash.insert(local);             inHash = true;
    } catch (...) {
        rollbackInsert(local, inList, inSorted, inAVL, inBTree, inBPlus, inHash);
        return false;
    }
    return true;
}

bool Branch::removeProduct(const std::string& barcode) {
    std::lock_guard<std::mutex> lk(_mtx);
    Product* existing = _hash.search(barcode);
    if (existing == nullptr) return false;

    const std::string name     = existing->name;
    const std::string date     = existing->expiry_date;
    const std::string category = existing->category;

    _hash.remove(barcode);
    _bplus.remove(category);
    _btree.remove(date);
    _avl.remove(name);
    _sortedList.remove(name);
    _list.remove(barcode);

    return true;
}

Product* Branch::searchByBarcode(const std::string& barcode) {
    std::lock_guard<std::mutex> lk(_mtx);
    return _hash.search(barcode);
}

Product* Branch::searchByName(const std::string& name) {
    std::lock_guard<std::mutex> lk(_mtx);
    return _avl.search(name);
}

void Branch::searchByCategory(const std::string& category, Product** results, int& count, int maxResults) const {
    std::lock_guard<std::mutex> lk(_mtx);
    _bplus.searchCategory(category, results, count, maxResults);
}

void Branch::searchByDateRange(const std::string& d1, const std::string& d2, Product** results, int& count, int maxResults) const {
    std::lock_guard<std::mutex> lk(_mtx);
    _btree.rangeSearch(d1, d2, results, count, maxResults);
}

bool Branch::isInventoryEmpty() const {
    std::lock_guard<std::mutex> lk(_mtx);
    return _hash.isEmpty();
}

void Branch::getProducts(const std::function<void(const Product&)>& fn) const {
    std::lock_guard<std::mutex> lk(_mtx);
    const ListNode* node = _list.getHead();
    while (node != nullptr) {
        fn(node->data);
        node = node->next;
    }
}

void Branch::enqueueIngreso(const Product& p)    { std::lock_guard<std::mutex> lk(_mtx); queueIngreso.enqueue(p); }
void Branch::dequeueIngreso()                    { std::lock_guard<std::mutex> lk(_mtx); queueIngreso.dequeue(); }
void Branch::enqueuePreparacion(const Product& p){ std::lock_guard<std::mutex> lk(_mtx); queuePreparacion.enqueue(p); }
void Branch::dequeuePreparacion()                { std::lock_guard<std::mutex> lk(_mtx); queuePreparacion.dequeue(); }
void Branch::enqueueSalida(const Product& p)     { std::lock_guard<std::mutex> lk(_mtx); queueSalida.enqueue(p); }
void Branch::dequeueSalida()                     { std::lock_guard<std::mutex> lk(_mtx); queueSalida.dequeue(); }

void Branch::rollbackInsert(const Product& p,
                            bool inList, bool inSorted, bool inAVL,
                            bool inBTree, bool inBPlus, bool inHash) {
    if (inHash)   _hash.remove(p.barcode);
    if (inBPlus)  _bplus.remove(p.category);
    if (inBTree)  _btree.remove(p.expiry_date);
    if (inAVL)    _avl.remove(p.name);
    if (inSorted) _sortedList.remove(p.name);
    if (inList)   _list.remove(p.barcode);
}
