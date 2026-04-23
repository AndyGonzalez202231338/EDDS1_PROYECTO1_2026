#ifndef GRAPH_H
#define GRAPH_H

#include <fstream>
/** Representa una arista en el grafo. 
 * Cada arista conecta dos sucursales (nodos) y tiene un tiempo de tránsito y un costo asociado.
 * El grafo se implementa como una lista de adyacencia, donde cada nodo tiene una lista de aristas 
 * que representan las conexiones a otras sucursales.
 * La función Dijkstra se implementa para encontrar la ruta óptima entre dos sucursales, 
 * ya sea por tiempo o por costo, utilizando un arreglo para almacenar el camino resultante y su longitud.
*/

struct Edge {
    int    destId;
    double tiempo;
    double costo;
    Edge*  next;   // lista enlazada de aristas adyacentes
};

struct GraphNode {
    int    branchId;
    Edge*  edges;    // cabeza de lista de aristas adyacentes
    GraphNode* nextNode;
};

class Graph {
public:
    Graph();
    ~Graph();

    void addBranch(int id);
    void addEdge(int originId, int destId, double tiempo, double costo, bool bidirectional);

    // Dijkstra retorna la ruta óptima como arreglo de IDs
    void dijkstraByTime(int origin, int dest, int* path, int& pathLen) const;
    void dijkstraByCost(int origin, int dest, int* path, int& pathLen) const;

    void toDot(std::ofstream& out) const;

private:
    GraphNode* _nodes;   // lista enlazada de nodos del grafo
    int        _count;

    GraphNode* findNode(int id) const;
    void       dijkstra(int origin, int dest, bool useCost, int* path, int& pathLen) const;
};

#endif // GRAPH_H