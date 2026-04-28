#ifndef CSVLOADER_H
#define CSVLOADER_H

#include "Product.h"
#include "Logger.h"
#include "BranchManager.h"
#include "Graph.h"
#include <string>

class Catalog;

/*
 * CSVLoader
 * Clase estatica (sin estado de instancia) responsable de leer el
 * archivo CSV de productos, validar cada linea e insertar los
 * productos validos en el Catalog.
 *
 * Formato CSV esperado (con o sin comillas):
 *   "Nombre","CodigoBarra","Categoria","FechaCaducidad","Marca","Precio","Stock"
 *
 * Reglas de validacion:
 *   1. El archivo debe existir y ser legible.
 *   2. Lineas malformadas se saltean y se loggean en errors.log.
 *   3. CodigoBarra duplicado: se loggea y se omite.
 *   4. Precio y Stock deben ser numericos (Precio > 0, Stock >= 0).
 *   5. FechaCaducidad debe tener formato YYYY-MM-DD.
 *   6. Si la insercion en alguna estructura falla, se hace rollback y se loggea.
 */
class CSVLoader {
public:
    CSVLoader()                            = delete;
    CSVLoader(const CSVLoader&)            = delete;
    CSVLoader& operator=(const CSVLoader&) = delete;

    /*
     * load
     * Abre el archivo en path, lee linea por linea y para cada linea valida
     * llama a catalog.addProduct(). Los errores se escriben en logger.
     *
     * Precondicion: path no vacio; catalog y logger inicializados.
     * Postcondicion: productos validos insertados en catalog;
     *                errores registrados en logger;
     *                retorna true si el archivo pudo abrirse, false si no.
     * Complejidad: O(n) donde n = numero de lineas del archivo.
     */
    static bool load(const std::string& path, Catalog& catalog, Logger& logger);

    /*
     * parseLine
     * Extrae los 7 campos de una linea CSV (maneja comillas opcionales).
     * Precondicion: line no vacia.
     * Postcondicion: retorna Product con campos extraidos. Si la linea tiene menos de 7 campos, retorna Product invalido.
     * Complejidad: O(1) (longitud de linea acotada)
     */
    static Product parseLine(const std::string& line);

    /*
     * validateFields
     * Verifica que los 7 campos de una linea cumplan las reglas del dominio.
     * Precondicion: fields tiene exactamente 7 elementos.
     * Postcondicion: retorna true si precio es double > 0, stock es int >= 0 y la fecha tiene formato YYYY-MM-DD.
     * Complejidad: O(1)
     */
    static bool validateFields(const std::string fields[7]);

    /*
     * loadBranches
     * Carga sucursales desde un archivo CSV con 6 campos:
     *   "ID","Nombre","Ubicacion","TiempoIngreso","TiempoPreparacion","IntervaloDespacho"
     * Precondicion: path no vacio; bm y logger inicializados.
     * Postcondicion: sucursales validas insertadas en bm; errores registrados en logger.
     * Complejidad: O(n) donde n = numero de lineas del archivo.
     */
    static bool loadBranches(const std::string& path, BranchManager& bm, Graph& graph, Logger& logger);

    /*
     * loadConnections
     * Carga conexiones entre sucursales desde un archivo CSV con 4 campos:
     *   "OrigenID","DestinoID","Tiempo","Costo"
     * Precondicion: path no vacio; bm, graph y logger inicializados.
     * Postcondicion: conexiones validas insertadas en graph; errores registrados en logger.
     * Complejidad: O(n) donde n = numero de lineas del archivo.
     */
    static bool loadConnections(const std::string& path, BranchManager& bm, Graph& graph, Logger& logger);

    /*
     * loadProducts
     * Carga productos con asignacion a sucursales desde un archivo CSV con 8 campos:
     *   "SucursalID","Nombre","CodigoBarra","Categoria","FechaCaducidad","Marca","Precio","Stock"
     * Precondicion: path no vacio; bm y logger inicializados.
     * Postcondicion: productos validos insertados en sucursales; errores registrados en logger.
     * Complejidad: O(n) donde n = numero de lineas del archivo.
     */
    static bool loadProducts(const std::string& path, BranchManager& bm, Logger& logger);

private:
    // Elimina comillas y espacios al inicio y final de un campo
    static std::string trimField(const std::string& field);

    // Divide una linea CSV por comas respetando comillas dobles
    // Retorna el numero de campos encontrados (maximo 7)
    static int splitLine(const std::string& line, std::string fields[7]);

    // Divide una linea CSV con numero variable de campos (maximo maxFields)
    // Retorna el numero de campos encontrados
    static int splitLineN(const std::string& line, std::string* fields, int maxFields);

    // Valida que date tenga el formato YYYY-MM-DD con valores de mes y dia validos
    static bool isValidDate(const std::string& date);

    // Valida que str represente un numero double
    static bool isDouble(const std::string& str);

    // Valida que str represente un numero entero
    static bool isInt(const std::string& str);
};

#endif