#include "Menu.h"
#include "Benchmark.h"
#include <iostream>
#include <limits>
#include <string>

#define BG_ROJO     "\033[41m"
#define ROJO_BRILLANTE  "\033[91m"
#define BG_AZUL     "\033[44m"
#define AZUL_BRILLANTE  "\033[94m"
#define RESET   "\033[0m"
#define BG_AMARILLO "\033[43m"
#define AMAR_BRILLANTE  "\033[93m"
#define BG_VERDE    "\033[42m"
#define VERDE_BRILLANTE "\033[92m"

Menu::Menu(Catalog& catalog)
    : _catalog(catalog), _running(true) {}

void Menu::run() {
    clearScreen();
    std::cout << AMAR_BRILLANTE << "======================================\n"<< RESET;
    std::cout << BG_AMARILLO    <<"  CATALOGO DE SUPERMERCADO - EDD 2026\n" << RESET;
    std::cout << AMAR_BRILLANTE << "======================================\n" << RESET;

    while (_running) {
        showMainMenu();
        int opt = readOption();
        handleOption(opt);
    }
}

void Menu::showMainMenu() const {
    std::cout << "\n";
    printSeparator();
    std::cout << "  1. Agregar producto\n";
    std::cout << "  2. Buscar por nombre\n";
    std::cout << "  3. Buscar por codigo de barra\n";
    std::cout << "  4. Buscar por categoria\n";
    std::cout << "  5. Buscar por rango de fechas\n";
    std::cout << "  6. Eliminar producto\n";
    std::cout << "  7. Listar todos (orden alfabetico)\n";
    std::cout << "  8. Cargar CSV\n";
    std::cout << "  9. Ejecutar benchmark\n";
    std::cout << " 10. Generar archivos .dot\n";
    std::cout << "  0. Salir\n";
    printSeparator();
    std::cout << "  Opcion: ";
}

void Menu::printSeparator() const {
    std::cout << VERDE_BRILLANTE << "--------------------------------------\n" << RESET;
}

void Menu::clearScreen() const {
#ifdef _WIN32 // Para Windows
    system("cls");
#else
    system("clear"); // Para Unix/Linux/Mac
#endif
}

// Lectura entrada con validacion
int Menu::readOption() const {
    int opt;
    while (!(std::cin >> opt)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Entrada invalida. Opcion: ";
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return opt;
}

// Lectura de campos con validacion
std::string Menu::readString(const std::string& prompt) const {
    std::cout << "  " << prompt << ": ";
    std::string value;
    std::getline(std::cin, value);
    return value;
}

// Lectura de double con validacion
double Menu::readDouble(const std::string& prompt) const {
    double value;
    std::cout << "  " << prompt << ": ";
    while (!(std::cin >> value) || value <= 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Valor invalido (debe ser > 0). " << prompt << ": ";
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return value;
}

// Lectura de int con validacion
int Menu::readInt(const std::string& prompt) const {
    int value;
    std::cout << "  " << prompt << ": ";
    while (!(std::cin >> value) || value < 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Valor invalido (debe ser >= 0). " << prompt << ": ";
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return value;
}

void Menu::handleOption(int opt) {
    switch (opt) {
        case 1:  actionAddProduct();     break;
        case 2:  actionFindByName();     break;
        case 3:  actionFindByBarcode();  break;
        case 4:  actionFindByCategory(); break;
        case 5:  actionFindByDateRange();break;
        case 6:  actionRemoveProduct();  break;
        case 7:  actionListAllByName();  break;
        case 8:  actionLoadCSV();        break;
        case 9:  actionBenchmark();      break;
        case 10: actionGenerateDot();    break;
        case 0:
            std::cout << "\n  Saliendo.\n";
            _running = false;
            break;
        default:
            std::cout << "\n  Opcion invalida. Intente de nuevo.\n";
    }
}

// Acciones
void Menu::actionAddProduct() {
    std::cout << "\n-- Agregar producto --\n";
    Product p = promptProduct();
    if (!p.isValid()) {
        std::cout << "  Producto invalido. Verifique los campos.\n";
        return;
    }
    if (_catalog.addProduct(p)) {
        std::cout << "  Producto agregado correctamente.\n";
    } else {
        std::cout << "  No se pudo agregar. Puede ser un barcode duplicado.\n";
    }
}

void Menu::actionFindByName() {
    std::cout << "\n-- Buscar por nombre --\n";
    std::string name = readString("Nombre");
    Product* p = _catalog.findByName(name);
    if (p) {
        std::cout << "\n";
        printProduct(*p);
    } else {
        std::cout << "  Producto no encontrado.\n";
    }
}

void Menu::actionFindByBarcode() {
    std::cout << "\n-- Buscar por codigo de barra --\n";
    std::string bc = readString("Codigo de barra");
    Product* p = _catalog.findByBarcode(bc);
    if (p) {
        std::cout << "\n";
        printProduct(*p);
    } else {
        std::cout << "  Producto no encontrado.\n";
    }
}

// Buscar por categoria y rango de fechas requieren manejo de arrays de resultados
void Menu::actionFindByCategory() {
    std::cout << "\n-- Buscar por categoria --\n";
    std::string cat = readString("Categoria");

    Product* results[MAX_RESULTS];
    int count = 0;
    _catalog.findByCategory(cat, results, count);

    if (count == 0) {
        std::cout << "  No se encontraron productos en esa categoria.\n";
        return;
    }
    std::cout << "\n  " << count << " producto(s) en categoria \"" << cat << "\":\n";
    printSeparator();
    for (int i = 0; i < count; ++i) {
        printProduct(*results[i]);
        if (i < count - 1) printSeparator();
    }
}

// Buscar por rango de fechas
void Menu::actionFindByDateRange() {
    std::cout << "\n-- Buscar por rango de fechas (YYYY-MM-DD) --\n";
    std::string d1 = readString("Fecha inicio");
    std::string d2 = readString("Fecha fin");

    if (d1 > d2) {
        std::cout << "  Error: la fecha inicio debe ser <= fecha fin.\n";
        return;
    }

    Product* results[MAX_RESULTS];
    int count = 0;
    _catalog.findByDateRange(d1, d2, results, count);

    if (count == 0) {
        std::cout << "  No se encontraron productos en ese rango.\n";
        return;
    }
    std::cout << "\n  " << count << " producto(s) con caducidad en ["
    << d1 << ", " << d2 << "]:\n";
    printSeparator();
    for (int i = 0; i < count; ++i) {
        printProduct(*results[i]);
        if (i < count - 1) printSeparator();
    }
}

void Menu::actionRemoveProduct() {
    std::cout << "\n-- Eliminar producto --\n";
    std::string bc = readString("Codigo de barra");
    if (_catalog.removeProduct(bc)) {
        std::cout << "  Producto eliminado de todas las estructuras.\n";
    } else {
        std::cout << "  Producto no encontrado.\n";
    }
}

void Menu::actionListAllByName() {
    std::cout << "\n-- Listado alfabetico --\n";
    _catalog.listAllByName();
}

void Menu::actionLoadCSV() {
    std::cout << "\n-- Cargar desde CSV --\n";
    std::string path = readString("Ruta del archivo");
    if (!_catalog.loadFromCSV(path)) {
        std::cout << "  No se pudo abrir el archivo.\n";
    }
}

void Menu::actionBenchmark() {
    std::cout << "\n-- Benchmark de busqueda --\n";
    Benchmark bench(_catalog, 20, 5);
    bench.run();
    bench.reportResults();
}

void Menu::actionGenerateDot() {
    std::cout << "\n-- Generar archivos .dot --\n";
    // Crear directorio output si no existe
    system("mkdir -p output");
    _catalog.generateDotFiles();
    std::cout << "  Archivos generados en output/\n";
    std::cout << "  Para convertir a PNG:\n";
    std::cout << "  dot -Tpng output/avl.dot -o output/avl.png\n";
    std::cout << "  dot -Tpng output/btree.dot -o output/btree.png\n";
    std::cout << "  dot -Tpng output/bplustree.dot -o output/bplustree.png\n";
}

Product Menu::promptProduct() const {
    std::string name     = readString("Nombre");
    std::string barcode  = readString("Codigo de barra");
    std::string category = readString("Categoria");
    std::string expiry   = readString("Fecha caducidad (YYYY-MM-DD)");
    std::string brand    = readString("Marca");
    double price         = readDouble("Precio");
    int    stock         = readInt("Stock");
    return Product(name, barcode, category, expiry, brand, price, stock);
}

// printProduct
void Menu::printProduct(const Product& p) const {
    std::cout << p.toString() << "\n";
}