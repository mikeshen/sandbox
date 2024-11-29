#include <iostream>
#include <limits>
#include <queue>
#include <vector>
#include "test_runner.h"

using namespace std;

const int INF = numeric_limits<int>::max();

class Graph
{
   public:
    Graph(int vertices);
    void addEdge(int u, int v, int weight);
    void dijkstra(int start);

   private:
    int                            vertices;
    vector<vector<pair<int, int>>> adjList;
};

Graph::Graph(int vertices) : vertices(vertices)
{
    adjList.resize(vertices);
}

void Graph::addEdge(int u, int v, int weight)
{
    adjList[u].emplace_back(v, weight);
    adjList[v].emplace_back(u, weight); // For undirected graph
}

void Graph::dijkstra(int start)
{
    vector<int> dist(vertices, INF);
    dist[start] = 0;

    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    pq.emplace(0, start);

    while (!pq.empty()) {
        int u = pq.top().second;
        pq.pop();

        for (const auto& neighbor : adjList[u]) {
            int v      = neighbor.first;
            int weight = neighbor.second;

            if (dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                pq.emplace(dist[v], v);
            }
        }
    }

    cout << "Vertex\tDistance from Source\n";
    for (int i = 0; i < vertices; ++i) {
        cout << i << "\t" << dist[i] << "\n";
    }
}

void testDjikstra()
{
    int   vertices = 5;
    Graph g(vertices);

    g.addEdge(0, 1, 4);
    g.addEdge(0, 2, 2);
    g.addEdge(1, 2, 5);
    g.addEdge(1, 3, 10);
    g.addEdge(2, 4, 3);
    g.addEdge(4, 3, 4);

    int startVertex = 0;
    cout << "Dijkstra's Algorithm starting from vertex " << startVertex << ":\n";
    g.dijkstra(startVertex);
}

void runTests()
{
    vector<string> testResults;
    testResults.push_back(runTest("testDjikstra", testDjikstra));

    // Print test results
    for (const auto& result : testResults) {
        cout << result << endl;
    }
}

int main()
{
    runTests();

    return 0;
}
