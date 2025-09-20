#include "fixed_containers/fixed_graph.hpp"
#include "fixed_containers/fixed_vector.hpp"

#include <iostream>

namespace
{
using Graph = fixed_containers::FixedGraph<int, void, 10, 5, true>;

void test_basic()
{
    Graph g;
    auto n0 = g.add_node(0);
    auto n1 = g.add_node(1);
    auto n2 = g.add_node(2);

    std::cout << "n0: " << n0 << ", n1: " << n1 << ", n2: " << n2 << std::endl;
    std::cout << "node_count: " << g.node_count() << std::endl;

    g.add_edge(n0, n1);
    g.add_edge(n1, n2);

    std::cout << "Neighbors of 0: " << g.neighbors(n0).size() << std::endl;
    std::cout << "Has edge 0-1: " << g.has_edge(n0, n1) << std::endl;
    std::cout << "Has edge 1-0: " << g.has_edge(n1, n0) << std::endl;

    std::cout << "BFS from 0: ";
    g.bfs(n0, [](auto idx) { std::cout << idx << " "; });
    std::cout << std::endl;

    std::cout << "DFS from 0: ";
    g.dfs(n0, [](auto idx) { std::cout << idx << " "; });
    std::cout << std::endl;

    auto path = g.shortest_path(n0, n2);
    std::cout << "Shortest path 0 to 2: ";
    for (auto p : path) std::cout << p << " ";
    std::cout << std::endl;
}

void test_new_features()
{
    std::cout << "\n=== Testing New Features ===\n";

    // Test cycle detection
    Graph g1;
    auto a = g1.add_node(0);
    auto b = g1.add_node(1);
    auto c = g1.add_node(2);
    g1.add_edge(a, b);
    g1.add_edge(b, c);
    g1.add_edge(c, a); // Creates a cycle

    std::cout << "Graph with cycle has cycles: " << g1.has_cycles() << std::endl;

    // Test acyclic graph
    Graph g2;
    auto x = g2.add_node(0);
    auto y = g2.add_node(1);
    auto z = g2.add_node(2);
    g2.add_edge(x, y);
    g2.add_edge(y, z);
    // No cycle

    std::cout << "Graph without cycle has cycles: " << g2.has_cycles() << std::endl;

    // Test connectivity (for undirected graphs)
    using UndirectedGraph = fixed_containers::FixedGraph<int, void, 10, 5, false>;
    UndirectedGraph ug;
    auto u1 = ug.add_node(0);
    auto u2 = ug.add_node(1);
    auto u3 = ug.add_node(2);
    ug.add_edge(u1, u2);
    ug.add_edge(u2, u3);

    std::cout << "Undirected graph is connected: " << ug.is_connected() << std::endl;

    // Test graph generators
    std::cout << "\n=== Graph Generators ===\n";
    auto complete = Graph::create_complete_graph(4);
    std::cout << "Complete graph (4 nodes) created with " << complete.node_count() << " nodes" << std::endl;

    auto cycle = Graph::create_cycle_graph(5);
    std::cout << "Cycle graph (5 nodes) created with " << cycle.node_count() << " nodes" << std::endl;

    // Test advanced features
    std::cout << "\n=== Advanced Graph Features ===\n";

    // Test bipartite checking
    using UndirectedGraphType = fixed_containers::FixedGraph<int, void, 10, 5, false>;
    UndirectedGraphType bipartite_graph;
    auto bp1 = bipartite_graph.add_node(0);
    auto bp2 = bipartite_graph.add_node(1);
    auto bp3 = bipartite_graph.add_node(2);
    auto bp4 = bipartite_graph.add_node(3);
    bipartite_graph.add_edge(bp1, bp2);
    bipartite_graph.add_edge(bp1, bp4);
    bipartite_graph.add_edge(bp2, bp3);
    bipartite_graph.add_edge(bp3, bp4);

    std::cout << "Bipartite graph is bipartite: " << bipartite_graph.is_bipartite() << std::endl;

    // Test graph properties
    std::cout << "Complete graph density: " << complete.density() << std::endl;
    std::cout << "Cycle graph diameter: " << cycle.diameter() << std::endl;

    // Test degree centrality
    auto degrees = complete.degree_centrality();
    std::cout << "Degree centrality of node 0 in complete graph: " << degrees[0] << std::endl;

    // Test topological sort
    using DirectedGraph = fixed_containers::FixedGraph<int, void, 10, 5, true>;
    DirectedGraph dag_graph;
    auto ts1 = dag_graph.add_node(0);
    auto ts2 = dag_graph.add_node(1);
    auto ts3 = dag_graph.add_node(2);
    auto ts4 = dag_graph.add_node(3);
    dag_graph.add_edge(ts1, ts2);
    dag_graph.add_edge(ts1, ts3);
    dag_graph.add_edge(ts2, ts4);
    dag_graph.add_edge(ts3, ts4);

    auto topo_order = dag_graph.topological_sort();
    std::cout << "Topological sort: ";
    for (auto node : topo_order) std::cout << node << " ";
    std::cout << std::endl;

    // Test strongly connected components
    DirectedGraph scc_graph;
    auto scc1 = scc_graph.add_node(0);
    auto scc2 = scc_graph.add_node(1);
    auto scc3 = scc_graph.add_node(2);
    auto scc4 = scc_graph.add_node(3);
    scc_graph.add_edge(scc1, scc2);
    scc_graph.add_edge(scc2, scc3);
    scc_graph.add_edge(scc3, scc1);
    scc_graph.add_edge(scc3, scc4);

    auto sccs = scc_graph.strongly_connected_components();
    std::cout << "Number of strongly connected components: " << sccs.size() << std::endl;

    // Test graph coloring
    auto colors = complete.greedy_coloring();
    std::cout << "Graph coloring used " << *std::max_element(colors.begin(), colors.end()) + 1 << " colors" << std::endl;
}

}  // namespace

int main()
{
    test_basic();
    test_new_features();
    return 0;
}