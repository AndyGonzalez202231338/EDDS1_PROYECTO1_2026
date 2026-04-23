#include "Product.h"
#include <sstream>
#include <iomanip>

Product::Product()
    : name(""), barcode(""), category(""), expiry_date(""),
      brand(""), price(0.0), stock(0) {}

Product::Product(const std::string& name, const std::string& barcode,
                 const std::string& category, const std::string& expiry_date,
                 const std::string& brand, double price, int stock)
    : name(name), barcode(barcode), category(category),
      expiry_date(expiry_date), brand(brand), price(price), stock(stock) {}

bool Product::isValid() const {
    if (name.empty())    return false;
    if (barcode.empty()) return false;
    if (expiry_date.size() != 10 ||
        expiry_date[4] != '-'    ||
        expiry_date[7] != '-')   return false;
    try {
        int month = std::stoi(expiry_date.substr(5, 2));
        int day   = std::stoi(expiry_date.substr(8, 2));
        if (month < 1 || month > 12) return false;
        if (day   < 1 || day   > 31) return false;
    } catch (const std::exception&) {
        return false;
    }
    if (price <= 0.0) return false;
    if (stock < 0)    return false;
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

bool Product::operator==(const Product& other) const {
    return barcode == other.barcode;
}

bool Product::operator<(const Product& other) const {
    return name < other.name;
}

const std::string& Product::getBarcode() const {
    return barcode;
}