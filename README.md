# Biblioteca de Manipulação de Grafos — COS 242

Trabalho Prático 1 da disciplina **Teoria dos Grafos (COS 242, 2026/2)**.

Biblioteca em C++17 para representar e manipular grafos não-direcionados, com
duas representações internas (matriz de adjacência e lista de adjacência) e um
conjunto de algoritmos clássicos, todos escritos uma única vez e reutilizados
por ambas as representações.

## Funcionalidades

- Leitura de um grafo a partir de arquivo texto
- Representação por **matriz de adjacência** ou **lista de adjacência**,
  escolhida pelo usuário da biblioteca
- Estatísticas do grafo: número de vértices, número de arestas, grau mínimo,
  máximo, médio e mediana de grau
- Geração de arquivo de saída com as estatísticas acima e a lista de
  componentes conexas (em ordem decrescente de tamanho)
- Busca em largura (BFS) e busca em profundidade (DFS) a partir de um vértice
  escolhido pelo usuário, com geração da árvore de busca (pai e nível de cada
  vértice)
- Distância entre dois vértices (usando BFS como primitiva)
- Diâmetro **exato** (O(n·(n+m))) e uma heurística **aproximada**
  (*double sweep*, O(n+m)) para grafos muito grandes
- Identificação de componentes conexas (quantidade, tamanho e vértices de
  cada uma)

## Estrutura do projeto

```
.
├── Graph.hpp            # Interface publica da biblioteca (classe Graph)
├── Graph.cpp            # Implementacao dos algoritmos
├── MemoryUtils.hpp       # Utilitario para medir memoria do processo (RSS)
├── main.cpp             # Programa de demonstracao / uso pontual (graph_program)
├── case_study.cpp       # Programa que automatiza os estudos de caso
├── Makefile
└── exemplo.txt          # Grafo de exemplo do enunciado (Figura 1)
```

## Compilando

Requisitos: `g++` com suporte a C++17 e `make`.

```bash
make
```

Gera dois executáveis: `graph_program` e `case_study`.

```bash
make clean
```

Remove binários, objetos e arquivos `.txt` gerados por execuções anteriores.

## Uso

### `graph_program` — uso pontual / demonstração da biblioteca

```bash
./graph_program <arquivo_do_grafo> <matrix|list> [vertice_inicial]
```

Exemplo:

```bash
./graph_program exemplo.txt list 1
```

Gera três arquivos de saída:

| Arquivo | Conteúdo |
|---|---|
| `output_info.txt` | número de vértices/arestas, graus, componentes conexas |
| `output_bfs.txt` | árvore de BFS a partir do vértice inicial (pai e nível) |
| `output_dfs.txt` | árvore de DFS a partir do vértice inicial (pai e nível) |

Também imprime no terminal o diâmetro (exato e aproximado) e a distância
entre o primeiro e o último vértice, como exemplo.

> **Atenção:** com a representação `matrix`, o diâmetro exato custa
> `O(n³)` e pode levar muitos minutos em grafos com dezenas de milhares de
> vértices ou mais. Use `list` nesses casos, ou aguarde — não é um travamento,
> apenas um cálculo genuinamente caro.

### `case_study` — automação dos estudos de caso

```bash
./case_study <arquivo_do_grafo> [--rep=matrix|list|both] [--threshold=N]
```

| Opção | Padrão | Descrição |
|---|---|---|
| `--rep` | `both` | `matrix` mede só a matriz; `list` só a lista; `both` roda as duas na mesma execução |
| `--threshold` | `20000` | acima desse número de vértices, o diâmetro **exato** é pulado (só o aproximado é calculado) |

Exemplo:

```bash
./case_study grafo_1.txt --rep=list --threshold=20000
```

Esse programa mede e imprime automaticamente:

- Memória (RSS) e tempo médio de 100 execuções de BFS e de DFS, por
  representação
- Pai e nível dos vértices 10, 20 e 30, nas árvores geradas a partir dos
  vértices 1, 2 e 3
- Distância entre os pares (10,20), (10,30) e (20,30)
- Número de componentes conexas e tamanho da maior e da menor
- Diâmetro exato (quando o grafo está abaixo do `--threshold`) e aproximado

Para isolar completamente a medição de memória de cada representação (sem
influência de memória retida pelo alocador de uma execução anterior), rode o
programa duas vezes, uma para cada representação:

```bash
./case_study grafo_1.txt --rep=matrix
./case_study grafo_1.txt --rep=list
```

## Formato do arquivo de entrada

```
<numero de vertices>
<u1> <v1>
<u2> <v2>
...
```

A primeira linha contém o número de vértices do grafo (numerados de 1 a n).
Cada linha seguinte descreve uma aresta. Exemplo (`exemplo.txt`, grafo da
Figura 1 do enunciado):

```
5
1 2
2 5
5 3
4 5
1 5
```

## Complexidade assintótica

`n` = número de vértices, `m` = número de arestas. Toda a diferença de custo
entre as duas representações vem do acesso aos vizinhos de um vértice
(`neighbors()`), usado internamente por todos os algoritmos abaixo.

| Operação | Matriz | Lista |
|---|---|---|
| Inserir aresta | O(1) | O(1) amortizado |
| Vizinhos de um vértice | O(n) | O(grau(v)) |
| Grau de um vértice | O(n) | O(1) |
| Grau mín./máx./médio | O(n²) | O(n) |
| Mediana de grau | O(n²) + O(n log n) | O(n) + O(n log n) |
| BFS / DFS completas | O(n²) | O(n + m) |
| Distância entre 2 vértices | O(n²) | O(n + m) |
| Diâmetro exato | O(n³) | O(n·(n + m)) |
| Diâmetro aproximado | O(n²) | O(n + m) |
| Componentes conexas | O(n²) | O(n + m) |

