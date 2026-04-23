cmake_minimum_required(VERSION 3.16)
project(SupermercadoEDD CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Fuentes comunes a ambos ejecutables (sin main ni server)
set(COMMON_SOURCES
    src/Product.cpp
    src/Logger.cpp
    src/LinkedList.cpp
    src/SortedLinkedList.cpp
    src/AVLTree.cpp
    src/BTree.cpp
    src/BPlusTree.cpp
    src/Catalog.cpp
    src/CSVLoader.cpp
    src/Benchmark.cpp
    src/HashTable.cpp
    src/Graph.cpp
    src/Branch.cpp
    src/BranchManager.cpp
)

# Ejecutable 1: menú de consola interactivo 
add_executable(supermercado
    ${COMMON_SOURCES}
    src/Menu.cpp
    src/main.cpp
)
target_include_directories(supermercado PRIVATE include)

# Ejecutable 2: servidor HTTP para Angular
add_executable(supermercado_server
    ${COMMON_SOURCES}
    src/Server.cpp
)
target_include_directories(supermercado_server PRIVATE include)

# Crear carpeta resultados/ al compilar ambos targets
foreach(TARGET supermercado supermercado_server)
    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            ${CMAKE_CURRENT_BINARY_DIR}/../resultados
        COMMENT "Creando directorio resultados/"
    )
endforeach()
