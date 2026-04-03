#include "Menu.h"
#include "Benchmark.h"
#include <iostream>
#include <limits>
#include <string>
#include <iomanip>
#include <sstream>
#include <functional>

#define RST     "\033[0m"
#define BOLD    "\033[1m"
#define CRED    "\033[91m"
#define CGRN    "\033[92m"
#define CYEL    "\033[93m"
#define CBLU    "\033[94m"
#define CMAG    "\033[95m"
#define CCYN    "\033[96m"
#define CWHT    "\033[97m"
#define BYEL    "\033[43m"

Menu::Menu(Catalog& catalog)
    : _catalog(catalog), _running(true) {}

void Menu::run() {
    clearScreen();
    std::cout << BOLD << CYEL
              << "  ╔══════════════════════════════════════╗\n"
              << "  ║   CATALOGO DE SUPERMERCADO EDD 2026  ║\n"
              << "  ╚══════════════════════════════════════╝\n"
              << RST;

    while (_running) {
        showMainMenu();
        int opt = readOption();
        handleOption(opt);
    }
}

void Menu::showMainMenu() const {
    std::cout << "\n";
    printSeparator();
    std::cout << CBLU << BOLD << "  MENU PRINCIPAL\n" << RST;
    printSeparator();
    std::cout << CGRN << "  1. " << RST << "Agregar producto\n";
    std::cout << CGRN << "  2. " << RST << "Buscar por nombre\n";
    std::cout << CGRN << "  3. " << RST << "Buscar por codigo de barra\n";
    std::cout << CGRN << "  4. " << RST << "Buscar por categoria\n";
    std::cout << CGRN << "  5. " << RST << "Buscar por rango de fechas\n";
    std::cout << CYEL << "  6. " << RST << "Eliminar producto\n";
    std::cout << CCYN << "  7. " << RST << "Listar todos (orden alfabetico)\n";
    std::cout << CCYN << "  8. " << RST << "Cargar CSV\n";
    std::cout << CMAG << "  9. " << RST << "Ejecutar benchmark\n";
    std::cout << CMAG << " 10. " << RST << "Generar archivos .dot y PNG\n";
    std::cout << CRED << "  0. " << RST << "Salir\n";
    printSeparator();
    std::cout << BOLD << CWHT << "  Opcion: " << RST;
}

void Menu::printSeparator() const {
    std::cout << CBLU << "  ──────────────────────────────────────\n" << RST;
}

void Menu::printThinSeparator() const {
    std::cout << CCYN << "  ......................................\n" << RST;
}

void Menu::clearScreen() const {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

int Menu::readOption() const {
    int opt;
    while (!(std::cin >> opt)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << CRED << "  Entrada invalida. Opcion: " << RST;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return opt;
}

std::string Menu::readString(const std::string& prompt) const {
    std::cout << CCYN << "  " << prompt << ": " << RST;
    std::string value;
    std::getline(std::cin, value);
    return value;
}

double Menu::readDouble(const std::string& prompt) const {
    double value;
    std::cout << CCYN << "  " << prompt << ": " << RST;
    while (!(std::cin >> value) || value <= 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << CRED << "  Valor invalido (debe ser > 0). "
                  << prompt << ": " << RST;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return value;
}

int Menu::readInt(const std::string& prompt) const {
    int value;
    std::cout << CCYN << "  " << prompt << ": " << RST;
    while (!(std::cin >> value) || value < 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << CRED << "  Valor invalido (>= 0). " << prompt << ": " << RST;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return value;
}

void Menu::handleOption(int opt) {
    switch (opt) {
        case 1:  actionAddProduct();      break;
        case 2:  actionFindByName();      break;
        case 3:  actionFindByBarcode();   break;
        case 4:  actionFindByCategory();  break;
        case 5:  actionFindByDateRange(); break;
        case 6:  actionRemoveProduct();   break;
        case 7:  actionListAllByName();   break;
        case 8:  actionLoadCSV();         break;
        case 9:  actionBenchmark();       break;
        case 10: actionGenerateDot();     break;
        case 0:
            std::cout << "\n" << CYEL << "  Hasta luego.\n" << RST;
            _running = false;
            break;
        default:
            std::cout << CRED << "\n  Opcion invalida. Intente de nuevo.\n" << RST;
    }
}

void Menu::actionAddProduct() {
    std::cout << "\n" << BOLD << CBLU << "-- Agregar producto --\n" << RST;
    Product p = promptProduct();
    if (!p.isValid()) {
        std::cout << CRED << "  Producto invalido. Verifique los campos.\n" << RST;
        return;
    }
    if (_catalog.addProduct(p)) {
        std::cout << CGRN << "  Producto agregado correctamente.\n" << RST;
    } else {
        std::cout << CRED << "  No se pudo agregar. Puede ser un barcode duplicado.\n" << RST;
    }
}

void Menu::actionFindByName() {
    std::cout << "\n" << BOLD << CBLU << "-- Buscar por nombre --\n" << RST;
    std::string name = readString("Nombre");
    Product* p = _catalog.findByName(name);
    if (p) { std::cout << "\n"; printProduct(*p); }
    else std::cout << CRED << "  Producto no encontrado.\n" << RST;
}

void Menu::actionFindByBarcode() {
    std::cout << "\n" << BOLD << CBLU << "-- Buscar por codigo de barra --\n" << RST;
    std::string bc = readString("Codigo de barra");
    Product* p = _catalog.findByBarcode(bc);
    if (p) { std::cout << "\n"; printProduct(*p); }
    else std::cout << CRED << "  Producto no encontrado.\n" << RST;
}

/**
 * printTablePaged
 * Muestra products en tabla de 3 columnas con todos
 * los campos visibles y paginacion S/A/Q.
 * Cada "fila visual" ocupa N_LINES lineas de terminal
 * (una por campo del producto + separador).
 */
void Menu::printTablePaged(Product** results, int count, const std::string& title) const {
    if (count == 0) {
        std::cout << CYEL << "  No hay productos para mostrar.\n" << RST;
        return;
    }

    const int COLS    = 3;   // productos por fila
    const int PAGE    = 5;   // filas por pagina (5 filas x 3 productos = 15 por pagina)
    const int CW      = 40;  // ancho de cada columna en caracteres

    int totalFilas = (count + COLS - 1) / COLS;
    int totalPags  = (totalFilas + PAGE - 1) / PAGE;
    int pag        = 0;

    // Trunca un string al ancho de columna disponible
    auto trunc = [&](const std::string& s) -> std::string {
        int w = CW - 2;
        return (static_cast<int>(s.size()) <= w) ? s : s.substr(0, w - 2) + "..";
    };

    // Imprime una linea de la tabla: etiqueta + valor para cada producto de la fila
    auto imprimirLinea = [&](const char* colorEtiq, const char* etiq, const char* colorVal, int fila, std::function<std::string(const Product&)> campo) {
        std::cout << "  ";
        for (int c = 0; c < COLS; ++c) {
            int idx = fila * COLS + c;
            if (idx < count) {
                std::string val = trunc(campo(*results[idx]));
                // Etiqueta
                std::cout << colorEtiq << BOLD
                          << std::left << std::setw(11) << etiq
                          << RST << ": "
                          << colorVal
                          << std::left << std::setw(CW - 13) << val
                          << RST;
            } else {
                std::cout << std::setw(CW) << " ";
            }
        }
        std::cout << "\n";
    };

    while (true) {
        clearScreen();

        int fIni = pag * PAGE;
        int fFin = std::min(fIni + PAGE, totalFilas);
        int pIni = fIni * COLS + 1;
        int pFin = std::min(fFin * COLS, count);

        // Titulo y estado de pagina
        std::cout << BOLD << CYEL
                  << "  " << title
                  << " | " << pIni << "-" << pFin
                  << " de " << count << " productos"
                  << " | Pag " << pag + 1 << "/" << totalPags
                  << "\n" << RST;
        printSeparator();

        // Cabecera de columnas
        std::cout << BOLD << BYEL << "  ";
        for (int c = 0; c < COLS; ++c) {
            int idx = fIni * COLS + c;
            if (idx < count)
                std::cout << std::left << std::setw(CW)
                          << ("  #" + std::to_string(idx + 1) + " " + trunc(results[idx]->name));
            else
                std::cout << std::setw(CW) << " ";
        }
        std::cout << RST << "\n";

        // Divisor
        std::cout << CBLU << "  " << std::string(COLS * CW, '-') << RST << "\n";

        for (int f = fIni; f < fFin; ++f) {
            // Numero de producto en cada columna como subencabezado
            std::cout << BOLD << BYEL << "  ";
            for (int c = 0; c < COLS; ++c) {
                int idx = f * COLS + c;
                if (idx < count)
                    std::cout << std::left << std::setw(CW)
                              << ("  #" + std::to_string(idx + 1) + " " +
                                  trunc(results[idx]->name));
                else
                    std::cout << std::setw(CW) << " ";
            }
            std::cout << RST << "\n";

            // Campos del producto
            imprimirLinea(CWHT, "  Codigo",    CWHT, f,
                [](const Product& p){ return p.barcode; });
            imprimirLinea(CWHT, "  Categoria", CMAG, f,
                [](const Product& p){ return p.category; });
            imprimirLinea(CWHT, "  Caducidad", CYEL, f,
                [](const Product& p){ return p.expiry_date; });
            imprimirLinea(CWHT, "  Marca",     CWHT, f,
                [](const Product& p){ return p.brand; });
            imprimirLinea(CWHT, "  Precio",    CGRN, f,
                [](const Product& p){
                    std::ostringstream ss;
                    ss << "$" << std::fixed << std::setprecision(2) << p.price;
                    return ss.str();
                });
            imprimirLinea(CWHT, "  Stock",     CWHT, f,
                [](const Product& p){ return std::to_string(p.stock); });

            // Separador entre filas de productos
            std::cout << CBLU << "  " << std::string(COLS * CW, '.') << RST << "\n";
        }

        printSeparator();

        // Navegacion
        if (totalPags > 1) {
            std::cout << CGRN << "  [S]siguiente" << RST << "  "
                      << CYEL << "[A]anterior"   << RST << "  "
                      << CRED << "[Q]volver"      << RST << "\n";
            std::cout << BOLD << "  Opcion: " << RST;
            std::string nav; std::getline(std::cin, nav);
            if      (nav == "s" || nav == "S") { if (pag + 1 < totalPags) ++pag; }
            else if (nav == "a" || nav == "A") { if (pag > 0) --pag; }
            else break;
        } else {
            std::cout << CRED << "  [Q]volver" << RST << "\n";
            std::cout << BOLD << "  Opcion: " << RST;
            std::string nav; std::getline(std::cin, nav);
            break;
        }
    }
}

void Menu::actionFindByCategory() {
    std::cout << "\n" << BOLD << CBLU << "-- Buscar por categoria --\n" << RST;
    std::string cat = readString("Categoria");

    Product* results[MAX_RESULTS];
    int count = 0;
    _catalog.findByCategory(cat, results, count);

    if (count == 0) {
        std::cout << CYEL << "  No se encontraron productos en esa categoria.\n" << RST;
        return;
    }
    printTablePaged(results, count,
                    "Categoria: " + cat);
}

void Menu::actionFindByDateRange() {
    std::cout << "\n" << BOLD << CBLU << "-- Buscar por rango de fechas (YYYY-MM-DD) --\n" << RST;
 
    // Validar fecha inicio
    std::string d1;
    bool d1Valida = false;
    do {
        d1 = readString("Fecha inicio (YYYY-MM-DD)");
        if (d1.size() == 10 && d1[4] == '-' && d1[7] == '-') {
            try {
                int m = std::stoi(d1.substr(5, 2));
                int d = std::stoi(d1.substr(8, 2));
                d1Valida = (m >= 1 && m <= 12 && d >= 1 && d <= 31);
            } catch (...) {}
        }
        if (!d1Valida)
            std::cout << CRED << "  Formato invalido. Use YYYY-MM-DD (ej: 2026-01-15).\n" << RST;
    } while (!d1Valida);
 
    // Validar fecha fin
    std::string d2;
    bool d2Valida = false;
    do {
        d2 = readString("Fecha fin   (YYYY-MM-DD)");
        if (d2.size() == 10 && d2[4] == '-' && d2[7] == '-') {
            try {
                int m = std::stoi(d2.substr(5, 2));
                int d = std::stoi(d2.substr(8, 2));
                d2Valida = (m >= 1 && m <= 12 && d >= 1 && d <= 31);
            } catch (...) {}
        }
        if (!d2Valida)
            std::cout << CRED << "  Formato invalido. Use YYYY-MM-DD (ej: 2026-12-31).\n" << RST;
    } while (!d2Valida);
 
    // Validar que inicio <= fin
    if (d1 > d2) {
        std::cout << CRED << "  Error: la fecha inicio debe ser <= fecha fin.\n" << RST;
        return;
    }
 
    Product* results[MAX_RESULTS];
    int count = 0;
    _catalog.findByDateRange(d1, d2, results, count);
 
    if (count == 0) {
        std::cout << CYEL << "  No se encontraron productos en ese rango.\n" << RST;
        return;
    }
    printTablePaged(results, count, "Caducidad [" + d1 + " / " + d2 + "]");
}

void Menu::actionListAllByName() {
    std::cout << "\n" << BOLD << CBLU << "-- Listado alfabetico --\n" << RST;

    if (_catalog.getAVL().isEmpty()) {
        std::cout << CYEL << "  El catalogo esta vacio.\n" << RST;
        return;
    }

    // Recolectar punteros en orden inorder del AVL
    // Usamos un arreglo de punteros estatico del mismo tamano que MAX_RESULTS
    static const int MAX_LIST = 2048;
    Product* buffer[MAX_LIST];
    int count = 0;

    _catalog.getAVL().inorder([&](const Product& p) {
        if (count < MAX_LIST)
            buffer[count++] = const_cast<Product*>(&p);
    });

    printTablePaged(buffer, count, "Listado alfabetico");
}

void Menu::actionRemoveProduct() {
    std::cout << "\n" << BOLD << CBLU << "-- Eliminar producto --\n" << RST;
    std::string bc = readString("Codigo de barra");
    if (_catalog.removeProduct(bc))
        std::cout << CGRN << "  Producto eliminado de todas las estructuras.\n" << RST;
    else
        std::cout << CRED << "  Producto no encontrado.\n" << RST;
}

void Menu::actionLoadCSV() {
    std::cout << "\n" << BOLD << CBLU << "-- Cargar desde CSV --\n" << RST;
    std::string path = readString("Ruta del archivo");
    if (!_catalog.loadFromCSV(path))
        std::cout << CRED << "  No se pudo abrir el archivo.\n" << RST;
}

void Menu::actionBenchmark() {
    std::cout << "\n" << BOLD << CMAG << "-- Benchmark de busqueda --\n" << RST;
    Benchmark bench(_catalog, 20, 5);
    bench.run();
    bench.reportResults();
}

void Menu::actionGenerateDot() {
    std::cout << "\n" << BOLD << CMAG << "-- Generar archivos Graphviz --\n" << RST;
    std::string label = readString("Nombre para los archivos (ej: prueba1)");
    if (label.empty()) label = "catalogo";

    std::string dir = "../resultados";
    system(("mkdir -p \"" + dir + "\"").c_str());

    std::cout << "\n";
    _catalog.generateDotFiles(label, dir);

    std::cout << CGRN << "\n  Archivos guardados en: resultados/\n" << RST;
    std::cout << "    " << CCYN << label << "_AVL.dot / .png\n" << RST;
    std::cout << "    " << CCYN << label << "_BTree.dot / .png\n" << RST;
    std::cout << "    " << CCYN << label << "_BPlus.dot / .png\n" << RST;
}

Product Menu::promptProduct() const {
    std::string name;
    do {
        name = readString("Nombre");
        if (name.empty())
            std::cout << CRED << "  El nombre no puede estar vacio.\n" << RST;
    } while (name.empty());

    std::string barcode;
    do {
        barcode = readString("Codigo de barra");
        if (barcode.empty())
            std::cout << CRED << "  El codigo de barra no puede estar vacio.\n" << RST;
    } while (barcode.empty());

    std::string category;
    do {
        category = readString("Categoria");
        if (category.empty())
            std::cout << CRED << "  La categoria no puede estar vacia.\n" << RST;
    } while (category.empty());

    std::string expiry;
    bool fechaValida = false;
    do {
        expiry = readString("Fecha caducidad (YYYY-MM-DD)");
        if (expiry.size() == 10 && expiry[4] == '-' && expiry[7] == '-') {
            try {
                int m = std::stoi(expiry.substr(5, 2));
                int d = std::stoi(expiry.substr(8, 2));
                fechaValida = (m >= 1 && m <= 12 && d >= 1 && d <= 31);
            } catch (...) {}
        }
        if (!fechaValida)
            std::cout << CRED << "  Formato invalido. Use YYYY-MM-DD (ej: 2026-06-15).\n" << RST;
    } while (!fechaValida);

    std::string brand;
    do {
        brand = readString("Marca");
        if (brand.empty())
            std::cout << CRED << "  La marca no puede estar vacia.\n" << RST;
    } while (brand.empty());

    double price = readDouble("Precio");
    int    stock = readInt("Stock");

    return Product(name, barcode, category, expiry, brand, price, stock);
}

void Menu::printProduct(const Product& p) const {
    std::cout << BOLD << CWHT << "  Nombre    " << RST << ": " << CCYN << p.name        << RST << "\n";
    std::cout << BOLD << CWHT << "  Codigo    " << RST << ": " << CWHT << p.barcode     << RST << "\n";
    std::cout << BOLD << CWHT << "  Categoria " << RST << ": " << CMAG << p.category    << RST << "\n";
    std::cout << BOLD << CWHT << "  Caducidad " << RST << ": " << CYEL << p.expiry_date << RST << "\n";
    std::cout << BOLD << CWHT << "  Marca     " << RST << ": " << CWHT << p.brand       << RST << "\n";
    std::cout << BOLD << CWHT << "  Precio    " << RST << ": " << CGRN << "$" << p.price << RST << "\n";
    std::cout << BOLD << CWHT << "  Stock     " << RST << ": " << CWHT << p.stock       << RST << "\n";
}