#include "Logger.h"
#include <ctime>
#include <iostream>


/* Abre el archivo en modo append para no sobreescribir logs de sesiones anteriores.
 * Si el archivo no puede abrirse, isOpen() retornara false y los mensajes
 * se ignoraran silenciosamente para no interrumpir la carga del CSV.
 */
Logger::Logger(const std::string& path): _path(path), _count(0) {
    _stream.open(_path, std::ios::app);
    if (!_stream.is_open()) {
        std::cerr << "[Logger] No se pudo abrir el archivo de log: " << _path << "\n";
    }
}

// Vacia el buffer y cierra el stream antes de destruir el objeto.
Logger::~Logger() {
    if (_stream.is_open()) {
        _stream.flush();
        _stream.close();
    }
}

/* logError
 * Escribe un mensaje de error generico con timestamp.
 * Formato: [YYYY-MM-DD HH:MM:SS] ERROR: msg
 */
void Logger::logError(const std::string& msg) {
    if (!_stream.is_open()) return;
    _stream << timestamp() << " ERROR: " << msg << "\n";
    ++_count;
}

/* logDuplicate
 * Registra un codigo de barra duplicado encontrado durante la carga.
 * Formato: [YYYY-MM-DD HH:MM:SS] DUPLICATE barcode: <barcode>
 */
void Logger::logDuplicate(const std::string& barcode) {
    if (!_stream.is_open()) return;
    _stream << timestamp() << " DUPLICATE barcode: " << barcode << "\n";
    ++_count;
}

/* logMalformed
 * Registra una linea malformada del CSV con su numero de linea.
 * Formato: [YYYY-MM-DD HH:MM:SS] MALFORMED line <N>: <contenido>
 */
void Logger::logMalformed(int lineNumber, const std::string& line) {
    if (!_stream.is_open()) return;
    _stream << timestamp() << " MALFORMED line " << lineNumber << ": " << line << "\n";
    ++_count;
}

/* flush
 * Vacia el buffer del stream a disco sin cerrarlo.
 * Sirve para llamar periodicamente durante cargas largas.
 * Complejidad: O(1)
 */
void Logger::flush() {
    if (_stream.is_open()) {
        _stream.flush();
    }
}

/* errorCount
 * Retorna la cantidad de errores registrados en la sesion actual.
 */
int Logger::errorCount() const {
    return _count;
}

/* isOpen
 * Retorna true si el stream esta abierto y listo para escribir.
 */
bool Logger::isOpen() const {
    return _stream.is_open();
}

/* timestamp
 * Genera un string con la fecha y hora actuales en formato [YYYY-MM-DD HH:MM:SS].
 */
std::string Logger::timestamp() const {
    std::time_t now = std::time(nullptr);
    std::tm*    tm  = std::localtime(&now);

    char buf[22];
    std::strftime(buf, sizeof(buf), "[%Y-%m-%d %H:%M:%S]", tm);
    return std::string(buf);
}