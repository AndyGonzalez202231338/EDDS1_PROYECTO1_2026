#include "Product.h"
#include <sstream>
#include <iomanip>

// Constructor por defecto, inicializa campos a valores vacios o cero.
Product::Product(): name(""), barcode(""), category(""), expiry_date(""), brand(""), price(0.0), stock(0) {}

// Constructor con todos los campos
Product::Product(const std::string& name, const std::string& barcode, const std::string& category, const std::string& expiry_date,
                 const std::string& brand, double price, int stock)
    : name(name), barcode(barcode), category(category), expiry_date(expiry_date), brand(brand), price(price), stock(stock) {}

bool Product::isValid() const {
    if (name.empty())     return false;
    if (barcode.empty())  return false;
    if (price <= 0.0)     return false;
    if (stock < 0)        return false;
    return true;
}

std::string Product::toString() const {
    std::ostringstream oss;
    oss << "Nombre     : " << name        << "\n"
        << "Codigo     : " << barcode     << "\n"
        << "Categoria  : " << category    << "\n"
        << "Caducidad  : " << expiry_date << "\n"
        << "Marca      : " << brand       << "\n"
        << "Precio     : " << std::fixed << std::setprecision(2) << price << "\n"
        << "Stock      : " << stock;
    return oss.str();
}


// Dos productos son iguales si tienen el mismo barcode (clave unica del sistema).
bool Product::operator==(const Product& other) const {
    return barcode == other.barcode;
}

// operator<
// Comparacion por nombre para ordenamiento en AVL y SortedLinkedList.
bool Product::operator<(const Product& other) const {
    return name < other.name;
}