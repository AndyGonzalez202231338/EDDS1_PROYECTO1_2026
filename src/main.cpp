#include "Catalog.h"
#include "Menu.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    // Crear directorio de resultados al mismo nivel que src/ e include/
    system("mkdir -p ../resultados");

    // Inicializar el catalogo - errors.log se guarda en resultados/
    Catalog catalog("../resultados/errors.log");

    // Carga automatica si se pasa un CSV como argumento
    // Uso: ./supermercado productos.csv
    if (argc == 2) {
        std::string csvPath = argv[1];
        std::cout << "Cargando catalogo desde: " << csvPath << "\n";
        if (!catalog.loadFromCSV(csvPath)) {
            std::cerr << "No se pudo abrir: " << csvPath << "\n";
            std::cerr << "Iniciando con catalogo vacio.\n";
        }
    }

    // Iniciar el menu interactivo
    Menu menu(catalog);
    menu.run();

    return 0;
}