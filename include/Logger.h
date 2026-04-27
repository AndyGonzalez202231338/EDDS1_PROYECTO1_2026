#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>

/*
 * Logger
 * Registra errores y eventos durante la carga del CSV en errors.log.
 * No lanza excepciones; si el archivo no puede abrirse, los mensajes
 * se ignoran silenciosamente para no interrumpir la carga.
 *
 * Todas las operaciones son O(1).
 */
class Logger {
public:
    /*
     * Constructor
     * Precondicion: path es una ruta de archivo valida con permisos de escritura.
     * Postcondicion: stream abierto en modo append sobre path; count = 0.
     */
    explicit Logger(const std::string& path = "errors.log");

    /*
     * Destructor
     * Postcondicion: stream vaciado y cerrado
     */
    ~Logger();

    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;

    /*
     * logError
     * Escribe un mensaje de error generico con timestamp.
     * Precondicion: msg no vacio.
     * Postcondicion: linea "[TIMESTAMP] ERROR: msg" escrita en stream; count++.
     * Complejidad: O(1)
     */
    void logError(const std::string& msg);

    /*
     * logDuplicate
     * Registra un codigo de barra duplicado encontrado durante la carga.
     * Precondicion: barcode no vacio.
     * Postcondicion: linea "[TIMESTAMP] DUPLICATE: barcode" escrita; count++.
     * Complejidad: O(1)
     */
    void logDuplicate(const std::string& barcode);

    /*
     * logMalformed
     * Registra una linea malformada del CSV con su numero de linea.
     * Precondicion: line no vacio.
     * Postcondicion: linea "[TIMESTAMP] MALFORMED line N: line" escrita; count++.
     * Complejidad: O(1)
     */
    void logMalformed(int lineNumber, const std::string& line);

    /*
     * flush
     * Vacia el buffer del stream a disco.
     * Precondicion: stream abierto.
     * Postcondicion: datos persistidos; buffer limpio.
     * Complejidad: O(1)
     */
    void flush();

    /*
     * errorCount
     * Precondicion: ninguna.
     * Postcondicion: retorna cantidad de errores registrados en la sesion.
     * Complejidad: O(1)
     */
    int errorCount() const;

    /*
     * isOpen
     * Precondicion: ninguna.
     * Postcondicion: retorna true si el stream esta abierto correctamente.
     * Complejidad: O(1)
     */
    bool isOpen() const;

    /*
     * getLogContent
     * Lee y retorna el contenido actual del archivo de log.
     * Precondicion: ninguna.
     * Postcondicion: retorna string con contenido del archivo; string vacio si no puede leer.
     * Complejidad: O(n) donde n = tamaño del archivo
     */
    std::string getLogContent() const;

private:
    std::string  _path;   // ruta del archivo de log
    std::ofstream _stream; // flujo de escritura
    int          _count;  // errores registrados en la sesion actual

    // Retorna timestamp actual como string "[YYYY-MM-DD HH:MM:SS]"
    std::string timestamp() const;
};

#endif