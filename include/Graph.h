#ifndef GRAPH_H
#define GRAPH_H

#include <fstream>
#include <functional>
#include <string>

struct Edge {
    int    destId;
    double tiempo;
    double costo;
    Edge*  next;
};

struct GraphNode {
    int    branchId;
    Edge*  edges;
    GraphNode* nextNode;
};

// Result returned by shortestPath methods
struct PathResult {
    int    path[200];
    int    length;
    double total;
};

class Graph {
public:
    Graph();
    ~Graph();

    void addBranch(int id);

    // Returns false if origin==dest (loop), edge already exists, or nodes don't exist
    bool addEdge(int originId, int destId, double tiempo, double costo, bool bidirectional);

    bool removeEdge(int originId, int destId, bool bidirectional = false);

    // True if a directed edge from->to exists
    bool hasEdge(int fromId, int toId) const;

    // Itera sobre vecinos del nodo: fn(destId, tiempo, costo)
    void forEachNeighbor(int branchId,
                         const std::function<void(int, double, double)>& fn) const;

    // Peso de una arista directa; -1.0 si no existe
    double edgeWeight(int fromId, int toId, bool useCost) const;

    // Dijkstra — retorna ruta óptima con distancia total
    PathResult shortestPathByTime(int origin, int dest) const;
    PathResult shortestPathByCost(int origin, int dest) const;

    // Legacy wrappers (array output)
    void dijkstraByTime(int origin, int dest, int* path, int& pathLen) const;
    void dijkstraByCost(int origin, int dest, int* path, int& pathLen) const;

    int nodeCount() const { return _count; }

    // DOT export — overload with name lookup for node labels
    void toDot(std::ofstream& out) const;
    void toDot(std::ofstream& out, const std::function<std::string(int)>& nameFor) const;

private:
    GraphNode* _nodes;
    int        _count;

    GraphNode* findNode(int id) const;
    void       dijkstra(int origin, int dest, bool useCost, int* path, int& pathLen) const;
};

#endif // GRAPH_H