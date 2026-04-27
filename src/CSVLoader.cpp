#include "CSVLoader.h"
#include "Catalog.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <iostream>

/**
 * Lee el archivo linea por linea, valida cada una
 * e inserta los productos validos en el catalogo.
 * Retorna true si el archivo pudo abrirse.
 * Complejidad: O(n) donde n = lineas del archivo.
 */
bool CSVLoader::load(const std::string& path, Catalog& catalog, Logger& logger) {
    std::ifstream file(path);
    if (!file.is_open()) {
        logger.logError("No se pudo abrir el archivo: " + path);
        return false;
    }
    
    std::string line;
    int lineNumber   = 0;
    int loaded       = 0;
    int skipped      = 0;

    // Saltar la linea de cabecera si existe
    if (std::getline(file, line)) {
        ++lineNumber;
        // Si la primera linea empieza con "Nombre" o comillas de cabecera
        if (line.find("Nombre") != std::string::npos ||
            line.find("nombre") != std::string::npos ||
            line.find("CodigoBarra") != std::string::npos) {
            // Es cabecera, continuar con la siguiente linea
        } else {
            // No es cabecera, procesar esta linea
            std::string fields[7];
            int n = splitLine(line, fields);
            if (n == 7 && validateFields(fields)) {
                Product p = parseLine(line);
                if (p.isValid()) {
                    if (catalog.addProduct(p)) ++loaded;
                    else ++skipped;
                } else {
                    logger.logMalformed(lineNumber, line);
                    ++skipped;
                }
            } else {
                logger.logMalformed(lineNumber, line);
                ++skipped;
            }
        }
    }

    // Procesar el resto de lineas
    while (std::getline(file, line)) {
        ++lineNumber;

        // Ignorar lineas vacias
        if (line.empty()) continue;

        std::string fields[7];
        int n = splitLine(line, fields);

        // Validar cantidad de campos
        if (n != 7) {
            logger.logMalformed(lineNumber, line);
            ++skipped;
            continue;
        }

        // Validar tipos y formato
        if (!validateFields(fields)) {
            logger.logMalformed(lineNumber, line);
            ++skipped;
            continue;
        }

        // Construir el producto
        Product p = parseLine(line);
        if (!p.isValid()) {
            logger.logMalformed(lineNumber, line);
            ++skipped;
            continue;
        }

        // Insertar en el catalogo (addProduct maneja duplicados y logging)
        if (catalog.addProduct(p)) {
            ++loaded;
        } else {
            ++skipped;
        }

        // Flush periodico del logger cada 100 lineas
        if (lineNumber % 100 == 0) logger.flush();
    }

    file.close();
    logger.flush();

    std::cout << "[CSVLoader] Carga completada: "
              << loaded  << " productos cargados, "
              << skipped << " omitidos, "
              << logger.errorCount() << " errores en log.\n";

    return true;
}

/**
 * Extrae los 7 campos de una linea CSV y construye
 * un Product. Si la linea es invalida retorna un
 * Product por defecto (isValid() == false).
 * Complejidad: O(1)
 */
Product CSVLoader::parseLine(const std::string& line) {
    std::string fields[7];
    int n = splitLine(line, fields);
    if (n != 7) return Product();

    std::string name        = trimField(fields[0]);
    std::string barcode     = trimField(fields[1]);
    std::string category    = trimField(fields[2]);
    std::string expiryDate  = trimField(fields[3]);
    std::string brand       = trimField(fields[4]);
    std::string priceStr    = trimField(fields[5]);
    std::string stockStr    = trimField(fields[6]);

    if (!isDouble(priceStr) || !isInt(stockStr)) return Product();

    double price = std::stod(priceStr);
    int    stock = std::stoi(stockStr);

    return Product(name, barcode, category, expiryDate, brand, price, stock);
}


/**
 * Verifica que los 7 campos cumplan las reglas.
 * Precondicion: fields tiene exactamente 7 elementos.
 * Complejidad: O(1)
 */
bool CSVLoader::validateFields(const std::string fields[7]) {
    // Campo 0: nombre no vacio
    if (trimField(fields[0]).empty()) return false;

    // Campo 1: barcode no vacio
    if (trimField(fields[1]).empty()) return false;
    // Campo 2: categoria no vacia
    if (trimField(fields[2]).empty()) return false;

    // Campo 3: fecha con formato YYYY-MM-DD
    if (!isValidDate(trimField(fields[3]))) return false;

    // Campo 4: marca no vacia
    if (trimField(fields[4]).empty()) return false;

    // Campo 5: precio numerico y positivo
    std::string priceStr = trimField(fields[5]);
    if (!isDouble(priceStr)) return false;
    if (std::stod(priceStr) <= 0.0) return false;

    // Campo 6: stock entero no negativo
    std::string stockStr = trimField(fields[6]);
    if (!isInt(stockStr)) return false;
    if (std::stoi(stockStr) < 0) return false;

    return true;
}

/**
 * Elimina comillas dobles, espacios y \r al inicio y final.
 * Complejidad: O(1)
 */
std::string CSVLoader::trimField(const std::string& field) {
    std::string result = field;

    // Eliminar \r (saltos de linea de Windows)
    if (!result.empty() && result.back() == '\r') {
        result.pop_back();
    }

    // Eliminar espacios y comillas al inicio
    size_t start = 0;
    while (start < result.size() &&
           (result[start] == ' ' || result[start] == '"')) {
        ++start;
    }

    // Eliminar espacios y comillas al final
    size_t end = result.size();
    while (end > start &&
           (result[end - 1] == ' ' || result[end - 1] == '"')) {
        --end;
    }

    return result.substr(start, end - start);
}

/**
 * Divide una linea CSV por comas respetando comillas dobles.
 * Retorna el numero de campos encontrados (maximo 7).
 * Maneja campos con comas dentro de comillas.
 * Complejidad: O(1) - longitud de linea acotada
 */
int CSVLoader::splitLine(const std::string& line, std::string fields[7]) {
    int    count    = 0;
    bool   inQuotes = false;
    std::string current;

    for (size_t i = 0; i < line.size() && count < 7; ++i) {
        char c = line[i];

        if (c == '"') {
            // Comilla doble escapada ("") dentro de un campo entre comillas
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                current += '"';
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            fields[count++] = current;
            current.clear();
        } else {
            current += c;
        }
    }

    // Ultimo campo
    if (count < 7) {
        fields[count++] = current;
    }

    return count;
}

/**
 * Verifica formato YYYY-MM-DD con valores de mes y dia validos.
 * Complejidad: O(1)
 */

bool CSVLoader::isValidDate(const std::string& date) {
    // Longitud exacta: YYYY-MM-DD = 10 caracteres
    if (date.size() != 10) return false;
    if (date[4] != '-' || date[7] != '-') return false;

    // Todos los demas caracteres deben ser digitos
    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) continue;
        if (!std::isdigit(static_cast<unsigned char>(date[i]))) return false;
    }

    int month = std::stoi(date.substr(5, 2));
    int day   = std::stoi(date.substr(8, 2));

    if (month < 1 || month > 12) return false;
    if (day   < 1 || day   > 31) return false;

    return true;
}

/**
 * Verifica que str represente un numero double valido.
 * Acepta negativos y decimales. Complejidad: O(1)
 */

bool CSVLoader::isDouble(const std::string& str) {
    if (str.empty()) return false;
    try {
        size_t pos;
        std::stod(str, &pos);
        // Verificar que se consumio toda la cadena
        return pos == str.size();
    } catch (...) {
        return false;
    }
}
/**
 * Verifica que str represente un entero valido.
 * Acepta negativos. Complejidad: O(1)
 */

bool CSVLoader::isInt(const std::string& str) {
    if (str.empty()) return false;
    try {
        size_t pos;
        std::stoi(str, &pos);
        return pos == str.size();
    } catch (...) {
        return false;
    }
}

/**
 * Divide una linea CSV con numero variable de campos.
 * Respeta comillas dobles y retorna numero de campos encontrados.
 * Complejidad: O(1) - longitud de linea acotada
 */
int CSVLoader::splitLineN(const std::string& line, std::string* fields, int maxFields) {
    int    count    = 0;
    bool   inQuotes = false;
    std::string current;

    for (size_t i = 0; i < line.size() && count < maxFields; ++i) {
        char c = line[i];

        if (c == '"') {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                current += '"';
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            fields[count++] = current;
            current.clear();
        } else {
            current += c;
        }
    }

    if (count < maxFields) {
        fields[count++] = current;
    }

    return count;
}

/**
 * loadBranches
 * Carga sucursales desde archivo CSV con 6 campos:
 *   "ID","Nombre","Ubicacion","TiempoIngreso","TiempoPreparacion","IntervaloDespacho"
 * Validaciones:
 *   - ID debe ser entero positivo y unico
 *   - Nombre, Ubicacion no deben estar vacios
 *   - TiempoIngreso, TiempoPreparacion, IntervaloDespacho deben ser enteros positivos
 * Complejidad: O(n) donde n = numero de lineas
 */
bool CSVLoader::loadBranches(const std::string& path, BranchManager& bm, Logger& logger) {
    std::ifstream file(path);
    if (!file.is_open()) {
        logger.logError("No se pudo abrir archivo de sucursales: " + path);
        return false;
    }

    std::string line;
    int lineNumber = 0;
    int loaded = 0;
    int skipped = 0;
    bool pastFirstLine = false;

    while (std::getline(file, line)) {
        ++lineNumber;

        if (line.empty()) continue;

        std::string fields[6];
        int n = splitLineN(line, fields, 6);
        std::string firstField = trimField(fields[0]);

        // Skip the first line only if it looks like a header (ID is not a numeric value)
        if (!pastFirstLine) {
            pastFirstLine = true;
            if (n < 1 || !isInt(firstField) || std::stoi(firstField) <= 0) {
                continue;
            }
        }

        if (n != 6) {
            logger.logMalformed(lineNumber, line);
            ++skipped;
            continue;
        }

        std::string idStr                = firstField;
        std::string nombre               = trimField(fields[1]);
        std::string ubicacion            = trimField(fields[2]);
        std::string tiempoIngresoStr     = trimField(fields[3]);
        std::string tiempoPreparacionStr = trimField(fields[4]);
        std::string intervaloDespachoStr = trimField(fields[5]);

        // Validaciones
        if (!isInt(idStr) || std::stoi(idStr) <= 0) {
            logger.logMalformed(lineNumber, "ID invalido: " + idStr);
            ++skipped;
            continue;
        }

        if (nombre.empty() || ubicacion.empty()) {
            logger.logMalformed(lineNumber, "Nombre o Ubicacion vacios");
            ++skipped;
            continue;
        }

        if (!isInt(tiempoIngresoStr) || std::stoi(tiempoIngresoStr) <= 0) {
            logger.logMalformed(lineNumber, "TiempoIngreso invalido");
            ++skipped;
            continue;
        }

        if (!isInt(tiempoPreparacionStr) || std::stoi(tiempoPreparacionStr) <= 0) {
            logger.logMalformed(lineNumber, "TiempoPreparacion invalido");
            ++skipped;
            continue;
        }

        if (!isInt(intervaloDespachoStr) || std::stoi(intervaloDespachoStr) <= 0) {
            logger.logMalformed(lineNumber, "IntervaloDespacho invalido");
            ++skipped;
            continue;
        }

        int id = std::stoi(idStr);
        int tiempoIngreso = std::stoi(tiempoIngresoStr);
        int tiempoPreparacion = std::stoi(tiempoPreparacionStr);
        int intervaloDespacho = std::stoi(intervaloDespachoStr);

        if (bm.addBranch(id, nombre, ubicacion, tiempoIngreso, tiempoPreparacion, intervaloDespacho)) {
            ++loaded;
        } else {
            logger.logError("No se pudo insertar sucursal ID=" + idStr);
            ++skipped;
        }

        if (lineNumber % 50 == 0) logger.flush();
    }

    file.close();
    logger.flush();

    std::cout << "[CSVLoader] Sucursales: " << loaded << " cargadas, " << skipped << " omitidas.\n";
    return true;
}

/**
 * loadConnections
 * Carga conexiones entre sucursales desde archivo CSV con 4 campos:
 *   "Origen","Destino","Distancia","Tiempo"
 * Validaciones:
 *   - Origen, Destino deben ser IDs validos de sucursales existentes
 *   - Distancia, Tiempo deben ser enteros positivos
 * Complejidad: O(n) donde n = numero de lineas
 */
bool CSVLoader::loadConnections(const std::string& path, BranchManager& bm, Graph& graph, Logger& logger) {
    std::ifstream file(path);
    if (!file.is_open()) {
        logger.logError("No se pudo abrir archivo de conexiones: " + path);
        return false;
    }

    std::string line;
    int lineNumber = 0;
    int loaded = 0;
    int skipped = 0;
    bool pastFirstLine = false;

    while (std::getline(file, line)) {
        ++lineNumber;

        if (line.empty()) continue;

        std::string fields[4];
        int n = splitLineN(line, fields, 4);
        std::string firstField = trimField(fields[0]);

        if (!pastFirstLine) {
            pastFirstLine = true;
            if (n < 1 || !isInt(firstField) || std::stoi(firstField) <= 0) {
                continue;
            }
        }

        if (n != 4) {
            logger.logMalformed(lineNumber, line);
            ++skipped;
            continue;
        }

        std::string origenStr  = trimField(fields[0]);
        std::string destinoStr = trimField(fields[1]);
        std::string tiempoStr  = trimField(fields[2]);
        std::string costoStr   = trimField(fields[3]);

        // Validaciones
        if (!isInt(origenStr) || std::stoi(origenStr) <= 0) {
            logger.logMalformed(lineNumber, "OrigenID invalido: " + origenStr);
            ++skipped;
            continue;
        }

        if (!isInt(destinoStr) || std::stoi(destinoStr) <= 0) {
            logger.logMalformed(lineNumber, "DestinoID invalido: " + destinoStr);
            ++skipped;
            continue;
        }

        if (!isDouble(tiempoStr) || std::stod(tiempoStr) <= 0) {
            logger.logMalformed(lineNumber, "Tiempo invalido");
            ++skipped;
            continue;
        }

        if (!isDouble(costoStr) || std::stod(costoStr) <= 0) {
            logger.logMalformed(lineNumber, "Costo invalido");
            ++skipped;
            continue;
        }

        int    origen  = std::stoi(origenStr);
        int    destino = std::stoi(destinoStr);
        double tiempo  = std::stod(tiempoStr);
        double costo   = std::stod(costoStr);

        // Verificar que ambas sucursales existan
        if (!bm.findBranch(origen)) {
            logger.logError("Sucursal origen no existe: " + origenStr);
            ++skipped;
            continue;
        }

        if (!bm.findBranch(destino)) {
            logger.logError("Sucursal destino no existe: " + destinoStr);
            ++skipped;
            continue;
        }

        graph.addEdge(origen, destino, tiempo, costo, true);
        ++loaded;

        if (lineNumber % 50 == 0) logger.flush();
    }

    file.close();
    logger.flush();

    std::cout << "[CSVLoader] Conexiones: " << loaded << " cargadas, " << skipped << " omitidas.\n";
    return true;
}

/**
 * loadProducts
 * Carga productos con asignacion a sucursales desde archivo CSV con 8 campos:
 *   "Nombre","CodigoBarra","Categoria","FechaCaducidad","Marca","Precio","Stock","SucursalID"
 * Validaciones:
 *   - Campos 1-6 deben cumplir reglas de productos normales (ver validateFields)
 *   - SucursalID debe existir en BranchManager
 * Complejidad: O(n) donde n = numero de lineas
 */
bool CSVLoader::loadProducts(const std::string& path, BranchManager& bm, Logger& logger) {
    std::ifstream file(path);
    if (!file.is_open()) {
        logger.logError("No se pudo abrir archivo de productos por sucursal: " + path);
        return false;
    }

    std::string line;
    int lineNumber = 0;
    int loaded = 0;
    int skipped = 0;
    bool pastFirstLine = false;

    while (std::getline(file, line)) {
        ++lineNumber;

        if (line.empty()) continue;

        std::string fields[8];
        int n = splitLineN(line, fields, 8);
        std::string firstField = trimField(fields[0]);

        if (!pastFirstLine) {
            pastFirstLine = true;
            if (n < 1 || !isInt(firstField) || std::stoi(firstField) <= 0) {
                continue;
            }
        }

        if (n != 8) {
            logger.logMalformed(lineNumber, line);
            ++skipped;
            continue;
        }

        // Extraer campos: SucursalID primero
        std::string branchIdStr = firstField;
        std::string name        = trimField(fields[1]);
        std::string barcode     = trimField(fields[2]);
        std::string category    = trimField(fields[3]);
        std::string expiryDate  = trimField(fields[4]);
        std::string brand       = trimField(fields[5]);
        std::string priceStr    = trimField(fields[6]);
        std::string stockStr    = trimField(fields[7]);

        // Validar SucursalID primero
        if (!isInt(branchIdStr) || std::stoi(branchIdStr) <= 0) {
            logger.logError("SucursalID invalido: " + branchIdStr);
            ++skipped;
            continue;
        }

        int branchId = std::stoi(branchIdStr);
        Branch* branch = bm.findBranch(branchId);
        if (!branch) {
            logger.logError("Sucursal no existe: " + branchIdStr);
            ++skipped;
            continue;
        }

        // Validar campos de producto
        if (name.empty() || barcode.empty() || category.empty() || brand.empty()) {
            logger.logMalformed(lineNumber, "Campos texto vacios");
            ++skipped;
            continue;
        }

        if (!isValidDate(expiryDate)) {
            logger.logMalformed(lineNumber, "FechaCaducidad invalida: " + expiryDate);
            ++skipped;
            continue;
        }

        if (!isDouble(priceStr)) {
            logger.logMalformed(lineNumber, "Precio invalido: " + priceStr);
            ++skipped;
            continue;
        }

        if (!isInt(stockStr)) {
            logger.logMalformed(lineNumber, "Stock invalido: " + stockStr);
            ++skipped;
            continue;
        }

        double price = std::stod(priceStr);
        int stock = std::stoi(stockStr);

        if (price <= 0.0) {
            logger.logMalformed(lineNumber, "Precio debe ser positivo");
            ++skipped;
            continue;
        }

        if (stock < 0) {
            logger.logMalformed(lineNumber, "Stock no puede ser negativo");
            ++skipped;
            continue;
        }

        // Construir producto y asignar a sucursal
        Product p(name, barcode, category, expiryDate, brand, price, stock);
        p.branchId = branchId;

        if (p.isValid()) {
            if (branch->insertProduct(p)) {
                ++loaded;
            } else {
                logger.logError("Codigo de barra duplicado o error de insercion: " + barcode);
                ++skipped;
            }
        } else {
            logger.logMalformed(lineNumber, line);
            ++skipped;
        }

        if (lineNumber % 100 == 0) logger.flush();
    }

    file.close();
    logger.flush();

    std::cout << "[CSVLoader] Productos: " << loaded << " cargados, " << skipped << " omitidos.\n";
    return true;
}