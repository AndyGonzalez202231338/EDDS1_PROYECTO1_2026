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