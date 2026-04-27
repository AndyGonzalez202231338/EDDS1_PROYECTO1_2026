#include "Catalog.h"
#include "Menu.h"
#include "utils/FileDialog.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    system("mkdir -p ../resultados");
    Catalog catalog("../resultados/errors.log");

    if (argc == 2) {
        std::string csvPath = argv[1];
        std::cout << "Cargando catalogo desde: " << csvPath << "\n";
        if (!catalog.loadFromCSV(csvPath)) {
            std::cerr << "No se pudo abrir: " << csvPath << "\n";
            std::cerr << "Iniciando con catalogo vacio.\n";
        }
    } else {
        auto selected = pickCsvFile();
        if (selected) {
            std::cout << "Cargando catalogo desde: " << *selected << "\n";
            if (!catalog.loadFromCSV(*selected)) {
                std::cerr << "No se pudo abrir: " << *selected << "\n";
                std::cerr << "Iniciando con catalogo vacio.\n";
            }
        } else {
            std::cout << "No se selecciono CSV. Iniciando con catalogo vacio.\n";
        }
    }

    Menu menu(catalog);
    menu.run();
    return 0;
}