cmake_minimum_required(VERSION 3.16)
project(SupermercadoEDD CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

set(SOURCES
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
    src/Menu.cpp
    src/main.cpp
)

add_executable(supermercado ${SOURCES})
target_include_directories(supermercado PRIVATE include)

# Crear carpeta output/ en el directorio de build al compilar
add_custom_command(TARGET supermercado POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_CURRENT_BINARY_DIR}/output
    COMMENT "Creando directorio output/"
)