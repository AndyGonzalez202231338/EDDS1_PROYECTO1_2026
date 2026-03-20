#ifndef MENU_H
#define MENU_H

#include "Catalog.h"
#include <string>

/*
 * Menu
 * Interfaz de consola que gestiona el ciclo de interaccion con el usuario.
 * Delega todas las operaciones de datos en el Catalog inyectado.
 *
 * El catalogo se pasa por referencia en el constructor
 * para que Menu no sea responsable de su ciclo de vida.
 */
class Menu {
public:
    /*
     * Constructor
     * Precondicion: catalog inicializado.
     * Postcondicion: menu listo; running = true.
     */
    explicit Menu(Catalog& catalog);

    /*
     * run
     * Bucle principal del menu. Muestra opciones, lee la seleccion del usuario
     * y devuelve la accion correspondiente hasta que el usuario elija salir.
     * Precondicion: catalog inicializado.
     * Postcondicion: bucle ejecutado hasta opcion de salida; running = false.
     * Complejidad: O(k * costo_operacion) donde k = operaciones realizadas.
     */
    void run();

private:
    Catalog& _catalog; // referencia al catalogo principal
    bool     _running; // true mientras el bucle principal este activo

    // Imprime el menu principal con todas las opciones numeradas
    void showMainMenu() const;

    // Imprime una linea separadora decorativa
    void printSeparator() const;

    // Limpia la pantalla
    void clearScreen() const;


    /*
     * readOption
     * Lee un entero de stdin. Si la entrada no es un numero valido,
     * lo pide de nuevo hasta obtener uno.
     * Precondicion: ninguna.
     * Postcondicion: retorna entero leido.
     * Complejidad: O(1) por intento valido.
     */
    int readOption() const;

    /*
     * readString
     * Lee una linea de texto de stdin con un prompt.
     * Precondicion: prompt no vacio.
     * Postcondicion: retorna string leido (puede estar vacio si el usuario
     *                presiona Enter sin escribir nada).
     * Complejidad: O(1)
     */
    std::string readString(const std::string& prompt) const;

    /*
     * readDouble
     * Lee un double de stdin con un prompt. Repite si la entrada es invalida.
     * Precondicion: prompt no vacio.
     * Postcondicion: retorna double > 0 leido de stdin.
     * Complejidad: O(1) por intento valido.
     */
    double readDouble(const std::string& prompt) const;

    /*
     * readInt
     * Lee un int de stdin con un prompt. Repite si la entrada es invalida.
     * Precondicion: prompt no vacio.
     * Postcondicion: retorna int >= 0 leido de stdin.
     * Complejidad: O(1) por intento valido.
     */
    int readInt(const std::string& prompt) const;

    /*
     * handleOption
     * Selecciona y ejecuta la accion correspondiente a opt.
     * Precondicion: opt en el rango [1, 10].
     * Postcondicion: accion ejecutada sobre _catalog; resultado en stdout.
     * Complejidad: O(1) propio, costo real depende de la operacion.
     */
    void handleOption(int opt);

    // 1. Agregar producto manualmente (lee campos uno por uno)
    void actionAddProduct();

    // 2. Buscar por nombre (AVL, O(log n))
    void actionFindByName();

    // 3. Buscar por codigo de barra (lista secuencial, O(n))
    void actionFindByBarcode();

    // 4. Buscar por categoria (BPlusTree, O(log n + k))
    void actionFindByCategory();

    // 5. Buscar por rango de fechas (BTree, O(log n + k))
    void actionFindByDateRange();

    // 6. Eliminar producto
    void actionRemoveProduct();

    // 7. Listar todos por nombre (AVL inorder, O(n))
    void actionListAllByName();

    // 8. Cargar CSV
    void actionLoadCSV();

    // 9. Ejecutar benchmark de busqueda
    void actionBenchmark();

    // 10. Generar archivos .dot de los arboles
    void actionGenerateDot();

    /*
     * promptProduct
     * Lee todos los campos de un Product campo a campo desde stdin.
     * Postcondicion: retorna Product con los campos ingresados;
     *                puede ser invalido si el usuario dejo campos vacios.
     * Complejidad: O(1)
     */
    Product promptProduct() const;

    // Imprime un Product formateado en stdout
    void printProduct(const Product& p) const;
};

#endif