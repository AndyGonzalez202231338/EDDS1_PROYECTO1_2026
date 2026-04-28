#include "httplib.h"
#include "json.hpp"
#include "Catalog.h"
#include "Benchmark.h"
#include "BranchManager.h"
#include "InventoryService.h"
#include "Graph.h"
#include "CSVLoader.h"
#include "TransferService.h"
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <vector>

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

static void setCORS(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
}

int main() {
    system("mkdir -p ../resultados");
    Catalog catalog("../resultados/errors.log");

    BranchManager branchManager;
    InventoryService inventory(branchManager);
    Graph graph;
    TransferService transferService(branchManager, graph);

    httplib::Server svr;

    // Endpoints de sucursales
    svr.Get("/api/branches", [&](const httplib::Request&, httplib::Response& res) {
        setCORS(res);
        json arr = json::array();
        branchManager.forEach([&](const Branch& b) {
            arr.push_back({
                {"id", b.getId()},
                {"nombre", b.getNombre()},
                {"ubicacion", b.getUbicacion()},
                {"tiempoIngreso", b.getTiempoIngreso()},
                {"tiempoPreparacion", b.getTiempoPreparacion()},
                {"intervaloDespacho", b.getIntervaloDespacho()}
            });
        });
        res.set_content(arr.dump(), "application/json");
    });

    svr.Post("/api/branches", [&](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        json body = json::parse(req.body, nullptr, false);
        if (body.is_discarded()) {
            res.status = 400;
            res.set_content(R"({"error":"JSON invalido"})", "application/json");
            return;
        }
        int id                 = body.value("id", -1);
        std::string nombre     = body.value("nombre", "");
        std::string ubicacion  = body.value("ubicacion", "");
        int tiempoIngreso      = body.value("tiempoIngreso", 0);
        int tiempoPreparacion  = body.value("tiempoPreparacion", 0);
        int intervaloDespacho  = body.value("intervaloDespacho", 0);

        if (id <= 0 || nombre.empty() || ubicacion.empty()) {
            res.status = 400;
            res.set_content(R"({"error":"id, nombre y ubicacion son requeridos"})", "application/json");
            return;
        }
        if (!branchManager.addBranch(id, nombre, ubicacion, tiempoIngreso, tiempoPreparacion, intervaloDespacho)) {
            res.status = 409;
            res.set_content(R"({"error":"Ya existe una sucursal con ese ID"})", "application/json");
            return;
        }
        res.status = 201;
        res.set_content(R"({"ok":true})", "application/json");
    });

    svr.Delete(R"(/api/branches/(\d+))", [&](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        int id = std::stoi(req.matches[1].str());
        if (!branchManager.removeBranch(id)) {
            res.status = 404;
            res.set_content(R"({"error":"Sucursal no encontrada"})", "application/json");
            return;
        }
        res.set_content(R"({"ok":true})", "application/json");
    });

    svr.Post("/api/transfer", [&](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded()) {
            res.status = 400;
            res.set_content(R"({"error":"JSON invalido"})", "application/json");
            return;
        }

        std::string barcode  = body.value("barcode",  "");
        int         originId = body.value("originId", -1);
        int         destId   = body.value("destId",   -1);
        // Accept "type" (PDF spec) or legacy "criteria"
        std::string typeVal  = body.value("type", body.value("criteria", "time"));
        bool        byTime   = (typeVal != "cost");

        if (barcode.empty() || originId <= 0 || destId <= 0) {
            res.status = 400;
            res.set_content(R"({"error":"barcode, originId y destId son requeridos"})",
                            "application/json");
            return;
        }

        TransferResult tr = transferService.simulate(barcode, originId, destId, byTime);

        if (!tr.ok) {
            res.status = 400;
            res.set_content(json{{"ok", false}, {"error", tr.error}}.dump(),
                            "application/json");
            return;
        }

        json path = json::array();
        for (int i = 0; i < tr.length; ++i) path.push_back(tr.path[i]);

        json steps = json::array();
        for (int i = 0; i < tr.stepCount; ++i) steps.push_back(tr.steps[i]);

        json r = {
            {"ok",        true},
            {"path",      path},
            {"totalTime", tr.totalTime},
            {"steps",     steps}
        };
        res.set_content(r.dump(), "application/json");
    });

    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin",  "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 204;
    });

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

    // POST /branch/{id}/product
    svr.Post(R"(/branch/(\d+)/product)", [&](const httplib::Request& req, httplib::Response& res) {
        int branchId = std::stoi(req.matches[1].str());

        json body = json::parse(req.body, nullptr, false);
        if (body.is_discarded()) {
            res.status = 400;
            res.set_content(R"({"error":"JSON invalido"})", "application/json");
            return;
        }

        Product p(
            body.value("name", ""),
            body.value("barcode", ""),
            body.value("category", ""),
            body.value("expiry_date", ""),
            body.value("brand", ""),
            body.value("price", 0.0),
            body.value("stock", 0)
        );

        std::string error;
        if (!inventory.insertProduct(branchId, p, error)) {
            res.status = (error == "Branch no existe") ? 404 : 400;
            res.set_content(json({{"error", error}}).dump(), "application/json");
            return;
        }

        res.status = 201;
        res.set_content(R"({"ok":true})", "application/json");
    });

    // GET /branch/{id}/product/{barcode}
    svr.Get(R"(/branch/(\d+)/product/([^/]+))", [&](const httplib::Request& req, httplib::Response& res) {
        int branchId = std::stoi(req.matches[1].str());
        std::string barcode = req.matches[2].str();

        std::string error;
        Product* p = inventory.searchByBarcode(branchId, barcode, error);
        if (!error.empty()) {
            res.status = 404;
            res.set_content(json({{"error", error}}).dump(), "application/json");
            return;
        }
        if (p == nullptr) {
            res.status = 404;
            res.set_content(R"({"error":"Producto no encontrado"})", "application/json");
            return;
        }

        json out = {
            {"name", p->name},
            {"barcode", p->barcode},
            {"category", p->category},
            {"expiry_date", p->expiry_date},
            {"brand", p->brand},
            {"price", p->price},
            {"stock", p->stock},
            {"branchId", p->branchId}
        };

        res.status = 200;
        res.set_content(out.dump(), "application/json");
    });

    // DELETE /branch/{id}/product/{barcode}
    svr.Delete(R"(/branch/(\d+)/product/([^/]+))", [&](const httplib::Request& req, httplib::Response& res) {
        int branchId = std::stoi(req.matches[1].str());
        std::string barcode = req.matches[2].str();

        std::string error;
        if (!inventory.removeProduct(branchId, barcode, error)) {
            res.status = (error == "Branch no existe") ? 404 : 404;
            res.set_content(json({{"error", error}}).dump(), "application/json");
            return;
        }

        res.status = 200;
        res.set_content(R"({"ok":true})", "application/json");
    });

    // ========== CSV ENDPOINTS ==========
    // POST /api/csv/branches-content - Cargar sucursales desde contenido
    svr.Post("/api/csv/branches-content", [&](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        json body;
        try { body = json::parse(req.body); } catch (...) { res.status = 400; return; }
        std::string content = body.value("content", "");
        if (content.empty()) {
            res.status = 400;
            res.set_content(R"({"error":"Contenido vacio"})", "application/json");
            return;
        }
        std::string tmpPath = "../resultados/temp_branches.csv";
        std::ofstream tmp(tmpPath);
        tmp << content;
        tmp.close();

        bool ok = CSVLoader::loadBranches(tmpPath, branchManager, graph, catalog.getLogger());
        json r = { {"ok", ok}, {"errors", catalog.getLogger().errorCount()} };
        res.set_content(r.dump(), "application/json");
    });

    // POST /api/csv/connections-content - Cargar conexiones desde contenido
    svr.Post("/api/csv/connections-content", [&](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        json body;
        try { body = json::parse(req.body); } catch (...) { res.status = 400; return; }
        std::string content = body.value("content", "");
        if (content.empty()) {
            res.status = 400;
            res.set_content(R"({"error":"Contenido vacio"})", "application/json");
            return;
        }
        std::string tmpPath = "../resultados/temp_connections.csv";
        std::ofstream tmp(tmpPath);
        tmp << content;
        tmp.close();

        bool ok = CSVLoader::loadConnections(tmpPath, branchManager, graph, catalog.getLogger());
        json r = { {"ok", ok}, {"errors", catalog.getLogger().errorCount()} };
        res.set_content(r.dump(), "application/json");
    });

    // POST /api/csv/products-content - Cargar productos desde contenido
    svr.Post("/api/csv/products-content", [&](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        json body;
        try { body = json::parse(req.body); } catch (...) { res.status = 400; return; }
        std::string content = body.value("content", "");
        if (content.empty()) {
            res.status = 400;
            res.set_content(R"({"error":"Contenido vacio"})", "application/json");
            return;
        }
        std::string tmpPath = "../resultados/temp_products.csv";
        std::ofstream tmp(tmpPath);
        tmp << content;
        tmp.close();

        bool ok = CSVLoader::loadProducts(tmpPath, branchManager, catalog.getLogger());
        json r = { {"ok", ok}, {"errors", catalog.getLogger().errorCount()} };
        res.set_content(r.dump(), "application/json");
    });

    // GET /api/csv/errors - Obtener errores de CSV
    svr.Get("/api/csv/errors", [&](const httplib::Request&, httplib::Response& res) {
        setCORS(res);
        json r = { {"errors", catalog.getLogger().errorCount()} };
        res.set_content(r.dump(), "application/json");
    });

    // ========== GRAPH ENDPOINTS ==========
    // POST /api/graph/edge - Agregar arista
    svr.Post("/api/graph/edge", [&](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        json body;
        try { body = json::parse(req.body); }
        catch (...) {
            res.status = 400;
            res.set_content(R"({"error":"JSON invalido"})", "application/json");
            return;
        }

        int originId = body.value("originId", -1);
        int destId = body.value("destId", -1);
        double tiempo = body.value("tiempo", 1.0);
        double costo = body.value("costo", 1.0);
        bool bidirectional = body.value("bidirectional", true);

        if (originId <= 0 || destId <= 0) {
            res.status = 400;
            res.set_content(R"({"error":"IDs deben ser mayores a 0"})", "application/json");
            return;
        }

        // Agregar nodos de sucursales si existen
        if (branchManager.findBranch(originId)) graph.addBranch(originId);
        if (branchManager.findBranch(destId)) graph.addBranch(destId);

        bool added = graph.addEdge(originId, destId, tiempo, costo, bidirectional);
        if (!added) {
            res.status = 409;
            res.set_content(R"({"error":"La arista ya existe o los nodos no existen"})", "application/json");
            return;
        }

        res.set_content(R"({"ok":true})", "application/json");
    });

    // GET /api/graph/path?from=X&to=Y&criteria=time|cost
    svr.Get("/api/graph/path", [&](const httplib::Request& req, httplib::Response& res) {
        setCORS(res);
        int from = std::stoi(req.get_param_value("from"), nullptr, 0);
        int to = std::stoi(req.get_param_value("to"), nullptr, 0);
        std::string criteria = req.get_param_value("criteria");

        if (from <= 0 || to <= 0) {
            res.status = 400;
            res.set_content(R"({"error":"IDs invalidos"})", "application/json");
            return;
        }

        bool useCost = (criteria == "cost");
        PathResult result = useCost ? graph.shortestPathByCost(from, to) : graph.shortestPathByTime(from, to);

        if (result.length == 0) {
            res.status = 404;
            res.set_content(R"({"error":"No hay ruta disponible"})", "application/json");
            return;
        }

        json path = json::array();
        for (int i = 0; i < result.length; ++i) {
            path.push_back(result.path[i]);
        }

        json r = {
            {"path", path},
            {"distance", result.total}
        };
        res.set_content(r.dump(), "application/json");
    });

    // GET /api/graph/dot - Obtener DOT y SVG del grafo
    svr.Get("/api/graph/dot", [&](const httplib::Request&, httplib::Response& res) {
        setCORS(res);

        if (graph.nodeCount() == 0) {
            json r = {{"dot", ""}, {"svg", ""}};
            res.set_content(r.dump(), "application/json");
            return;
        }

        std::string dotPath = "../resultados/temp_graph.dot";
        std::string svgPath = "../resultados/temp_graph.svg";
        std::ofstream out(dotPath);

        if (!out.is_open()) {
            res.status = 500;
            res.set_content(R"({"error":"No se pudo crear archivo temporal"})", "application/json");
            return;
        }

        graph.toDot(out, [&](int branchId) -> std::string {
            const Branch* b = branchManager.findBranch(branchId);
            return b ? b->getNombre() : "";
        });
        out.close();

        // Generar SVG con graphviz
        std::string cmd = "/usr/bin/dot -Tsvg \"" + dotPath + "\" -o \"" + svgPath + "\"";
        int ret = system(cmd.c_str());

        std::string svgContent;
        if (ret == 0) {
            std::ifstream svgFile(svgPath);
            svgContent = std::string((std::istreambuf_iterator<char>(svgFile)),
                                     std::istreambuf_iterator<char>());
        }

        std::ifstream in(dotPath);
        std::string dotContent((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        in.close();

        json r = {{"dot", dotContent}, {"svg", svgContent}};
        res.set_content(r.dump(), "application/json");
    });

    std::cout << "Backend corriendo en http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);
}
