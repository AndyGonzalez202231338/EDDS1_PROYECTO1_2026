#include "FileDialog.h"
#include <cstdio>

std::optional<std::string> pickCsvFile() {
    const char* cmd =
        "zenity --file-selection "
        "--title='Seleccionar archivo CSV' "
        "--file-filter='Archivos CSV | *.csv' "
        "--file-filter='Todos los archivos | *' 2>/dev/null";

    FILE* pipe = popen(cmd, "r");
    if (!pipe) return std::nullopt;

    char buffer[4096];
    std::string path;
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        path = buffer;
        while (!path.empty() && (path.back() == '\n' || path.back() == '\r')) {
            path.pop_back();
        }
    }

    int status = pclose(pipe);
    if (status != 0 || path.empty()) return std::nullopt;

    return path;
}