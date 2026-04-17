#include "httplib.h"
#include "json.hpp"
#include "Catalog.h"

using json = nlohmann::json;

int main() {
    Catalog catalog("../resultados/errors.log");

    httplib::Server svr;

    // CORS obligatorio para Angular
    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin",  "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 204;
    });

    auto setCORS = [](httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
    };

    // Endpoint de prueba — verificar que el server responde
    svr.Get("/api/ping", [&](const httplib::Request&, httplib::Response& res) {
        setCORS(res);
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    // Cargar CSV
    svr.Post("/api/load", [&](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        auto body = json::parse(req.body);
        std::string path = body["path"];
        bool ok = catalog.loadFromCSV(path);
        json r = { {"ok", ok}, {"errors", catalog.getLogger().errorCount()} };
        res.set_content(r.dump(), "application/json");
    });

    // GET productos
    svr.Get("/api/products", [&](const httplib::Request&, httplib::Response& res) {
        setCORS(res);
        json arr = json::array();
        catalog.getAVL().inorder([&](const Product& p) {
            arr.push_back({
                {"name",        p.name},
                {"barcode",     p.barcode},
                {"category",    p.category},
                {"expiry_date", p.expiry_date},
                {"brand",       p.brand},
                {"price",       p.price},
                {"stock",       p.stock}
            });
        });
        res.set_content(arr.dump(), "application/json");
    });

    std::cout << "Backend corriendo en http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);
}