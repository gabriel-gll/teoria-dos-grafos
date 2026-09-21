#include "Graph.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>

Graph::Graph(Representation rep)
    : numVertices(0), numEdges(0), representation(rep) {}

// ---------------------------------------------------------------------------
// Entrada
// ---------------------------------------------------------------------------
void Graph::loadFromFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        throw std::runtime_error("Nao foi possivel abrir o arquivo: " + filename);
    }

    std::string line;

    // Primeira linha: numero de vertices.
    if (!std::getline(in, line)) {
        throw std::runtime_error("Arquivo de entrada vazio: " + filename);
    }
    {
        std::istringstream iss(line);
        if (!(iss >> numVertices)) {
            throw std::runtime_error("Formato invalido na primeira linha do arquivo.");
        }
    }

    numEdges = 0;

    if (representation == Representation::MATRIX) {
        adjMatrix.assign(numVertices + 1, std::vector<int>(numVertices + 1, 0));
    } else {
        adjList.assign(numVertices + 1, std::vector<int>());
    }

    // Linhas seguintes: arestas "u v".
    while (std::getline(in, line)) {
        if (line.find_first_not_of(" \t\r\n") == std::string::npos) {
            continue; // ignora linhas em branco
        }
        std::istringstream iss(line);
        int u, v;
        if (!(iss >> u >> v)) {
            continue; // ignora linhas malformadas
        }
        addEdge(u, v);
    }

    in.close();
}

void Graph::addEdge(int u, int v) {
    if (u < 1 || u > numVertices || v < 1 || v > numVertices) {
        throw std::runtime_error("Aresta referencia vertice fora do intervalo [1, n].");
    }

    if (representation == Representation::MATRIX) {
        adjMatrix[u][v] = 1;
        adjMatrix[v][u] = 1;
    } else {
        adjList[u].push_back(v);
        adjList[v].push_back(u);
    }

    ++numEdges;
}

std::vector<int> Graph::neighbors(int v) const {
    std::vector<int> result;
    if (representation == Representation::MATRIX) {
        result.reserve(numVertices);
        for (int u = 1; u <= numVertices; ++u) {
            if (adjMatrix[v][u]) {
                result.push_back(u);
            }
        }
    } else {
        result = adjList[v];
    }
    return result;
}

// ---------------------------------------------------------------------------
// Estatisticas de grau
// ---------------------------------------------------------------------------
int Graph::degree(int v) const {
    if (representation == Representation::MATRIX) {
        int d = 0;
        for (int u = 1; u <= numVertices; ++u) {
            d += adjMatrix[v][u];
        }
        return d;
    }
    return static_cast<int>(adjList[v].size());
}

int Graph::minDegree() const {
    int best = numVertices > 0 ? degree(1) : 0;
    for (int v = 2; v <= numVertices; ++v) {
        best = std::min(best, degree(v));
    }
    return best;
}

int Graph::maxDegree() const {
    int best = numVertices > 0 ? degree(1) : 0;
    for (int v = 2; v <= numVertices; ++v) {
        best = std::max(best, degree(v));
    }
    return best;
}

double Graph::averageDegree() const {
    if (numVertices == 0) return 0.0;
    long long sum = 0;
    for (int v = 1; v <= numVertices; ++v) {
        sum += degree(v);
    }
    return static_cast<double>(sum) / numVertices;
}

double Graph::medianDegree() const {
    if (numVertices == 0) return 0.0;
    std::vector<int> degrees(numVertices);
    for (int v = 1; v <= numVertices; ++v) {
        degrees[v - 1] = degree(v);
    }
    std::sort(degrees.begin(), degrees.end());

    size_t n = degrees.size();
    if (n % 2 == 1) {
        return degrees[n / 2];
    }
    return (degrees[n / 2 - 1] + degrees[n / 2]) / 2.0;
}

// ---------------------------------------------------------------------------
// BFS / DFS
// ---------------------------------------------------------------------------
Graph::SearchTree Graph::bfs(int start) const {
    SearchTree tree;
    tree.parent.assign(numVertices + 1, -1);
    tree.level.assign(numVertices + 1, -1);

    if (start < 1 || start > numVertices) {
        throw std::runtime_error("Vertice inicial fora do intervalo [1, n].");
    }

    std::vector<char> visited(numVertices + 1, 0);
    std::vector<int> queue;
    queue.reserve(numVertices);

    size_t head = 0;
    visited[start] = 1;
    tree.parent[start] = 0; // raiz nao tem pai; convencao: 0
    tree.level[start] = 0;
    queue.push_back(start);

    while (head < queue.size()) {
        int u = queue[head++];
        for (int w : neighbors(u)) {
            if (!visited[w]) {
                visited[w] = 1;
                tree.parent[w] = u;
                tree.level[w] = tree.level[u] + 1;
                queue.push_back(w);
            }
        }
    }

    return tree;
}

Graph::SearchTree Graph::dfs(int start) const {
    SearchTree tree;
    tree.parent.assign(numVertices + 1, -1);
    tree.level.assign(numVertices + 1, -1);

    if (start < 1 || start > numVertices) {
        throw std::runtime_error("Vertice inicial fora do intervalo [1, n].");
    }

    std::vector<char> visited(numVertices + 1, 0);

    // Pilha explicita para evitar estouro de pilha em grafos grandes.
    // Cada item: (vertice, pai, nivel)
    struct Frame { int v, parent, level; };
    std::vector<Frame> stack;
    stack.push_back({start, 0, 0});

    while (!stack.empty()) {
        Frame f = stack.back();
        stack.pop_back();

        if (visited[f.v]) continue;

        visited[f.v] = 1;
        tree.parent[f.v] = f.parent;
        tree.level[f.v] = f.level;

        // Empilha vizinhos em ordem reversa para manter uma ordem de
        // visita "natural" (menor vizinho explorado primeiro).
        std::vector<int> adj = neighbors(f.v);
        for (auto it = adj.rbegin(); it != adj.rend(); ++it) {
            if (!visited[*it]) {
                stack.push_back({*it, f.v, f.level + 1});
            }
        }
    }

    return tree;
}

static void writeSearchTreeToFile(const Graph::SearchTree& tree,
                                   int numVertices,
                                   const std::string& outFilename) {
    std::ofstream out(outFilename);
    if (!out.is_open()) {
        throw std::runtime_error("Nao foi possivel criar o arquivo: " + outFilename);
    }
    out << "vertice pai nivel\n";
    for (int v = 1; v <= numVertices; ++v) {
        out << v << " " << tree.parent[v] << " " << tree.level[v] << "\n";
    }
    out.close();
}

void Graph::bfs(int start, const std::string& outFilename) const {
    SearchTree tree = bfs(start);
    writeSearchTreeToFile(tree, numVertices, outFilename);
}

void Graph::dfs(int start, const std::string& outFilename) const {
    SearchTree tree = dfs(start);
    writeSearchTreeToFile(tree, numVertices, outFilename);
}

// ---------------------------------------------------------------------------
// Distancias e diametro
// ---------------------------------------------------------------------------
std::vector<int> Graph::bfsLevels(int start) const {
    return bfs(start).level;
}

int Graph::distance(int u, int v) const {
    if (u < 1 || u > numVertices || v < 1 || v > numVertices) {
        throw std::runtime_error("Vertice fora do intervalo [1, n].");
    }
    std::vector<int> levels = bfsLevels(u);
    return levels[v]; // -1 se nao alcancavel
}

int Graph::diameter() const {
    int best = 0;
    for (int v = 1; v <= numVertices; ++v) {
        std::vector<int> levels = bfsLevels(v);
        for (int u = 1; u <= numVertices; ++u) {
            if (levels[u] > best) {
                best = levels[u];
            }
        }
    }
    return best;
}

int Graph::diameterApprox() const {
    if (numVertices == 0) return 0;

    // 1a BFS a partir de um vertice arbitrario (vertice 1).
    std::vector<int> levels1 = bfsLevels(1);
    int a = 1, bestDist = 0;
    for (int v = 1; v <= numVertices; ++v) {
        if (levels1[v] > bestDist) {
            bestDist = levels1[v];
            a = v;
        }
    }

    // 2a BFS a partir do vertice mais distante encontrado.
    std::vector<int> levels2 = bfsLevels(a);
    int approx = 0;
    for (int v = 1; v <= numVertices; ++v) {
        if (levels2[v] > approx) {
            approx = levels2[v];
        }
    }

    return approx;
}

// ---------------------------------------------------------------------------
// Componentes conexas
// ---------------------------------------------------------------------------
Graph::Components Graph::connectedComponents() const {
    std::vector<char> visited(numVertices + 1, 0);
    Components components;

    for (int start = 1; start <= numVertices; ++start) {
        if (visited[start]) continue;

        std::vector<int> component;
        std::vector<int> queue;
        size_t head = 0;

        visited[start] = 1;
        queue.push_back(start);

        while (head < queue.size()) {
            int u = queue[head++];
            component.push_back(u);
            for (int w : neighbors(u)) {
                if (!visited[w]) {
                    visited[w] = 1;
                    queue.push_back(w);
                }
            }
        }

        std::sort(component.begin(), component.end());
        components.push_back(std::move(component));
    }

    std::sort(components.begin(), components.end(),
              [](const std::vector<int>& a, const std::vector<int>& b) {
                  return a.size() > b.size();
              });

    return components;
}

// ---------------------------------------------------------------------------
// Saida: informacoes gerais do grafo
// ---------------------------------------------------------------------------
void Graph::writeGraphInfo(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
        throw std::runtime_error("Nao foi possivel criar o arquivo: " + filename);
    }

    out << "numero de vertices: " << numVertices << "\n";
    out << "numero de arestas: " << numEdges << "\n";
    out << "grau minimo: " << minDegree() << "\n";
    out << "grau maximo: " << maxDegree() << "\n";
    out << "grau medio: " << averageDegree() << "\n";
    out << "mediana de grau: " << medianDegree() << "\n";
    out << "\n";

    Components components = connectedComponents();
    out << "numero de componentes conexas: " << components.size() << "\n";
    for (size_t i = 0; i < components.size(); ++i) {
        out << "componente " << (i + 1) << " (tamanho " << components[i].size() << "): ";
        for (size_t j = 0; j < components[i].size(); ++j) {
            out << components[i][j];
            if (j + 1 < components[i].size()) out << " ";
        }
        out << "\n";
    }

    out.close();
}
