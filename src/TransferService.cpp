#include "TransferService.h"
#include "Branch.h"
#include <string>

TransferService::TransferService(BranchManager& bm, Graph& g)
    : _bm(bm), _g(g) {}

void TransferService::addStep(TransferResult& r, const std::string& s) {
    if (r.stepCount < 600)
        r.steps[r.stepCount++] = s;
}

TransferResult TransferService::simulate(const std::string& barcode,
                                         int originId, int destId, bool byTime) {
    TransferResult result;

    // --- Validaciones ---
    Branch* origin = _bm.findBranch(originId);
    if (!origin) {
        result.error = "Sucursal origen no existe (ID=" + std::to_string(originId) + ")";
        return result;
    }
    Branch* dest = _bm.findBranch(destId);
    if (!dest) {
        result.error = "Sucursal destino no existe (ID=" + std::to_string(destId) + ")";
        return result;
    }
    if (originId == destId) {
        result.error = "Origen y destino son la misma sucursal";
        return result;
    }
    Product* prod = origin->searchByBarcode(barcode);
    if (!prod) {
        result.error = "Producto '" + barcode + "' no encontrado en sucursal origen";
        return result;
    }

    // --- Ruta optima ---
    PathResult pr = byTime ? _g.shortestPathByTime(originId, destId)
                           : _g.shortestPathByCost(originId, destId);
    if (pr.length == 0) {
        result.error = "No existe ruta entre sucursal " +
                       std::to_string(originId) + " y " + std::to_string(destId);
        return result;
    }

    result.length = pr.length;
    for (int i = 0; i < pr.length; ++i) result.path[i] = pr.path[i];

    // Copia de trabajo del producto para simular estados
    Product item = *prod;

    // --- Simulacion de colas ---
    // Para cada sucursal en la ruta:
    //   Si es intermedia: ingreso -> preparacion -> salida -> transito
    //   Si es destino:    ingreso -> disponible
    for (int i = 0; i < pr.length; ++i) {
        int     bId  = pr.path[i];
        Branch* b    = _bm.findBranch(bId);
        if (!b) continue;
        bool isLast   = (i == pr.length - 1);
        std::string bName = b->getNombre();

        // Cola de ingreso
        item.status = ProductStatus::EN_ESPERA;
        b->queueIngreso.enqueue(item);
        addStep(result,
            "Ingreso en " + bName +
            " (+" + std::to_string(b->getTiempoIngreso()) + " min)");
        result.totalTime += b->getTiempoIngreso();
        b->queueIngreso.dequeue();

        if (!isLast) {
            // Cola de preparacion (solo sucursales intermedias)
            item.status = ProductStatus::EN_PREPARACION;
            b->queuePreparacion.enqueue(item);
            addStep(result,
                "Preparacion en " + bName +
                " (+" + std::to_string(b->getTiempoPreparacion()) + " min)");
            result.totalTime += b->getTiempoPreparacion();
            b->queuePreparacion.dequeue();

            // Cola de salida
            item.status = ProductStatus::IN_TRANSIT;
            b->queueSalida.enqueue(item);
            int     nextId   = pr.path[i + 1];
            Branch* nextB    = _bm.findBranch(nextId);
            std::string nextName = nextB ? nextB->getNombre()
                                        : "Sucursal " + std::to_string(nextId);
            addStep(result,
                "Salida desde " + bName + " hacia " + nextName +
                " (+" + std::to_string(b->getIntervaloDespacho()) + " min)");
            result.totalTime += b->getIntervaloDespacho();
            b->queueSalida.dequeue();

            // Tiempo de transito entre nodos (siempre en minutos de tiempo)
            double tEdge = _g.edgeWeight(bId, nextId, false);
            if (tEdge < 0) tEdge = 0;
            addStep(result,
                "Transito " + bName + " -> " + nextName +
                " (+" + std::to_string(static_cast<int>(tEdge)) + " min)");
            result.totalTime += tEdge;
        } else {
            // Destino final: producto queda disponible
            item.status = ProductStatus::AVAILABLE;
            addStep(result, "Entrega en " + bName + " - producto disponible");
        }
    }

    // --- Movimiento real del producto ---
    // Re-leer el puntero (sigue valido porque no se modifico el inventario aun)
    Product moved = *origin->searchByBarcode(barcode);
    moved.branchId = destId;
    moved.status   = ProductStatus::AVAILABLE;

    if (!dest->insertProduct(moved)) {
        result.error = "No se pudo insertar en destino (barcode duplicado o error interno)";
        return result;
    }
    if (!origin->removeProduct(barcode)) {
        dest->removeProduct(barcode); // rollback
        result.error = "No se pudo eliminar de origen (rollback aplicado)";
        return result;
    }

    result.ok = true;
    return result;
}
