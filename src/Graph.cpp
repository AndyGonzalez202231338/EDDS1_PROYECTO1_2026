#include "Graph.h"
#include <climits>
#include <fstream>

Graph::Graph() : _nodes(nullptr), _count(0) {}

Graph::~Graph() {
    GraphNode* cur = _nodes;
    while (cur) {
        Edge* e = cur->edges;
        while (e) {
            Edge* ne = e->next;
            delete e;
            e = ne;
        }
        GraphNode* nn = cur->nextNode;
        delete cur;
        cur = nn;
    }
}

void Graph::addBranch(int id) {
    if (findNode(id)) return;
    GraphNode* node = new GraphNode{id, nullptr, _nodes};
    _nodes = node;
    ++_count;
}

void Graph::addEdge(int originId, int destId, double tiempo, double costo, bool bidirectional) {
    GraphNode* o = findNode(originId);
    if (!o) { addBranch(originId); o = findNode(originId); }
    GraphNode* d = findNode(destId);
    if (!d) { addBranch(destId); d = findNode(destId); }

    Edge* e = new Edge{destId, tiempo, costo, o->edges};
    o->edges = e;

    if (bidirectional) {
        Edge* re = new Edge{originId, tiempo, costo, d->edges};
        d->edges = re;
    }
}

bool Graph::removeEdge(int originId, int destId, bool bidirectional) {
    auto removeOne = [&](int from, int to) -> bool {
        GraphNode* node = findNode(from);
        if (!node) return false;
        Edge* cur  = node->edges;
        Edge* prev = nullptr;
        while (cur) {
            if (cur->destId == to) {
                if (prev) prev->next = cur->next;
                else       node->edges = cur->next;
                delete cur;
                return true;
            }
            prev = cur;
            cur  = cur->next;
        }
        return false;
    };

    bool removed = removeOne(originId, destId);
    if (bidirectional) removeOne(destId, originId);
    return removed;
}

void Graph::forEachNeighbor(int branchId,
                             const std::function<void(int, double, double)>& fn) const {
    GraphNode* node = findNode(branchId);
    if (!node) return;
    for (Edge* e = node->edges; e; e = e->next)
        fn(e->destId, e->tiempo, e->costo);
}

double Graph::edgeWeight(int fromId, int toId, bool useCost) const {
    GraphNode* node = findNode(fromId);
    if (!node) return -1.0;
    for (Edge* e = node->edges; e; e = e->next)
        if (e->destId == toId) return useCost ? e->costo : e->tiempo;
    return -1.0;
}

GraphNode* Graph::findNode(int id) const {
    GraphNode* cur = _nodes;
    while (cur) {
        if (cur->branchId == id) return cur;
        cur = cur->nextNode;
    }
    return nullptr;
}

void Graph::dijkstra(int origin, int dest, bool useCost, int* path, int& pathLen) const {
    const int INF = INT_MAX;
    const int MAX = 200;
    double dist[MAX];
    int    prev[MAX];
    int    ids[MAX];
    bool   visited[MAX];
    int    n = 0;

    GraphNode* cur = _nodes;
    while (cur && n < MAX) {
        ids[n] = cur->branchId;
        dist[n] = INF;
        prev[n] = -1;
        visited[n] = false;
        ++n;
        cur = cur->nextNode;
    }

    auto idx = [&](int id) {
        for (int i = 0; i < n; ++i) if (ids[i] == id) return i;
        return -1;
    };

    int si = idx(origin);
    if (si < 0) { pathLen = 0; return; }
    dist[si] = 0;

    for (int iter = 0; iter < n; ++iter) {
        int u = -1;
        for (int i = 0; i < n; ++i)
            if (!visited[i] && (u < 0 || dist[i] < dist[u])) u = i;
        if (u < 0 || dist[u] == INF) break;
        visited[u] = true;
        GraphNode* un = findNode(ids[u]);
        for (Edge* e = un->edges; e; e = e->next) {
            int v = idx(e->destId);
            if (v < 0) continue;
            double w = useCost ? e->costo : e->tiempo;
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                prev[v] = u;
            }
        }
    }

    int di = idx(dest);
    if (di < 0 || dist[di] == INF) { pathLen = 0; return; }

    int tmp[MAX];
    int len = 0;
    for (int v = di; v >= 0; v = prev[v]) {
        tmp[len++] = ids[v];
        if (ids[v] == origin) break;
    }
    pathLen = len;
    for (int i = 0; i < len; ++i) path[i] = tmp[len - 1 - i];
}

void Graph::dijkstraByTime(int origin, int dest, int* path, int& pathLen) const {
    dijkstra(origin, dest, false, path, pathLen);
}

void Graph::dijkstraByCost(int origin, int dest, int* path, int& pathLen) const {
    dijkstra(origin, dest, true, path, pathLen);
}

void Graph::toDot(std::ofstream& out) const {
    out << "digraph G {\n";
    GraphNode* cur = _nodes;
    while (cur) {
        for (Edge* e = cur->edges; e; e = e->next) {
            out << "  " << cur->branchId << " -> " << e->destId
                << " [label=\"t=" << e->tiempo << ",c=" << e->costo << "\"];\n";
        }
        cur = cur->nextNode;
    }
    out << "}\n";
}
