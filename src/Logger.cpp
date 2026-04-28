#include "Logger.h"
#include <ctime>
#include <iostream>


/**
 * Abre el archivo en modo append para no sobreescribir logs de sesiones anteriores.
 * Si el archivo no puede abrirse, isOpen() retornara false y los mensajes
 * se ignoraran silenciosamente para no interrumpir la carga del CSV.
 */
Logger::Logger(const std::string& path)
    : _path(path), _count(0) {
    _stream.open(_path, std::ios::app);
    if (!_stream.is_open()) {
        std::cerr << "[Logger] No se pudo abrir el archivo de log: " << _path << "\n";
    }
}

Logger::~Logger() {
    if (_stream.is_open()) {
        _stream.flush();
        _stream.close();
    }
}

/**
 * logError
 * Escribe un mensaje de error generico con timestamp.
 * Formato: [YYYY-MM-DD HH:MM:SS] ERROR: msg
 * Complejidad: O(1)
 */
void Logger::logError(const std::string& msg) {
    if (!_stream.is_open()) return;
    // Consola: rojo brillante para errores generales
    std::cerr << "\033[91m[ERROR]\033[0m " << msg << "\n";
    _stream << timestamp() << " ERROR: " << msg << "\n";
    ++_count;
}

/**
 * logDuplicate
 * Registra un codigo de barra duplicado encontrado durante la carga.
 * Formato: [YYYY-MM-DD HH:MM:SS] DUPLICATE barcode: <barcode>
 * Complejidad: O(1)
 */
void Logger::logDuplicate(const std::string& barcode) {
    if (!_stream.is_open()) return;
    // Consola: amarillo para duplicados (advertencia, no error critico)
    std::cerr << "\033[93m[DUPLICATE]\033[0m barcode: " << barcode << "\n";
    _stream << timestamp() << " DUPLICATE barcode: " << barcode << "\n";
    ++_count;
}

/**
 * logMalformed
 * Registra una linea malformada del CSV con su numero de linea.
 * Formato: [YYYY-MM-DD HH:MM:SS] MALFORMED line <N>: <contenido>
 * Complejidad: O(1)
 */
void Logger::logInfo(const std::string& msg) {
    std::cout << "\033[96m[INFO]\033[0m " << msg << "\n";
    if (!_stream.is_open()) return;
    _stream << timestamp() << " INFO: " << msg << "\n";
}

void Logger::logMalformed(int lineNumber, const std::string& line) {
    if (!_stream.is_open()) return;
    // Consola: magenta para lineas malformadas
    std::cerr << "\033[95m[MALFORMED]\033[0m linea "
              << lineNumber << ": " << line << "\n";
    _stream << timestamp() << " MALFORMED line " << lineNumber << ": " << line << "\n";
    ++_count;
}

/**
 * flush
 * Vacia el buffer del stream a disco sin cerrarlo.
 * Util para llamar periodicamente durante cargas largas.
 * Complejidad: O(1)
 */
void Logger::flush() {
    if (_stream.is_open()) {
        _stream.flush();
    }
}

/**
 * errorCount
 * Retorna la cantidad de errores registrados en la sesion actual.
 * Complejidad: O(1)
 */
int Logger::errorCount() const {
    return _count;
}

/**
 * isOpen
 * Retorna true si el stream esta abierto y listo para escribir.
 * Complejidad: O(1)
 */
bool Logger::isOpen() const {
    return _stream.is_open();
}

/**
 * timestamp
 * Genera un string con la fecha y hora actuales en formato [YYYY-MM-DD HH:MM:SS].
 * Complejidad: O(1)
 */
std::string Logger::timestamp() const {
    std::time_t now = std::time(nullptr);
    std::tm*    tm  = std::localtime(&now);

    char buf[22];
    std::strftime(buf, sizeof(buf), "[%Y-%m-%d %H:%M:%S]", tm);
    return std::string(buf);
}

/**
 * getLogContent
 * Lee y retorna el contenido del archivo de log.
 * Complejidad: O(n) donde n = tamaño del archivo
 */
std::string Logger::getLogContent() const {
    std::ifstream file(_path);
    if (!file.is_open()) {
        return "";
    }

    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }
    file.close();
    return content;
}
