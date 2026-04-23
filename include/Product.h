#ifndef PRODUCT_H
    #define PRODUCT_H

    #include <string>


    enum class ProductStatus { AVAILABLE, IN_TRANSIT, DEPLETED };

    /*
    * Product
    * Entidad base del catalogo de supermercado.
    * Todos los campos son publicos para facilitar el acceso directo
    */
    struct Product {
        std::string name;         
        std::string barcode;      // codigo de barra (clave unica en el sistema)
        std::string category;     // categoria (ej: lacteos, bebidas, carnes)
        std::string expiry_date;  // fecha de caducidad en formato ISO "YYYY-MM-DD"
        std::string brand;        
        double      price;        // precio unitario, debe ser > 0
        int         stock;        // unidades disponibles, debe ser >= 0
        ProductStatus status = ProductStatus::AVAILABLE;
        int         branchId = -1;

        Product();
        
        Product(const std::string& name,
            const std::string& barcode,
            const std::string& category,
            const std::string& expiry_date,
            const std::string& brand,
            double price,
            int stock
        );

        /*
        * isValid
        * Postcondicion: retorna true si name y barcode no estan vacios, price > 0 y stock >= 0.
        */
        bool isValid() const;

        std::string toString() const;

        /*
        * operator ==
        * retorna true si ambos productos tienen el mismo barcode.
        */
        bool operator==(const Product& other) const;

        /*
        * operator <
        * Comparacion por nombre (para uso en AVL y SortedLinkedList).
        * retorna true si this.name < other.name lexicograficamente.
        */
        bool operator<(const Product& other) const;

        // Getter correcto como método de instancia
        const std::string& getBarcode() const;
    };

#endif