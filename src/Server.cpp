#include "httplib.h"
#include "json.hpp"
#include "Catalog.h"
#include "Benchmark.h"
#include <fstream>
#include <cstdlib>

using json = nlohmann::json;

static std::string urlDecode(const std::string& s) {
    std::string r;
    r.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            int v = std::stoi(s.substr(i + 1, 2), nullptr, 16);
            r += static_cast<char>(v);
            i += 2;
        } else if (s[i] == '+') {
            r += ' ';
        } else {
            r += s[i];
        }
    }
    return r;
}

static json productToJson(const Product& p) {
    return {
        {"name",        p.name},
        {"barcode",     p.barcode},
        {"category",    p.category},
        {"expiry_date", p.expiry_date},
        {"brand",       p.brand},
        {"price",       p.price},
        {"stock",       p.stock}
    };
}

int main() {
    system("mkdir -p ../resultados");
    Catalog catalog("../resultados/errors.log");

    httplib::Server svr;

    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin",  "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 204;
    });

    auto setCORS = [](httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
    };

    // PING 
    svr.Get("/api/ping", [&](const httplib::Request&, httplib::Response& res) {
        setCORS(res);
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    // LISTAR TODOS (AVL inorder)
    svr.Get("/api/products", [&](const httplib::Request&, httplib::Response& res) {
        setCORS(res);
        json arr = json::array();
        catalog.getAVL().inorder([&](const Product& p) {
            arr.push_back(productToJson(p));
        });
        res.set_content(arr.dump(), "application/json");
    });

    // BUSCAR POR NOMBRE (AVL)
    svr.Get(R"(/api/products/name/(.+))", [&](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        std::string name = urlDecode(req.matches[1]);
        Product* p = catalog.findByName(name);
        if (!p) {
            res.status = 404;
            res.set_content("{\"error\":\"No encontrado\"}", "application/json");
            return;
        }
        res.set_content(productToJson(*p).dump(), "application/json");
    });

    // BUSCAR POR CODIGO DE BARRAS (LinkedList)
    svr.Get(R"(/api/products/barcode/(.+))", [&](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        std::string barcode = urlDecode(req.matches[1]);
        Product* p = catalog.findByBarcode(barcode);
        if (!p) {
            res.status = 404;
            res.set_content("{\"error\":\"No encontrado\"}", "application/json");
            return;
        }
        res.set_content(productToJson(*p).dump(), "application/json");
    });

    // BUSCAR POR CATEGORIA (B+ Tree)
    svr.Get(R"(/api/products/category/(.+))", [&](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        std::string cat = urlDecode(req.matches[1]);
        Product* results[MAX_RESULTS];
        int count = 0;
        catalog.findByCategory(cat, results, count);
        json arr = json::array();
        for (int i = 0; i < count; ++i) arr.push_back(productToJson(*results[i]));
        res.set_content(arr.dump(), "application/json");
    });

    // BUSCAR POR RANGO DE FECHAS (B-Tree)
    svr.Get("/api/products/range", [&](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        std::string d1 = req.get_param_value("d1");
        std::string d2 = req.get_param_value("d2");
        Product* results[MAX_RESULTS];
        int count = 0;
        catalog.findByDateRange(d1, d2, results, count);
        json arr = json::array();
        for (int i = 0; i < count; ++i) arr.push_back(productToJson(*results[i]));
        res.set_content(arr.dump(), "application/json");
    });

    // AGREGAR PRODUCTO
    svr.Post("/api/products", [&](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        json body;
        try { body = json::parse(req.body); }
        catch (...) {
            res.status = 400;
            res.set_content("{\"error\":\"JSON invalido\"}", "application/json");
            return;
        }
        Product p(
            body.value("name",        ""),
            body.value("barcode",     ""),
            body.value("category",    ""),
            body.value("expiry_date", ""),
            body.value("brand",       ""),
            body.value("price",       0.0),
            body.value("stock",       0)
        );
        if (!p.isValid()) {
            res.status = 400;
            res.set_content("{\"error\":\"Datos invalidos\"}", "application/json");
            return;
        }
        if (catalog.addProduct(p)) {
            res.set_content("{\"ok\":true}", "application/json");
        } else {
            res.status = 409;
            res.set_content("{\"error\":\"Codigo de barras duplicado o error al insertar\"}", "application/json");
        }
    });

    // ELIMINAR PRODUCTO
    svr.Delete(R"(/api/products/(.+))", [&](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        std::string barcode = urlDecode(req.matches[1]);
        if (catalog.removeProduct(barcode)) {
            res.set_content("{\"ok\":true}", "application/json");
        } else {
            res.status = 404;
            res.set_content("{\"error\":\"Producto no encontrado\"}", "application/json");
        }
    });

    // CARGAR CSV POR RUTA
    svr.Post("/api/load", [&](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        json body;
        try { body = json::parse(req.body); }
        catch (...) { res.status = 400; return; }
        std::string path = body.value("path", "");
        int before = 0;
        catalog.getAVL().inorder([&](const Product&) { before++; });
        bool ok = catalog.loadFromCSV(path);
        int after = 0;
        catalog.getAVL().inorder([&](const Product&) { after++; });
        json r = { {"ok", ok}, {"loaded", after - before}, {"errors", catalog.getLogger().errorCount()} };
        res.set_content(r.dump(), "application/json");
    });

    // CARGAR CSV POR CONTENIDO (upload desde navegador)
    svr.Post("/api/upload", [&](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        json body;
        try { body = json::parse(req.body); }
        catch (...) {
            res.status = 400;
            res.set_content("{\"error\":\"JSON invalido\"}", "application/json");
            return;
        }
        std::string content = body.value("content", "");
        if (content.empty()) {
            res.status = 400;
            res.set_content("{\"error\":\"Contenido vacio\"}", "application/json");
            return;
        }
        std::string tmpPath = "../resultados/temp_upload.csv";
        std::ofstream tmp(tmpPath);
        if (!tmp.is_open()) {
            res.status = 500;
            res.set_content("{\"error\":\"No se pudo crear archivo temporal\"}", "application/json");
            return;
        }
        tmp << content;
        tmp.close();
        int before = 0;
        catalog.getAVL().inorder([&](const Product&) { before++; });
        bool ok = catalog.loadFromCSV(tmpPath);
        int after = 0;
        catalog.getAVL().inorder([&](const Product&) { after++; });
        json r = { {"ok", ok}, {"loaded", after - before}, {"errors", catalog.getLogger().errorCount()} };
        res.set_content(r.dump(), "application/json");
    });

    // GENERAR ARCHIVOS DOT (devuelve lista de archivos generados)
    svr.Post("/api/dot", [&](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        json body;
        try { body = json::parse(req.body); } catch (...) { res.status = 400; return; }
        std::string label = body.value("label", "snapshot");
        catalog.generateDotFiles(label, "../resultados");

        json files = json::array();
        std::vector<std::string> names = {
            label + "_AVL.dot",   label + "_AVL.png",
            label + "_BTree.dot", label + "_BTree.png",
            label + "_BPlus.dot", label + "_BPlus.png"
        };
        for (const auto& name : names) {
            std::ifstream f("../resultados/" + name);
            if (f.good())
                files.push_back({ {"name", name}, {"url", "/api/files/" + name} });
        }
        json r = { {"ok", true}, {"label", label}, {"files", files} };
        res.set_content(r.dump(), "application/json");
    });

    // SERVIR ARCHIVOS DE resultados/
    svr.Get(R"(/api/files/(.+))", [&](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        std::string name = urlDecode(req.matches[1]);
        if (name.find("..") != std::string::npos || name.find('/') != std::string::npos) {
            res.status = 400; return;
        }
        std::string path = "../resultados/" + name;
        std::ifstream f(path, std::ios::binary);
        if (!f.is_open()) { res.status = 404; return; }
        std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

        std::string ct = "application/octet-stream";
        if (name.size() > 4 && name.substr(name.size() - 4) == ".png") ct = "image/png";
        else if (name.size() > 4 && name.substr(name.size() - 4) == ".dot") ct = "text/plain";

        res.set_content(content, ct.c_str());
    });

    // CONSOLA DE ERRORES
    svr.Get("/api/errors", [&](const httplib::Request&, httplib::Response& res) {
        setCORS(res);
        std::ifstream f("../resultados/errors.log");
        std::string content;
        if (f.is_open())
            content = std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        json r = { {"log", content} };
        res.set_content(r.dump(), "application/json");
    });

    // LIMPIAR LOG DE ERRORES
    svr.Delete("/api/errors", [&](const httplib::Request&, httplib::Response& res) {
        setCORS(res);
        std::ofstream f("../resultados/errors.log", std::ios::trunc);
        f.close();
        res.set_content("{\"ok\":true}", "application/json");
    });

    // BENCHMARK
    svr.Get("/api/benchmark", [&](const httplib::Request&, httplib::Response& res) {
        setCORS(res);
        Benchmark bm(catalog, 20, 5);
        bm.run();
        json arr = json::array();
        for (int i = 0; i < bm.getResultCount(); ++i) {
            const BenchmarkResult& r = bm.getResults()[i];
            arr.push_back({
                {"operation",  r.operation},
                {"structure",  r.structure},
                {"caseType",   r.caseType},
                {"avgTimeUs",  r.avgTimeUs},
                {"minTimeUs",  r.minTimeUs},
                {"maxTimeUs",  r.maxTimeUs}
            });
        }
        res.set_content(arr.dump(), "application/json");
    });

    std::cout << "Backend corriendo en http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);
}
