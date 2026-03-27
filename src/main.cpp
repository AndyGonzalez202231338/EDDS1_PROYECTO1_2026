#include "Catalog.h"
#include "Menu.h"
#include <iostream>
#include <string>

#define BG_ROJO     "\033[41m"
#define ROJO_BRILLANTE  "\033[91m"
#define BG_AZUL     "\033[44m"
#define AZUL_BRILLANTE  "\033[94m"
#define RESET   "\033[0m"

int main(int argc, char* argv[]) {
    // Crear directorio de salida para los .dot
    system("mkdir -p output");

    // Inicializar el catalogo con su logger
    Catalog catalog("errors.log");

    // Carga automatica si se pasa un CSV como argumento
    if (argc == 2) {
        std::string csvPath = argv[1];
        std::cout <<AZUL_BRILLANTE<< "Cargando catalogo desde: " << csvPath << RESET << "\n";
        if (!catalog.loadFromCSV(csvPath)) {
            std::cerr <<BG_ROJO<<"!"<< ROJO_BRILLANTE << "No se pudo abrir: " << csvPath << RESET << "\n";
            std::cerr << AZUL_BRILLANTE << "Iniciando con catalogo vacio.\n" << RESET;
        }
    }

    Menu menu(catalog);
    menu.run();

    return 0;
}