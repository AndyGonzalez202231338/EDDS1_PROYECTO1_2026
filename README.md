# EDDS1_PROYECTO1_2026

#   --INTALAR--
g++ --version
cmake --version
dot -V          # Graphviz, solo para generar PNG

#   --INSTALAR EN UBUNTU--
sudo apt update
sudo apt install build-essential cmake graphviz

#   --CARPETAS--
Proyecto1/
    src/                  # Implementaciones .cpp
    include/              # Headers .h
    data/
    resultados/           # .dot, .png y errors.log (se crea al ejecutar)
    build/                # Ejecutable generado por CMake
    CMakeLists.txt
    README.md

#   --COMPILACION CMAKE--
# Desde la raíz del proyecto (donde está CMakeLists.txt)
mkdir build
cd build
cmake ..
make

#   --Ejecucion--
# Desde el directorio build/

# Iniciar con catálogo vacío
./supermercado

# Cargar CSV automáticamente al iniciar
./supermercado ../data/productos.csv

# Cargar CSV de demostración (20 productos, ideal para visualizar árboles)
./supermercado ../data/productos_demo.csv

#   --MENU PRINCIPAL--
  1. Agregar producto
  2. Buscar por nombre            AVL  O(log n)
  3. Buscar por código de barra   Lista secuencial O(n)
  4. Buscar por categoría         B+ tree O(log n + k)
  5. Buscar por rango de fechas   B tree O(log n + k)
  6. Eliminar producto
  7. Listar todos (orden alfabético)
  8. Cargar CSV
  9. Ejecutar benchmark
 10. Generar archivos .dot y PNG
  0. Salir

#   --Archivos Generados--
Todos los archivos de salida se guardan en resultados/, al mismo nivel que src/

resultados/errors.log           Errores de carga del CSV (líneas malformadas, duplicados)
resultados/<nombre>_AVL.dot     Graphviz del árbol AVL
resultados/<nombre>_AVL.png     Imagen del árbol AVL
resultados/<nombre>_BTree.dot   Graphviz del árbol B
resultados/<nombre>_BTree.png   Imagen del árbol B
resultados/<nombre>_BPlus.dot   Graphviz del árbol B+
resultados/<nombre>_BPlus.png   Imagen del árbol B+

#   --Formato CSV--
"Nombre","CodigoBarra","Categoria","FechaCaducidad","Marca","Precio","Stock"
"Leche Entera","7500000000001","Lacteos","2026-06-15","Lala",18.50,100

