#pragma once
#include <optional>
#include <string>
#include <iostream>

// Declaración e implementación (inline)
inline std::optional<std::string> pickCsvFile() {
    std::string path;
    std::cout << "Ingresa la ruta del CSV: ";
    std::getline(std::cin, path);
    return path.empty() ? std::nullopt : std::optional(path);
}