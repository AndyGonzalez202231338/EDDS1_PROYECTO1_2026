# EDDS1_FASE2_PROYECTO1_2026

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
## Tecnologías
- **Frontend**: Angular 18+, Bootstrap 5, RxJS, Reactive Forms
- **Backend**: C++17, cpp-httplib, nlohmann/json
- **Estado**: BehaviorSubject (RxJS)
- **Patrón**: Servicios compartidos, Componentes standalone

###  --Ejecucion--
## Iniciar Backend
# Desde el directorio build/
cd build
cmake .. && cmake --build .
./supermercado_server

## Iniciar Frontend
cd frontend
npm install  # si no lo ha hecho
npm run dev

### Acceder a la Aplicación
http://localhost:4200/

### Backend puerto
El backend escucha en `http://0.0.0.0:8080`

## Flujo de Uso
1. **Seleccionar Sucursal**: En la pestaña "Sucursales", haz clic en una sucursal para activarla
2. **Gestionar Productos**: 
   - Ir a "Productos"
   - Agregar nuevo producto (formulario izquierdo)
   - Buscar por código de barra (formulario derecho)
   - Eliminar si es necesario
3. **Transferir**: En "Transferencias", especificar origen, destino, barcode y cantidad

## Estrutura de Datos (Backend)
Cada sucursal mantiene:
- LinkedList (búsqueda secuencial)
- AVLTree (búsqueda binaria)
- BTree (por fecha de caducidad)
- BPlusTree (por categoría)
- HashTable (por código de barra)

## API Response Examples

### GET /api/branches
[
  {
    "id": 1,
    "nombre": "Sucursal Centro",
    "ubicacion": "Calle Principal 123",
    "tiempoIngreso": 5,
    "tiempoPreparacion": 10,
    "intervaloDespacho": 15
  }
]


### POST /api/branch/1/product
{
  "name": "Arroz",
  "barcode": "123456789",
  "category": "Alimentos",
  "expiry_date": "2026-12-31",
  "brand": "Marca A",
  "price": 25.50,
  "stock": 100
}


### GET /api/branch/1/product/123456789
{
  "name": "Arroz",
  "barcode": "123456789",
  "category": "Alimentos",
  "expiry_date": "2026-12-31",
  "brand": "Marca A",
  "price": 25.50,
  "stock": 100,
  "branchId": 1
}

### POST /api/transfer
{
  "barcode": "123456789",
  "originId": 1,
  "destId": 2,
  "quantity": 10
}

| GET | `/api/branches` | Listar sucursales |
| POST | `/api/branch/{id}/product` | Agregar producto a sucursal |
| GET | `/api/branch/{id}/product/{barcode}` | Buscar producto en sucursal |
| DELETE | `/api/branch/{id}/product/{barcode}` | Eliminar producto de sucursal |
| POST | `/api/transfer` | Transferir entre sucursales |