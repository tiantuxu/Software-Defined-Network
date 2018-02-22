//
// Created by teddyxu on 1/23/18.
//

#ifndef ECE50863_PROJECT1_GRAPH_H
#define ECE50863_PROJECT1_GRAPH_H
#include <utility>
#include <list>
#include <vector>

struct edge
{
    int to;
    int length;
};

using path = std::vector<int>;
using node = std::vector<edge>;
using graph = std::vector<node>;

struct result {
    int distance;
    path p;
};

void add_edge( graph& g, int start, int finish, int length );
result dijkstra(const graph &graph, int source, int target);

/*
using namespace std;
# define INF 0x3f3f3f3f

// This class represents a directed graph using
// adjacency list representation
class Graph
{
    int V;    // No. of vertices

    // In a weighted graph, we need to store vertex
    // and weight pair for every edge
    list< pair<int, int> > *adj;

public:
    Graph(int V);  // Constructor

    // function to add an edge to graph
    void addEdge(int u, int v, int w);

    // prints shortest path from s
    void shortestPath(int s);
};
*/
#endif //ECE50863_PROJECT1_GRAPH_H
