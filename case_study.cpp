#include "Graph.hpp"
#include "MemoryUtils.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// case_study
//
// Programa auxiliar para responder as perguntas da secao "Estudos de Caso"
// do enunciado, usando a biblioteca Graph. Ele NAO altera a biblioteca --
// so chama as funcoes publicas dela na ordem certa e cronometra/mede o que
// e pedido.
//
// Uso:
//   ./case_study <arquivo_do_grafo> [limite_diametro_exato]
//
// "limite_diametro_exato" (opcional, padrao 20000): se o grafo tiver mais
// vertices que esse limite, o diametro EXATO (que e O(n*(n+m)), muito caro)
// e pulado e so o aproximado e calculado -- exatamente a ideia sugerida no
// enunciado ("implemente tambem um algoritmo aproximativo, que pode ser
// usado em grafos muito grandes").
//
// O programa roda a analise duas vezes, uma para cada representacao
// (MATRIX e LIST), para permitir a comparacao pedida nos itens 1, 2 e 3 dos
// estudos de caso. As perguntas que nao dependem da representacao (4, 5, 6,
// 7) sao respondidas uma unica vez, usando a lista de adjacencia (mais
// eficiente para grafos grandes/esparsos).
// ---------------------------------------------------------------------------

using Clock = std::chrono::high_resolution_clock;

// Roda `numRuns` BFS's (ou DFS's) a partir de vertices distintos e retorna o
// tempo MEDIO de uma execucao, em milissegundos. Os vertices de partida sao
// escolhidos ciclicamente entre 1 e n (se n < numRuns, alguns vertices se
// repetem -- o programa avisa isso ao usuario).
template <typename SearchFn>
static double averageSearchTimeMs(const Graph& g, int numRuns, SearchFn searchFn) {
    int n = g.getNumVertices();
    if (n == 0) return 0.0;

    double totalMs = 0.0;
    for (int i = 0; i < numRuns; ++i) {
        int start = (i % n) + 1; // vertices distintos enquanto i < n

        // Importante: cronometra SO o algoritmo de busca, sem leitura nem
        // escrita em disco, conforme pedido no enunciado.
        auto t0 = Clock::now();
        searchFn(g, start);
        auto t1 = Clock::now();

        std::chrono::duration<double, std::milli> elapsed = t1 - t0;
        totalMs += elapsed.count();
    }
    return totalMs / numRuns;
}

static void printSeparator() {
    std::cout << "----------------------------------------------------------\n";
}

// Roda a parte da analise que depende da representacao: memoria e tempo
// medio de BFS/DFS (perguntas 1, 2 e 3 dos estudos de caso).
static void runRepresentationAnalysis(const std::string& filename,
                                       Graph::Representation rep,
                                       const std::string& repName,
                                       int numSearchRuns) {
    printSeparator();
    std::cout << "Representacao: " << repName << "\n";
    printSeparator();

    double rssBefore = getCurrentRSSMB();

    Graph g(rep);
    g.loadFromFile(filename);

    double rssAfter = getCurrentRSSMB();
    double usedMB = rssAfter - rssBefore;

    std::cout << "Vertices: " << g.getNumVertices()
              << " | Arestas: " << g.getNumEdges() << "\n";
    std::cout << "Memoria RSS antes de carregar : " << std::fixed
              << std::setprecision(2) << rssBefore << " MB\n";
    std::cout << "Memoria RSS depois de carregar: " << rssAfter << " MB\n";
    std::cout << "Memoria usada pelo grafo (aprox.): " << usedMB << " MB\n";

    if (g.getNumVertices() < numSearchRuns) {
        std::cout << "[Aviso] grafo tem menos de " << numSearchRuns
                  << " vertices; vertices de partida serao reaproveitados"
                     " ciclicamente para completar as " << numSearchRuns
                  << " execucoes.\n";
    }

    double bfsAvgMs = averageSearchTimeMs(
        g, numSearchRuns,
        [](const Graph& graph, int start) { graph.bfs(start); });

    double dfsAvgMs = averageSearchTimeMs(
        g, numSearchRuns,
        [](const Graph& graph, int start) { graph.dfs(start); });

    std::cout << "Tempo medio de BFS (" << numSearchRuns << " execucoes): "
              << bfsAvgMs << " ms\n";
    std::cout << "Tempo medio de DFS (" << numSearchRuns << " execucoes): "
              << dfsAvgMs << " ms\n";
}

// Pergunta 4: pai dos vertices 10, 20, 30 nas arvores geradas por BFS e DFS
// quando a busca comeca nos vertices 1, 2, 3.
static void runParentQuestion(const Graph& g) {
    printSeparator();
    std::cout << "Pergunta 4: pai dos vertices 10, 20, 30 (BFS e DFS)\n";
    printSeparator();

    std::vector<int> targets = {10, 20, 30};
    std::vector<int> starts = {1, 2, 3};

    if (g.getNumVertices() < 30) {
        std::cout << "[Aviso] o grafo tem menos de 30 vertices; "
                     "esta pergunta nao se aplica a este grafo.\n";
        return;
    }

    for (int start : starts) {
        Graph::SearchTree bfsTree = g.bfs(start);
        Graph::SearchTree dfsTree = g.dfs(start);

        std::cout << "Busca iniciada no vertice " << start << ":\n";
        for (int t : targets) {
            std::cout << "  vertice " << t
                      << " -> pai(BFS) = " << bfsTree.parent[t]
                      << ", nivel(BFS) = " << bfsTree.level[t]
                      << " | pai(DFS) = " << dfsTree.parent[t]
                      << ", nivel(DFS) = " << dfsTree.level[t] << "\n";
        }
    }
}

// Pergunta 5: distancia entre os pares (10,20), (10,30), (20,30).
static void runDistanceQuestion(const Graph& g) {
    printSeparator();
    std::cout << "Pergunta 5: distancia entre pares de vertices\n";
    printSeparator();

    if (g.getNumVertices() < 30) {
        std::cout << "[Aviso] o grafo tem menos de 30 vertices; "
                     "esta pergunta nao se aplica a este grafo.\n";
        return;
    }

    std::vector<std::pair<int, int>> pairs = {{10, 20}, {10, 30}, {20, 30}};
    for (auto& p : pairs) {
        int d = g.distance(p.first, p.second);
        std::cout << "dist(" << p.first << ", " << p.second << ") = ";
        if (d == -1) {
            std::cout << "nao alcancavel (vertices em componentes diferentes)\n";
        } else {
            std::cout << d << "\n";
        }
    }
}

// Pergunta 6: componentes conexas.
static void runComponentsQuestion(const Graph& g) {
    printSeparator();
    std::cout << "Pergunta 6: componentes conexas\n";
    printSeparator();

    Graph::Components components = g.connectedComponents();
    std::cout << "Numero de componentes conexas: " << components.size() << "\n";
    if (!components.empty()) {
        std::cout << "Tamanho da maior componente : "
                  << components.front().size() << " vertices\n";
        std::cout << "Tamanho da menor componente : "
                  << components.back().size() << " vertices\n";
    }
    std::cout << "(a lista completa de vertices de cada componente e escrita "
                 "em output_info.txt via writeGraphInfo)\n";
}

// Pergunta 7: diametro exato e aproximado.
static void runDiameterQuestion(const Graph& g, int exactThreshold) {
    printSeparator();
    std::cout << "Pergunta 7: diametro\n";
    printSeparator();

    auto t0 = Clock::now();
    int approx = g.diameterApprox();
    auto t1 = Clock::now();
    std::chrono::duration<double, std::milli> approxMs = t1 - t0;

    std::cout << "Diametro aproximado (double sweep): " << approx
              << " (calculado em " << approxMs.count() << " ms)\n";

    if (g.getNumVertices() > exactThreshold) {
        std::cout << "[Info] grafo tem " << g.getNumVertices()
                  << " vertices, acima do limite de " << exactThreshold
                  << " definido para o calculo exato (O(n*(n+m)) e muito "
                     "caro nesse tamanho). Diametro exato NAO calculado.\n";
        return;
    }

    t0 = Clock::now();
    int exact = g.diameter();
    t1 = Clock::now();
    std::chrono::duration<double, std::milli> exactMs = t1 - t0;

    std::cout << "Diametro exato: " << exact
              << " (calculado em " << exactMs.count() << " ms)\n";
}

static void printUsage(const char* progName) {
    std::cerr << "Uso: " << progName
              << " <arquivo_do_grafo> [--rep=matrix|list|both] [--threshold=N]\n"
              << "\n"
              << "  --rep=both    (padrao) roda matrix e list na mesma execucao,\n"
              << "                pratico para ver tudo de uma vez.\n"
              << "  --rep=matrix  roda so a matriz de adjacencia, num processo\n"
              << "                isolado -- use isto (e depois --rep=list numa\n"
              << "                segunda chamada) se quiser a medicao de memoria\n"
              << "                mais rigorosa possivel, sem nenhuma influencia\n"
              << "                de alocacoes feitas pela outra representacao.\n"
              << "  --rep=list    roda so a lista de adjacencia, isolada.\n"
              << "  --threshold=N define o limite de vertices para o calculo do\n"
              << "                diametro EXATO (padrao 20000).\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string filename = argv[1];
    std::string repChoice = "both";
    int exactThreshold = 20000;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.rfind("--rep=", 0) == 0) {
            repChoice = arg.substr(6);
        } else if (arg.rfind("--threshold=", 0) == 0) {
            exactThreshold = std::stoi(arg.substr(12));
        } else {
            std::cerr << "Argumento nao reconhecido: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    if (repChoice != "both" && repChoice != "matrix" && repChoice != "list") {
        std::cerr << "--rep invalido: use matrix, list ou both.\n";
        return 1;
    }

    const int NUM_SEARCH_RUNS = 100;

    try {
        // --- Perguntas 1, 2 e 3: memoria e tempo -----------------------
        // Cada chamada de runRepresentationAnalysis cria e destroi seu
        // proprio Graph. Ainda assim, rodar as duas dentro do MESMO
        // processo (--rep=both) pode deixar a medicao de memoria da
        // segunda representacao levemente influenciada por memoria que o
        // alocador do processo reteve da primeira (o C++ libera a memoria
        // do objeto, mas o alocador nem sempre devolve isso ao sistema
        // operacional na hora). Para eliminar essa duvida por completo,
        // rode duas vezes o programa, uma com --rep=matrix e outra com
        // --rep=list: assim cada medicao acontece em um processo novo,
        // sem nenhum historico de alocacao anterior.
        if (repChoice == "both" || repChoice == "matrix") {
            runRepresentationAnalysis(filename, Graph::Representation::MATRIX,
                                       "MATRIZ DE ADJACENCIA", NUM_SEARCH_RUNS);
        }
        if (repChoice == "both" || repChoice == "list") {
            runRepresentationAnalysis(filename, Graph::Representation::LIST,
                                       "LISTA DE ADJACENCIA", NUM_SEARCH_RUNS);
        }

        // --- Perguntas 4 a 7 --------------------------------------------
        // Essas perguntas tem resultado matematico, nao de desempenho --
        // o pai/nivel de um vertice, a distancia entre dois vertices, as
        // componentes conexas e o diametro sao os MESMOS nas duas
        // representacoes (elas descrevem o mesmo grafo). Por isso rodamos
        // essa parte so uma vez, com LIST (mais leve), independente do
        // --rep escolhido acima -- nao ha necessidade de isolamento aqui.
        Graph g(Graph::Representation::LIST);
        g.loadFromFile(filename);

        runParentQuestion(g);
        runDistanceQuestion(g);
        runComponentsQuestion(g);
        runDiameterQuestion(g, exactThreshold);

        // Tambem gera o arquivo de informacoes gerais, com a lista completa
        // de vertices de cada componente conexa.
        g.writeGraphInfo("output_info.txt");

        printSeparator();
        

    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
