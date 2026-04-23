#ifndef BRANCH_H
#define BRANCH_H

#include "Catalog.h"
#include "Queue.h"
#include "Stack.h"

/**
 * Branch representa una sucursal de la cadena de supermercados. 
 * Cada sucursal tiene su propio inventario (Catalog) y colas para gestionar el flujo de productos:
 * - colaIngreso: productos que acaban de llegar y están siendo procesados para ingreso al inventario.
 * - colaTraspaso: productos que están siendo preparados para ser enviados a otra sucursal.
 * - colaSalida: productos listos para ser despachados a su destino final.
 */

struct TransferRecord {
    std::string barcode;
    int         originId;
    int         destId;
    std::string timestamp;
};

class Branch {
public:
    int         id;
    std::string name;
    std::string location;
    int         t_ingreso;   //segundos para procesar llegada
    int         t_traspaso;  //segundos para preparar envío
    int         t_despacho;  //intervalo entre despachos

    Branch(int id, const std::string& name, const std::string& location, int t_ingreso, int t_traspaso, int t_despacho);

    // Inventario
    Catalog& getCatalog();
    bool addProduct(const Product& p);
    bool removeProduct(const std::string& barcode);
    Product* findByBarcode(const std::string& barcode);

    // Colas de despacho
    void receiveProduct(Product* p);       // colaIngreso
    void prepareTransfer(Product* p);      // colaIngreso a colaTraspaso
    void dispatchProduct();                // colaTraspaso a colaSalida
    Product* getNextDispatch();            // extrae de colaSalida

    // Rollback
    void pushRollback(const TransferRecord& r);
    TransferRecord popRollback();
    bool hasRollback() const;

private:
    Catalog                  _catalog;
    Queue<Product*>          _colaIngreso;
    Queue<Product*>          _colaTraspaso;
    Queue<Product*>          _colaSalida;
    Stack<TransferRecord>    _rollbackStack;
};

#endif // BRANCH_H