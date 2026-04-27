# Sistema de Inventario Distribuido por Sucursales

## Componentes Implementados

### Frontend (Angular Standalone)

#### Servicios
- **StateService** - Manejo de sucursal seleccionada
- **BranchService** - Operaciones CRUD de sucursales
- **InventoryService** - Gestión de productos por sucursal
- **TransferService** - Transferencias entre sucursales

#### Componentes
1. **BranchListComponent** (`pages/branch-list/`)
   - Listar todas las sucursales disponibles
   - Seleccionar sucursal activa
   - Indicador visual de sucursal seleccionada

2. **ProductComponent** (`pages/product/`)
   - Formulario reactivo para agregar productos
   - Búsqueda por código de barra
   - Visualización de detalles del producto
   - Eliminación de productos
   - Validaciones en tiempo real

3. **TransferComponent** (`pages/transfer/`)
   - Transferir producto entre sucursales
   - Selector de sucursal origen/destino
   - Validación de sucursales diferentes
   - Control de cantidad

#### Layout
- **InventoryLayoutComponent** - Navbar y routing
- Rutas: `/inventory/branches`, `/inventory/products`, `/inventory/transfer`

### Backend (C++)

#### Endpoints REST

| Método | Endpoint | Descripción |
|--------|----------|-------------|
| GET | `/api/branches` | Listar sucursales |
| POST | `/api/branch/{id}/product` | Agregar producto a sucursal |
| GET | `/api/branch/{id}/product/{barcode}` | Buscar producto en sucursal |
| DELETE | `/api/branch/{id}/product/{barcode}` | Eliminar producto de sucursal |
| POST | `/api/transfer` | Transferir entre sucursales |

## Instrucciones de Uso

### 1. Compilar Backend
```bash
cd build
cmake .. && cmake --build .
./supermercado
```

### 2. Iniciar Frontend
```bash
cd frontend
npm install  # si no lo ha hecho
npm run dev
```

### 3. Acceder a la Aplicación
```
http://localhost:4200/
```

## Flujo de Uso

1. **Seleccionar Sucursal**: En la pestaña "Sucursales", haz clic en una sucursal para activarla
2. **Gestionar Productos**: 
   - Ir a "Productos"
   - Agregar nuevo producto (formulario izquierdo)
   - Buscar por código de barra (formulario derecho)
   - Eliminar si es necesario
3. **Transferir**: En "Transferencias", especificar origen, destino, barcode y cantidad

## Configuración

### proxy.conf.json
```json
{
    "/api": {
        "target": "http://localhost:8080",
        "secure": false,
        "changeOrigin": true
    }
}
```

### Backend puerto
El backend escucha en `http://0.0.0.0:8080`

## Requisitos Previos

1. **Sucursales deben estar inicializadas**: El backend necesita tener sucursales cargadas
   - Verificar en `BranchManager.cpp` o agregar en `Server.cpp` main()
   - Ejemplo:
     ```cpp
     branchManager.addBranch(1, "Sucursal Centro", "Calle Principal 123", 5, 10, 15);
     branchManager.addBranch(2, "Sucursal Norte", "Avenida Norte 456", 5, 10, 15);
     ```

2. **Base de datos**: El catálogo general (global) se puede cargar desde CSV
   - Usar `/api/load` endpoint

## Tecnologías

- **Frontend**: Angular 18+, Bootstrap 5, RxJS, Reactive Forms
- **Backend**: C++17, cpp-httplib, nlohmann/json
- **Estado**: BehaviorSubject (RxJS)
- **Patrón**: Servicios compartidos, Componentes standalone

## Estrutura de Datos (Backend)

Cada sucursal mantiene:
- LinkedList (búsqueda secuencial)
- SortedLinkedList (ordenada por nombre)
- AVLTree (búsqueda binaria)
- BTree (por fecha de caducidad)
- BPlusTree (por categoría)
- HashTable (por código de barra)

## API Response Examples

### GET /api/branches
```json
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
```

### POST /api/branch/1/product
```json
{
  "name": "Arroz",
  "barcode": "123456789",
  "category": "Alimentos",
  "expiry_date": "2026-12-31",
  "brand": "Marca A",
  "price": 25.50,
  "stock": 100
}
```

### GET /api/branch/1/product/123456789
```json
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
```

### POST /api/transfer
```json
{
  "barcode": "123456789",
  "originId": 1,
  "destId": 2,
  "quantity": 10
}
```

## Troubleshooting

### Error: Cannot find module
- Verificar que todos los servicios estén en `src/app/services/`
- Verificar rutas de importación

### Error: Cannot connect to backend
- Verificar que `./supermercado` está corriendo en puerto 8080
- Verificar proxy.conf.json
- Comprobar con `curl http://localhost:8080/api/ping`

### Sucursales no aparecen
- Verificar que `BranchManager` tiene sucursales initialized
- Ver logs del backend

## Archivos Creados

```
frontend/src/
├── app/
│   ├── app.ts                      # Componente raíz
│   ├── app.config.ts               # Configuración
│   ├── app.routes.ts               # Rutas
│   ├── services/
│   │   ├── state.service.ts
│   │   ├── branch.service.ts
│   │   ├── inventory.service.ts
│   │   └── transfer.service.ts
│   └── pages/
│       ├── branch-list/
│       ├── product/
│       ├── transfer/
│       └── inventory-layout/
├── main.ts
└── index.html
```

---

**Creado**: 2026-04-26  
**Versión**: 1.0  
**Estado**: Funcional y listo para pruebas
