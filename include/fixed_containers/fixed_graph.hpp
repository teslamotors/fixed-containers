#pragma once

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_map.hpp"
#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/preconditions.hpp"
#include "fixed_containers/source_location.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <cstring> // std::memcpy
#include <optional>


namespace fixed_containers
{
/**
 * Fixed-capacity graph with maximum nodes and edges declared at compile-time.
 * Supports directed and undirected graphs, weighted and unweighted edges.
 * Properties:
 *  - constexpr
 *  - no dynamic allocations
 *  - adjacency list or matrix representation (configurable)
 */
template <typename NodeType,
          typename EdgeType = void,
          std::size_t MAX_NODES = 100,
          std::size_t MAX_EDGES_PER_NODE = 10,
          bool DIRECTED = true,
          bool USE_MATRIX = false,
          bool USE_POOL = false,
          std::size_t MAX_TOTAL_EDGES = MAX_EDGES_PER_NODE * MAX_NODES>
class FixedGraph
{
public:
    using node_type = NodeType;
    using edge_type = EdgeType;
    static constexpr std::size_t max_nodes = MAX_NODES;
    static constexpr std::size_t max_edges_per_node = MAX_EDGES_PER_NODE;
    static constexpr bool use_matrix = USE_MATRIX;
    static constexpr bool use_pool = USE_POOL;
    static_assert(MAX_NODES > 0, "MAX_NODES must be > 0");
    static_assert(MAX_EDGES_PER_NODE > 0 || USE_MATRIX || USE_POOL, "MAX_EDGES_PER_NODE must be > 0 for adjacency list or pool");
    static_assert(!USE_MATRIX || MAX_EDGES_PER_NODE >= MAX_NODES, "For matrix, MAX_EDGES_PER_NODE should be at least MAX_NODES");
    static_assert(!USE_POOL || MAX_TOTAL_EDGES > 0, "MAX_TOTAL_EDGES must be > 0 for pool");

private:
    using NodeIndex = std::size_t;
    static constexpr NodeIndex INTERNAL_INVALID_INDEX = (std::numeric_limits<NodeIndex>::max)();

    // For edges: if EdgeType is void, use NodeIndex, else pair<NodeIndex, EdgeType>
    using EdgeStorage = typename std::conditional<std::is_void<EdgeType>::value,
                                           NodeIndex,
                                           std::pair<NodeIndex, EdgeType>>::type;
    using AdjacencyList = FixedVector<EdgeStorage, MAX_EDGES_PER_NODE>;
    using NodeList = FixedVector<std::pair<NodeType, NodeIndex>, MAX_NODES>;

    // For matrix representation
    using MatrixElement = typename std::conditional<std::is_void<EdgeType>::value,
                                                     bool,
                                                     std::optional<EdgeType>>::type;
    using AdjacencyMatrix = FixedVector<FixedVector<MatrixElement, MAX_NODES>, MAX_NODES>;

    // Storage
    using NonMatrixStorage = typename std::conditional<USE_POOL,
                                                       FixedVector<EdgeStorage, MAX_TOTAL_EDGES>,
                                                       FixedVector<AdjacencyList, MAX_NODES>>::type;
    using StorageType = typename std::conditional<USE_MATRIX,
                                                  AdjacencyMatrix,
                                                  NonMatrixStorage>::type;
    StorageType adjacency_storage_;

    // For pool mode: per-node start/end indices in the pool
    FixedVector<std::pair<size_t, size_t>, MAX_NODES> node_ranges_;

    NodeList node_list_;
    FixedVector<NodeType, MAX_NODES> index_to_node_;

    NodeIndex next_index_ = 0;


public:
    // Public constant for invalid index so callers don't rely on internal naming
    static constexpr NodeIndex INVALID_INDEX = INTERNAL_INVALID_INDEX;

public:
    constexpr FixedGraph() noexcept
    {
        if constexpr (USE_MATRIX) {
            adjacency_storage_.resize(MAX_NODES);
            for (auto& row : adjacency_storage_) {
                row.resize(MAX_NODES);
                if constexpr (std::is_void_v<EdgeType>) {
                    // bool is false by default
                } else {
                    for (auto& elem : row) {
                        elem = std::nullopt;
                    }
                }
            }
        } else if constexpr (USE_POOL) {
            // adjacency_storage_ is already sized
            node_ranges_.resize(MAX_NODES, {0, 0});
        } else {
            adjacency_storage_.resize(MAX_NODES, AdjacencyList{});
        }
    }

    // Generate a complete graph (every node connected to every other node)
    static constexpr FixedGraph create_complete_graph(std::size_t num_nodes)
    {
        FixedGraph graph{};
        for (std::size_t i = 0; i < num_nodes && i < MAX_NODES; ++i)
        {
            graph.add_node(static_cast<NodeType>(i));
        }

        for (std::size_t i = 0; i < num_nodes; ++i)
        {
            for (std::size_t j = i + 1; j < num_nodes; ++j)
            {
                graph.add_edge(i, j);
                if constexpr (!DIRECTED)
                    graph.add_edge(j, i);
            }
        }
        return graph;
    }

    // Generate a cycle graph
    static constexpr FixedGraph create_cycle_graph(std::size_t num_nodes)
    {
        FixedGraph graph{};
        for (std::size_t i = 0; i < num_nodes && i < MAX_NODES; ++i)
        {
            graph.add_node(static_cast<NodeType>(i));
        }

        for (std::size_t i = 0; i < num_nodes; ++i)
        {
            std::size_t next = (i + 1) % num_nodes;
            graph.add_edge(i, next);
            if constexpr (!DIRECTED)
                graph.add_edge(next, i);
        }
        return graph;
    }

    // Add a node, returns its index
    constexpr NodeIndex add_node(const NodeType& node)
    {
        for (const auto& p : node_list_)
        {
            if (p.first == node)
            {
                return p.second;
            }
        }
        if (node_list_.size() >= MAX_NODES)
        {
            return INVALID_INDEX; // Capacity exhausted
        }
        NodeIndex idx = next_index_++;
        node_list_.push_back({node, idx});
        index_to_node_.push_back(node);
        return idx;
    }

    // Return true if the node exists
    constexpr bool has_node(const NodeType& node) const noexcept
    {
        for (const auto& p : node_list_) { if (p.first == node) return true; }
        return false;
    }

    // Find node index or INVALID_INDEX if not present
    constexpr NodeIndex find_node_index(const NodeType& node) const noexcept
    {
        for (const auto& p : node_list_) { if (p.first == node) return p.second; }
        return INVALID_INDEX;
    }

    // Add an edge
    template <typename... Args>
    constexpr void add_edge(NodeIndex from, NodeIndex to, Args&&... args)
    {
        if (from >= next_index_ || to >= next_index_)
        {
            return;
        }
        if constexpr (USE_MATRIX) {
            if constexpr (std::is_void_v<EdgeType>) {
                adjacency_storage_[from][to] = true;
            } else {
                adjacency_storage_[from][to] = std::forward<Args...>(args...);
            }
        } else if constexpr (USE_POOL) {
            if (adjacency_storage_.size() >= MAX_TOTAL_EDGES)
            {
                return; // Capacity full
            }
            size_t insert_pos = adjacency_storage_.size();
            if constexpr (std::is_void_v<EdgeType>)
            {
                static_assert(sizeof...(Args) == 0, "No weight for void EdgeType");
                adjacency_storage_.push_back(to);
            }
            else
            {
                static_assert(sizeof...(Args) == 1, "Need weight for non-void EdgeType");
                adjacency_storage_.push_back({to, std::forward<Args>(args)...});
            }
            // Update node's range
            node_ranges_[from].second = insert_pos + 1;
        } else {
            if (adjacency_storage_[from].size() >= MAX_EDGES_PER_NODE)
            {
                return;
            }
            if constexpr (std::is_void_v<EdgeType>)
            {
                static_assert(sizeof...(Args) == 0, "No weight for void EdgeType");
                adjacency_storage_[from].push_back(to);
            }
            else
            {
                static_assert(sizeof...(Args) == 1, "Need weight for non-void EdgeType");
                adjacency_storage_[from].push_back({to, std::forward<Args>(args)...});
            }
        }
        if constexpr (!DIRECTED)
        {
            if constexpr (USE_MATRIX) {
                if constexpr (std::is_void_v<EdgeType>) {
                    adjacency_storage_[to][from] = true;
                } else {
                    adjacency_storage_[to][from] = std::forward<Args...>(args...);
                }
            } else if constexpr (USE_POOL) {
                if (adjacency_storage_.size() >= MAX_TOTAL_EDGES)
                {
                    return;
                }
                size_t insert_pos = adjacency_storage_.size();
                if constexpr (std::is_void_v<EdgeType>)
                {
                    adjacency_storage_.push_back(from);
                }
                else
                {
                    adjacency_storage_.push_back({from, std::forward<Args>(args)...});
                }
                node_ranges_[to].second = insert_pos + 1;
            } else {
                if (adjacency_storage_[to].size() >= MAX_EDGES_PER_NODE)
                {
                    return;
                }
                if constexpr (std::is_void_v<EdgeType>)
                {
                    adjacency_storage_[to].push_back(from);
                }
                else
                {
                    adjacency_storage_[to].push_back({from, std::forward<Args>(args)...});
                }
            }
        }
    }

    // Check if edge exists
    constexpr bool has_edge(NodeIndex from, NodeIndex to) const
    {
        if (from >= next_index_ || to >= next_index_)
        {
            return false;
        }
        if constexpr (USE_MATRIX) {
            if constexpr (std::is_void_v<EdgeType>) {
                return adjacency_storage_[from][to];
            } else {
                return adjacency_storage_[from][to].has_value();
            }
        } else if constexpr (USE_POOL) {
            auto [start, end] = node_ranges_[from];
            for (size_t i = start; i < end; ++i) {
                if (get_neighbor(adjacency_storage_[i]) == to) return true;
            }
            return false;
        } else {
            const auto& list = adjacency_storage_[from];
            if constexpr (std::is_void_v<EdgeType>)
            {
                return std::find(list.begin(), list.end(), to) != list.end();
            }
            else
            {
                return std::find_if(list.begin(), list.end(), [to](const auto& p) { return p.first == to; }) != list.end();
            }
        }
    }

    // Get neighbors
    constexpr FixedVector<EdgeStorage, MAX_NODES> neighbors(NodeIndex node) const
    {
        FixedVector<EdgeStorage, MAX_NODES> result{};
        if (node >= next_index_) {
            return result;
        }
        if constexpr (USE_MATRIX) {
            for (NodeIndex v = 0; v < next_index_; ++v) {
                if constexpr (std::is_void<EdgeType>::value) {
                    if (adjacency_storage_[node][v]) {
                        result.push_back(v);
                    }
                } else {
                    if (adjacency_storage_[node][v].has_value()) {
                        result.push_back({v, *adjacency_storage_[node][v]});
                    }
                }
            }
        } else if constexpr (USE_POOL) {
            auto [start, end] = node_ranges_[node];
            for (size_t i = start; i < end; ++i) {
                result.push_back(adjacency_storage_[i]);
            }
        } else {
            const auto& list = adjacency_storage_[node];
            for (const auto& e : list) {
                result.push_back(e);
            }
        }
        return result;
    }

    // Get node count
    constexpr std::size_t node_count() const noexcept { return next_index_; }

    // Count edges (directed counts each directed edge; undirected counts each undirected edge once)
    constexpr std::size_t edge_count() const noexcept
    {
        std::size_t count = 0;
        if constexpr (USE_MATRIX) {
            for (NodeIndex u = 0; u < next_index_; ++u) {
                for (NodeIndex v = 0; v < next_index_; ++v) {
                    if constexpr (std::is_void<EdgeType>::value) {
                        if (adjacency_storage_[u][v]) {
                            if constexpr (DIRECTED) {
                                ++count;
                            } else {
                                if (u < v) ++count;
                            }
                        }
                    } else {
                        if (adjacency_storage_[u][v].has_value()) {
                            if constexpr (DIRECTED) {
                                ++count;
                            } else {
                                if (u < v) ++count;
                            }
                        }
                    }
                }
            }
        } else {
            for (NodeIndex u = 0; u < next_index_; ++u)
            {
                if constexpr (DIRECTED)
                {
                    count += adjacency_storage_[u].size();
                }
                else
                {
                    for (const auto& e : adjacency_storage_[u])
                    {
                        NodeIndex v = get_neighbor(e);
                        if (u < v) ++count;
                    }
                }
            }
        }
        return count;
    }

    // Get node by index
    constexpr const NodeType& node_at(NodeIndex idx) const { return index_to_node_[idx]; }

    // Remove an edge; returns true if removed. For undirected graphs removes symmetric edge too.
    constexpr bool remove_edge(NodeIndex from, NodeIndex to)
    {
        if (from >= next_index_ || to >= next_index_) return false;
        bool removed = false;
        auto& list = adjacency_storage_[from];
        if constexpr (std::is_void_v<EdgeType>)
        {
            for (auto it = list.begin(); it != list.end(); ++it)
            {
                if (*it == to) { list.erase(it); removed = true; break; }
            }
        }
        else
        {
            for (auto it = list.begin(); it != list.end(); ++it)
            {
                if (it->first == to) { list.erase(it); removed = true; break; }
            }
        }
        if constexpr (!DIRECTED)
        {
            if (removed)
            {
                auto& list2 = adjacency_storage_[to];
                if constexpr (std::is_void_v<EdgeType>)
                {
                    for (auto it = list2.begin(); it != list2.end(); ++it)
                    { if (*it == from) { list2.erase(it); break; } }
                }
                else
                {
                    for (auto it = list2.begin(); it != list2.end(); ++it)
                    { if (it->first == from) { list2.erase(it); break; } }
                }
            }
        }
        return removed;
    }

    // BFS traversal
    template <typename Visitor>
    constexpr void bfs(NodeIndex start, Visitor visitor) const
    {
        if (start >= next_index_) return;
        FixedVector<bool, MAX_NODES> visited{}; visited.resize(MAX_NODES, false);
        FixedVector<NodeIndex, MAX_NODES> queue{}; queue.push_back(start); visited[start] = true;
        std::size_t head = 0;
        while (head < queue.size())
        {
            NodeIndex current = queue[head++];
            visitor(current);
            for (const auto& edge : neighbors(current))
            {
                NodeIndex neighbor = get_neighbor(edge);
                if (!visited[neighbor]) { visited[neighbor] = true; queue.push_back(neighbor); }
            }
        }
    }

    // DFS traversal
    template <typename Visitor>
    constexpr void dfs(NodeIndex start, Visitor visitor) const
    {
        if (start >= next_index_)
            return;
        FixedVector<bool, MAX_NODES> visited{};
        visited.resize(MAX_NODES, false);
        FixedVector<NodeIndex, MAX_NODES> stack{};
        stack.push_back(start);
        while (!stack.empty())
        {
            NodeIndex current = stack.back();
            stack.pop_back();
            if (!visited[current])
            {
                visited[current] = true;
                visitor(current);
                for (const auto& edge : neighbors(current)) {
                    NodeIndex neighbor = get_neighbor(edge);
                    if (!visited[neighbor]) {
                        stack.push_back(neighbor);
                    }
                }
            }
        }
    }

    // Shortest path using BFS (for unweighted graphs)
    constexpr FixedVector<NodeIndex, MAX_NODES> shortest_path(NodeIndex start, NodeIndex end) const
    {
        FixedVector<NodeIndex, MAX_NODES> path{};
        if (start >= next_index_ || end >= next_index_ || start == end)
        {
            if (start == end && start < next_index_)
                path.push_back(start);
            return path;
        }

        FixedVector<NodeIndex, MAX_NODES> parent{};
        parent.resize(MAX_NODES, INVALID_INDEX);
        FixedVector<bool, MAX_NODES> visited{};
        visited.resize(MAX_NODES, false);
        FixedVector<NodeIndex, MAX_NODES> queue{}; queue.push_back(start); visited[start] = true;
        bool found = false; std::size_t head = 0;
        while (head < queue.size() && !found)
        {
            NodeIndex current = queue[head++];
            for (const auto& edge : neighbors(current))
            {
                NodeIndex neighbor = get_neighbor(edge);
                if (!visited[neighbor]) { visited[neighbor] = true; parent[neighbor] = current; queue.push_back(neighbor); if (neighbor == end) { found = true; break; } }
            }
        }

        // Reconstruct path (build reversed then reverse in-place to avoid O(n^2) inserts)
        if (found)
        {
            FixedVector<NodeIndex, MAX_NODES> rev{};
            NodeIndex current = end;
            while (current != INVALID_INDEX)
            {
                rev.push_back(current);
                current = parent[current];
            }
            // Reverse into path
            for (std::size_t i = 0; i < rev.size(); ++i)
            {
                path.push_back(rev[rev.size() - 1 - i]);
            }
        }

        return path;
    }

    // Topological sort (for directed graphs)
    constexpr FixedVector<NodeIndex, MAX_NODES> topological_sort() const
        requires(DIRECTED)
    {
        FixedVector<NodeIndex, MAX_NODES> result{};
        FixedVector<std::size_t, MAX_NODES> in_degree{};
        in_degree.resize(MAX_NODES, 0);

        // Calculate in-degrees
        for (NodeIndex i = 0; i < next_index_; ++i)
        {
            for (const auto& edge : neighbors(i))
            {
                NodeIndex neighbor = get_neighbor(edge);
                if (neighbor < MAX_NODES)
                    in_degree[neighbor]++;
            }
        }

        // Find nodes with no incoming edges
        FixedVector<NodeIndex, MAX_NODES> queue{};
        for (NodeIndex i = 0; i < next_index_; ++i)
        {
            if (in_degree[i] == 0)
                queue.push_back(i);
        }
        std::size_t head = 0;
        while (head < queue.size())
        {
            NodeIndex current = queue[head++];
            result.push_back(current);

            // Reduce in-degree of neighbors
            for (const auto& edge : neighbors(current))
            {
                NodeIndex neighbor = get_neighbor(edge);
                if (neighbor < MAX_NODES)
                {
                    in_degree[neighbor]--;
                    if (in_degree[neighbor] == 0)
                        queue.push_back(neighbor);
                }
            }
        }

        // Check for cycles (if not all nodes are included)
        if (result.size() != next_index_)
            return {}; // Cycle detected

        return result;
    }

    // Check if graph is connected (for undirected graphs)
    constexpr bool is_connected() const
        requires(!DIRECTED)
    {
        if (next_index_ == 0) return true;
        if (next_index_ == 1) return true;

        FixedVector<bool, MAX_NODES> visited{};
        visited.resize(MAX_NODES, false);
        FixedVector<NodeIndex, MAX_NODES> stack{};

        // Start DFS from node 0
        stack.push_back(0);
        visited[0] = true;
        std::size_t visited_count = 1;

        while (!stack.empty())
        {
            NodeIndex current = stack.back();
            stack.pop_back();

            for (const auto& edge : neighbors(current))
            {
                NodeIndex neighbor = get_neighbor(edge);
                if (!visited[neighbor])
                {
                    visited[neighbor] = true;
                    visited_count++;
                    stack.push_back(neighbor);
                }
            }
        }

        return visited_count == next_index_;
    }

    // Dijkstra's shortest path algorithm for weighted graphs
    template <typename WeightType = EdgeType>
    constexpr FixedVector<NodeIndex, MAX_NODES> dijkstra_shortest_path(NodeIndex start, NodeIndex end) const
        requires(!std::is_void_v<EdgeType> && std::is_arithmetic_v<WeightType>)
    {
        using DistanceType = WeightType;
        constexpr DistanceType INF = std::numeric_limits<DistanceType>::max() / 2;

        FixedVector<DistanceType, MAX_NODES> distances{};
        distances.resize(MAX_NODES, INF);
        FixedVector<NodeIndex, MAX_NODES> previous{};
        previous.resize(MAX_NODES, INVALID_INDEX);
        FixedVector<bool, MAX_NODES> visited{};
        visited.resize(MAX_NODES, false);

        distances[start] = 0;

        // Simple priority queue implementation using FixedVector
        auto find_min_distance = [&](const FixedVector<bool, MAX_NODES>& visited) -> NodeIndex {
            DistanceType min_dist = INF;
            NodeIndex min_node = INVALID_INDEX;
            for (NodeIndex i = 0; i < next_index_; ++i) {
                if (!visited[i] && distances[i] < min_dist) {
                    min_dist = distances[i];
                    min_node = i;
                }
            }
            return min_node;
        };

        for (NodeIndex count = 0; count < next_index_; ++count) {
            NodeIndex u = find_min_distance(visited);
            if (u == INVALID_INDEX || distances[u] == INF) break;

            visited[u] = true;

            for (const auto& edge : neighbors(u)) {
                NodeIndex v = get_neighbor(edge);
                DistanceType weight = get_weight(edge);
                if (!visited[v] && distances[u] != INF && distances[u] + weight < distances[v]) {
                    distances[v] = distances[u] + weight;
                    previous[v] = u;
                }
            }
        }

        // Reconstruct path (reversed accumulation then reverse)
        FixedVector<NodeIndex, MAX_NODES> path{};
        if (distances[end] == INF) return path; // No path found
        FixedVector<NodeIndex, MAX_NODES> rev{};
        for (NodeIndex at = end; at != INVALID_INDEX; at = previous[at]) {
            rev.push_back(at);
        }
        for (std::size_t i = 0; i < rev.size(); ++i) {
            path.push_back(rev[rev.size() - 1 - i]);
        }
        return path;
    }

    // Bellman-Ford algorithm for graphs with negative weights
    template <typename WeightType = EdgeType>
    constexpr std::pair<FixedVector<WeightType, MAX_NODES>, bool> bellman_ford_shortest_paths(NodeIndex start) const
        requires(!std::is_void_v<EdgeType> && std::is_arithmetic_v<WeightType>)
    {
        using DistanceType = WeightType;
        constexpr DistanceType INF = std::numeric_limits<DistanceType>::max() / 2;

        FixedVector<DistanceType, MAX_NODES> distances{};
        distances.resize(MAX_NODES, INF);
        distances[start] = 0;

        // Relax edges |V|-1 times
        for (NodeIndex i = 0; i < next_index_ - 1; ++i) {
            for (NodeIndex u = 0; u < next_index_; ++u) {
                for (const auto& edge : neighbors(u)) {
                    NodeIndex v = get_neighbor(edge);
                    DistanceType weight = get_weight(edge);
                    if (distances[u] != INF && distances[u] + weight < distances[v]) {
                        distances[v] = distances[u] + weight;
                    }
                }
            }
        }

        // Check for negative cycles
        bool has_negative_cycle = false;
        for (NodeIndex u = 0; u < next_index_; ++u) {
            for (const auto& edge : neighbors(u)) {
                NodeIndex v = get_neighbor(edge);
                DistanceType weight = get_weight(edge);
                if (distances[u] != INF && distances[u] + weight < distances[v]) {
                    has_negative_cycle = true;
                    break;
                }
            }
            if (has_negative_cycle) break;
        }

        return {distances, has_negative_cycle};
    }

    // Minimum spanning tree using Kruskal's algorithm
    template <typename WeightType = EdgeType>
    constexpr FixedVector<std::pair<NodeIndex, NodeIndex>, MAX_NODES * MAX_NODES> kruskal_mst() const
        requires(!std::is_void_v<EdgeType> && std::is_arithmetic_v<WeightType> && !DIRECTED)
    {
        // Union-Find structure
        FixedVector<NodeIndex, MAX_NODES> parent{};
        parent.resize(MAX_NODES);
        for (NodeIndex i = 0; i < next_index_; ++i) parent[i] = i;

        auto find = [&](auto& self, NodeIndex x) -> NodeIndex {
            return parent[x] == x ? x : parent[x] = self(self, parent[x]);
        };

        auto unite = [&](NodeIndex x, NodeIndex y) {
            NodeIndex px = find(find, x), py = find(find, y);
            if (px != py) parent[px] = py;
        };

        // Collect all edges
        FixedVector<std::tuple<WeightType, NodeIndex, NodeIndex>, MAX_NODES * MAX_NODES> edges{};
        for (NodeIndex u = 0; u < next_index_; ++u) {
            for (const auto& edge : neighbors(u)) {
                NodeIndex v = get_neighbor(edge);
                WeightType weight = get_weight(edge);
                if (u < v) { // Avoid duplicates in undirected graph
                    edges.push_back({weight, u, v});
                }
            }
        }

        // Sort edges by weight
        std::sort(edges.begin(), edges.end());

        FixedVector<std::pair<NodeIndex, NodeIndex>, MAX_NODES * MAX_NODES> mst{};
        for (const auto& [weight, u, v] : edges) {
            if (find(find, u) != find(find, v)) {
                unite(u, v);
                mst.push_back({u, v});
            }
        }

        return mst;
    }

    // Strongly connected components using Kosaraju's algorithm
    constexpr FixedVector<FixedVector<NodeIndex, MAX_NODES>, MAX_NODES> strongly_connected_components() const
        requires(DIRECTED)
    {
        FixedVector<bool, MAX_NODES> visited{};
        visited.resize(MAX_NODES, false);
        FixedVector<NodeIndex, MAX_NODES> order{};

        // First DFS to get finishing times
        auto dfs1 = [&](auto& self, NodeIndex node) -> void {
            visited[node] = true;
            for (const auto& edge : neighbors(node)) {
                NodeIndex neighbor = get_neighbor(edge);
                if (!visited[neighbor]) {
                    self(self, neighbor);
                }
            }
            order.push_back(node);
        };

        for (NodeIndex i = 0; i < next_index_; ++i) {
            if (!visited[i]) {
                dfs1(dfs1, i);
            }
        }

        // Create transpose graph
        FixedGraph transpose = create_transpose();

        // Reset visited
        visited = FixedVector<bool, MAX_NODES>{};
        visited.resize(MAX_NODES, false);

        FixedVector<FixedVector<NodeIndex, MAX_NODES>, MAX_NODES> sccs{};

        // Second DFS on transpose graph in decreasing finish time order
        auto dfs2 = [&](auto& self, NodeIndex node, FixedVector<NodeIndex, MAX_NODES>& component) -> void {
            visited[node] = true;
            component.push_back(node);
            for (const auto& edge : transpose.neighbors(node)) {
                NodeIndex neighbor = get_neighbor(edge);
                if (!visited[neighbor]) {
                    self(self, neighbor, component);
                }
            }
        };

        while (!order.empty()) {
            NodeIndex node = order.back();
            order.pop_back();
            if (!visited[node]) {
                FixedVector<NodeIndex, MAX_NODES> component{};
                dfs2(dfs2, node, component);
                sccs.push_back(component);
            }
        }

        return sccs;
    }

    // Create transpose graph (reverse all edges)
    constexpr FixedGraph create_transpose() const
        requires(DIRECTED)
    {
        FixedGraph transpose{};
        for (NodeIndex i = 0; i < next_index_; ++i) {
            transpose.add_node(index_to_node_[i]);
        }

        for (NodeIndex u = 0; u < next_index_; ++u) {
            for (const auto& edge : neighbors(u)) {
                NodeIndex v = get_neighbor(edge);
                if constexpr (std::is_void_v<EdgeType>) {
                    transpose.add_edge(v, u);
                } else {
                    transpose.add_edge(v, u, get_weight(edge));
                }
            }
        }

        return transpose;
    }

    // Check if graph is bipartite
    constexpr bool is_bipartite() const
    {
        FixedVector<int, MAX_NODES> colors{};
        colors.resize(MAX_NODES, -1); // -1: uncolored, 0: color 0, 1: color 1

        for (NodeIndex start = 0; start < next_index_; ++start) {
            if (colors[start] == -1) {
                FixedVector<NodeIndex, MAX_NODES> queue{}; queue.push_back(start);
                colors[start] = 0;
                std::size_t head = 0;
                while (head < queue.size()) {
                    NodeIndex u = queue[head++];

                    for (const auto& edge : neighbors(u)) {
                        NodeIndex v = get_neighbor(edge);
                        if (colors[v] == -1) {
                            colors[v] = 1 - colors[u];
                            queue.push_back(v);
                        } else if (colors[v] == colors[u]) {
                            return false; // Same color as parent
                        }
                    }
                }
            }
        }
        return true;
    }

    // Graph coloring using greedy algorithm
    constexpr FixedVector<int, MAX_NODES> greedy_coloring() const
    {
        FixedVector<int, MAX_NODES> colors{};
        colors.resize(MAX_NODES, -1);

        for (NodeIndex u = 0; u < next_index_; ++u) {
            FixedVector<bool, MAX_NODES> used_colors{};
            used_colors.resize(MAX_NODES, false);

            // Check colors of neighbors
            for (const auto& edge : neighbors(u)) {
                NodeIndex v = get_neighbor(edge);
                if (colors[v] != -1) {
                    used_colors[colors[v]] = true;
                }
            }

            // Find first unused color
            int color = 0;
            while (color < static_cast<int>(MAX_NODES) && used_colors[color]) {
                ++color;
            }
            colors[u] = color;
        }

        return colors;
    }

    // Degree centrality
    constexpr FixedVector<std::size_t, MAX_NODES> degree_centrality() const
    {
        FixedVector<std::size_t, MAX_NODES> centrality{};
        centrality.resize(MAX_NODES, 0);

        for (NodeIndex u = 0; u < next_index_; ++u) {
            if constexpr (USE_MATRIX) {
                std::size_t deg = 0;
                for (NodeIndex v = 0; v < next_index_; ++v) {
                    if constexpr (std::is_void_v<EdgeType>) {
                        if (adjacency_storage_[u][v]) ++deg;
                    } else {
                        if (adjacency_storage_[u][v].has_value()) ++deg;
                    }
                }
                centrality[u] = deg;
            } else if constexpr (USE_POOL) {
                centrality[u] = node_ranges_[u].second - node_ranges_[u].first;
            } else {
                centrality[u] = adjacency_storage_[u].size();
            }
            if constexpr (!DIRECTED) {
                // For undirected graphs, degree is already correct
            } else {
                // For directed graphs, we might want in-degree or out-degree
                // Here we use out-degree
            }
        }

        return centrality;
    }

    // Betweenness centrality (approximate for large graphs)
    constexpr FixedVector<double, MAX_NODES> betweenness_centrality() const
    {
        FixedVector<double, MAX_NODES> centrality{};
        centrality.resize(MAX_NODES, 0.0);

        for (NodeIndex s = 0; s < next_index_; ++s) {
            FixedVector<double, MAX_NODES> sigma{};
            sigma.resize(MAX_NODES, 0.0);
            sigma[s] = 1.0;

            FixedVector<double, MAX_NODES> distance{};
            distance.resize(MAX_NODES, -1.0);
            distance[s] = 0.0;

            FixedVector<FixedVector<NodeIndex, MAX_NODES>, MAX_NODES> predecessors{};
            predecessors.resize(MAX_NODES);

            FixedVector<NodeIndex, MAX_NODES> queue{}; queue.push_back(s);
            std::size_t head = 0;
            while (head < queue.size()) {
                NodeIndex v = queue[head++];

                for (const auto& edge : neighbors(v)) {
                    NodeIndex w = get_neighbor(edge);
                    if (distance[w] == -1.0) {
                        queue.push_back(w);
                        distance[w] = distance[v] + 1.0;
                    }
                    if (distance[w] == distance[v] + 1.0) {
                        sigma[w] += sigma[v];
                        predecessors[w].push_back(v);
                    }
                }
            }

            FixedVector<double, MAX_NODES> delta{};
            delta.resize(MAX_NODES, 0.0);

            while (!queue.empty()) queue.pop_back(); // Clear queue
            for (NodeIndex v = 0; v < next_index_; ++v) {
                if (distance[v] != -1.0) queue.push_back(v);
            }

            while (!queue.empty()) {
                NodeIndex w = queue.back();
                queue.pop_back();

                for (NodeIndex v : predecessors[w]) {
                    delta[v] += (sigma[v] / sigma[w]) * (1.0 + delta[w]);
                }

                if (w != s) {
                    centrality[w] += delta[w];
                }
            }
        }

        return centrality;
    }

    // Check if graph has Eulerian circuit
    constexpr bool has_eulerian_circuit() const
    {
        if constexpr (DIRECTED)
        {
            // For directed graphs: strongly connected (when ignoring zero in/out nodes) & in-degree==out-degree per node
            // Quick check: every node must have equal in/out degree and be reachable in undirected sense
            // Build in-degrees
            FixedVector<std::size_t, MAX_NODES> in_degree{}; in_degree.resize(MAX_NODES, 0);
            FixedVector<bool, MAX_NODES> has_any_edge{}; has_any_edge.resize(MAX_NODES, false);
            for (NodeIndex u = 0; u < next_index_; ++u)
            {
                if (get_degree(u) > 0) has_any_edge[u] = true;
                for (const auto& edge : neighbors(u))
                {
                    NodeIndex v = get_neighbor(edge);
                    in_degree[v]++;
                    has_any_edge[v] = true;
                }
            }
            for (NodeIndex u = 0; u < next_index_; ++u)
            {
                if (has_any_edge[u])
                {
                    if (in_degree[u] != get_degree(u)) return false;
                }
            }
            // Weak connectivity check: perform DFS treating edges as undirected on nodes that participate
            // Find first node with an edge
            NodeIndex start = INVALID_INDEX;
            for (NodeIndex u = 0; u < next_index_; ++u) if (has_any_edge[u]) { start = u; break; }
            if (start == INVALID_INDEX) return true; // Trivial (all isolated)
            FixedVector<bool, MAX_NODES> visited{}; visited.resize(MAX_NODES, false);
            FixedVector<NodeIndex, MAX_NODES> stack{}; stack.push_back(start); visited[start] = true;
            while (!stack.empty())
            {
                NodeIndex cur = stack.back(); stack.pop_back();
                // Out edges
                for (const auto& edge : neighbors(cur)) { NodeIndex v = get_neighbor(edge); if (!visited[v]) { visited[v] = true; stack.push_back(v);} }
                // In edges (scan all) - O(VE) worst case but bounded by template sizes
                for (NodeIndex v = 0; v < next_index_; ++v)
                {
                    for (const auto& e2 : neighbors(v)) { if (get_neighbor(e2) == cur && !visited[v]) { visited[v] = true; stack.push_back(v); } }
                }
            }
            for (NodeIndex u = 0; u < next_index_; ++u) if (has_any_edge[u] && !visited[u]) return false;
            return true;
        }
        else
        {
            if (!is_connected()) return false;
            for (NodeIndex u = 0; u < next_index_; ++u) {
                if (get_degree(u) % 2 != 0) return false;
            }
            return true;
        }
    }

    // Graph density
    constexpr double density() const
    {
        std::size_t max_possible_edges = DIRECTED ? next_index_ * (next_index_ - 1) : next_index_ * (next_index_ - 1) / 2;
        if (max_possible_edges == 0) return 0.0;

        std::size_t actual_edges = 0;
        if constexpr (USE_MATRIX) {
            for (NodeIndex u = 0; u < next_index_; ++u) {
                for (NodeIndex v = 0; v < next_index_; ++v) {
                    if constexpr (std::is_void_v<EdgeType>) {
                        if (adjacency_storage_[u][v]) {
                            if constexpr (DIRECTED) ++actual_edges; else if (u < v) ++actual_edges;
                        }
                    } else {
                        if (adjacency_storage_[u][v].has_value()) {
                            if constexpr (DIRECTED) ++actual_edges; else if (u < v) ++actual_edges;
                        }
                    }
                }
            }
        } else {
            for (NodeIndex u = 0; u < next_index_; ++u) {
                for (const auto& edge : neighbors(u)) {
                    NodeIndex v = get_neighbor(edge);
                    if constexpr (DIRECTED) {
                        ++actual_edges;
                    } else {
                        if (u < v) ++actual_edges; // Count each undirected edge once
                    }
                }
            }
        }

        return static_cast<double>(actual_edges) / max_possible_edges;
    }

    // Graph diameter (longest shortest path)
    constexpr std::size_t diameter() const
    {
        std::size_t max_distance = 0;
        for (NodeIndex start = 0; start < next_index_; ++start) {
            FixedVector<std::size_t, MAX_NODES> distances = bfs_distances(start);
            for (std::size_t dist : distances) {
                if (dist != std::numeric_limits<std::size_t>::max() && dist > max_distance) {
                    max_distance = dist;
                }
            }
        }
        return max_distance;
    }

    // BFS distances from a source node
    constexpr FixedVector<std::size_t, MAX_NODES> bfs_distances(NodeIndex start) const
    {
        FixedVector<std::size_t, MAX_NODES> distances{};
        distances.resize(MAX_NODES, std::numeric_limits<std::size_t>::max());
        FixedVector<bool, MAX_NODES> visited{};
        visited.resize(MAX_NODES, false);
        FixedVector<NodeIndex, MAX_NODES> queue{}; queue.push_back(start); distances[start] = 0; visited[start] = true;
        std::size_t head = 0;
        while (head < queue.size()) {
            NodeIndex current = queue[head++];

            for (const auto& edge : neighbors(current)) {
                NodeIndex neighbor = get_neighbor(edge);
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    distances[neighbor] = distances[current] + 1;
                    queue.push_back(neighbor);
                }
            }
        }

        return distances;
    }

    // Clustering coefficient for undirected graphs
    constexpr double clustering_coefficient(NodeIndex node) const
        requires(!DIRECTED)
    {
        if (node >= next_index_) return 0.0;

        const auto& neighbors_list = neighbors(node);
        std::size_t degree = neighbors_list.size();
        if (degree < 2) return 0.0;

        std::size_t triangles = 0;
        for (std::size_t i = 0; i < neighbors_list.size(); ++i) {
            NodeIndex u = get_neighbor(neighbors_list[i]);
            for (std::size_t j = i + 1; j < neighbors_list.size(); ++j) {
                NodeIndex v = get_neighbor(neighbors_list[j]);
                if (has_edge(u, v)) ++triangles;
            }
        }

        return 2.0 * triangles / (degree * (degree - 1));
    }

    // Average clustering coefficient
    constexpr double average_clustering_coefficient() const
        requires(!DIRECTED)
    {
        double sum = 0.0;
        std::size_t count = 0;
        for (NodeIndex i = 0; i < next_index_; ++i) {
            if (get_degree(i) >= 2) {
                sum += clustering_coefficient(i);
                ++count;
            }
        }
        return count > 0 ? sum / count : 0.0;
    }

    // Graph complement
    constexpr FixedGraph complement() const
    {
        FixedGraph comp{};
        for (NodeIndex i = 0; i < next_index_; ++i) {
            comp.add_node(index_to_node_[i]);
        }

        for (NodeIndex u = 0; u < next_index_; ++u) {
            for (NodeIndex v = 0; v < next_index_; ++v) {
                if (u != v && !has_edge(u, v)) {
                    if constexpr (std::is_void_v<EdgeType>) {
                        comp.add_edge(u, v);
                        if constexpr (!DIRECTED) comp.add_edge(v, u);
                    } else {
                        comp.add_edge(u, v, EdgeType{});
                        if constexpr (!DIRECTED) comp.add_edge(v, u, EdgeType{});
                    }
                }
            }
        }

        return comp;
    }

    // Graph union
    constexpr FixedGraph graph_union(const FixedGraph& other) const
    {
        FixedGraph result = *this;

        // Add nodes from other graph
        for (NodeIndex i = 0; i < other.next_index_; ++i) {
            result.add_node(other.index_to_node_[i]);
        }

        // Add edges from other graph
        for (NodeIndex u = 0; u < other.next_index_; ++u) {
            for (const auto& edge : other.neighbors(u)) {
                NodeIndex v = other.get_neighbor(edge);
                if constexpr (std::is_void_v<EdgeType>) {
                    result.add_edge(u, v);
                } else {
                    result.add_edge(u, v, other.get_weight(edge));
                }
            }
        }

        return result;
    }

    // Graph intersection
    constexpr FixedGraph graph_intersection(const FixedGraph& other) const
    {
        FixedGraph result{};

        // Add common nodes
        for (NodeIndex i = 0; i < next_index_; ++i) {
            for (NodeIndex j = 0; j < other.next_index_; ++j) {
                if (index_to_node_[i] == other.index_to_node_[j]) {
                    result.add_node(index_to_node_[i]);
                    break;
                }
            }
        }

        // Add common edges
        for (NodeIndex u = 0; u < result.next_index_; ++u) {
            for (NodeIndex v = 0; v < result.next_index_; ++v) {
                if (has_edge(u, v) && other.has_edge(u, v)) {
                    if constexpr (std::is_void_v<EdgeType>) {
                        result.add_edge(u, v);
                    } else {
                        // Find the weight from this graph
                        for (const auto& edge : neighbors(u)) {
                            if (get_neighbor(edge) == v) {
                                result.add_edge(u, v, get_weight(edge));
                                break;
                            }
                        }
                    }
                }
            }
        }

        return result;
    }

    // Serialize graph to binary format
    template <typename OutputIterator>
    constexpr OutputIterator serialize(OutputIterator out) const
    {
        static_assert(std::is_trivially_copyable_v<NodeType>, "NodeType must be trivially copyable for serialize");
        if constexpr (!std::is_void_v<EdgeType>) { static_assert(std::is_trivially_copyable_v<EdgeType>, "EdgeType must be trivially copyable for serialize"); }
        // Version byte (simple) then node count
        *out++ = static_cast<std::byte>(1); // version
        *out++ = static_cast<std::byte>(next_index_);
        for (NodeIndex i = 0; i < next_index_; ++i) {
            const NodeType& node = index_to_node_[i];
            std::memcpy(&*out, &node, sizeof(NodeType));
            out += sizeof(NodeType);
        }
        for (NodeIndex u = 0; u < next_index_; ++u) {
            const auto& list = neighbors(u);
            *out++ = static_cast<std::byte>(list.size());
            for (const auto& edge : list) {
                if constexpr (std::is_void_v<EdgeType>) {
                    NodeIndex v = get_neighbor(edge);
                    std::memcpy(&*out, &v, sizeof(NodeIndex));
                    out += sizeof(NodeIndex);
                } else {
                    std::pair<NodeIndex, EdgeType> pod{get_neighbor(edge), get_weight(edge)};
                    std::memcpy(&*out, &pod, sizeof(pod));
                    out += sizeof(pod);
                }
            }
        }
        return out;
    }

    // Deserialize graph from binary format
    template <typename InputIterator>
    constexpr InputIterator deserialize(InputIterator in)
    {
        static_assert(std::is_trivially_copyable_v<NodeType>, "NodeType must be trivially copyable for deserialize");
        if constexpr (!std::is_void_v<EdgeType>) { static_assert(std::is_trivially_copyable_v<EdgeType>, "EdgeType must be trivially copyable for deserialize"); }
        // Version
        (void)*in++; // ignore version for now
        NodeIndex node_count = static_cast<NodeIndex>(*in++);
        for (NodeIndex i = 0; i < node_count; ++i) {
            NodeType node; std::memcpy(&node, &*in, sizeof(NodeType)); in += sizeof(NodeType); add_node(node); }
        for (NodeIndex u = 0; u < node_count; ++u) {
            std::size_t edge_count = static_cast<std::size_t>(*in++);
            for (std::size_t j = 0; j < edge_count; ++j) {
                if constexpr (std::is_void_v<EdgeType>) {
                    NodeIndex v; std::memcpy(&v, &*in, sizeof(NodeIndex)); in += sizeof(NodeIndex); add_edge(u, v);
                } else {
                    std::pair<NodeIndex, EdgeType> pod; std::memcpy(&pod, &*in, sizeof(pod)); in += sizeof(pod); add_edge(u, pod.first, pod.second);
                }
            }
        }
        return in;
    }

    // Check if graph has cycles
    constexpr bool has_cycles() const
    {
        FixedVector<bool, MAX_NODES> visited{};
        visited.resize(MAX_NODES, false);
        FixedVector<bool, MAX_NODES> rec_stack{};
        rec_stack.resize(MAX_NODES, false);

        for (NodeIndex i = 0; i < next_index_; ++i)
        {
            if (!visited[i] && has_cycles_helper(i, visited, rec_stack))
                return true;
        }
        return false;
    }

private:
    NodeIndex get_neighbor(const EdgeStorage& edge) const
    {
        if constexpr (std::is_void_v<EdgeType>)
        {
            return edge;
        }
        else
        {
            return edge.first;
        }
    }

    auto get_weight(const EdgeStorage& edge) const
    {
        if constexpr (std::is_void_v<EdgeType>)
        {
            return 1; // Default weight for unweighted graphs
        }
        else
        {
            return edge.second;
        }
    }

    constexpr std::size_t get_degree(NodeIndex u) const
    {
        if constexpr (USE_MATRIX) {
            std::size_t deg = 0;
            for (NodeIndex v = 0; v < next_index_; ++v) {
                if constexpr (std::is_void_v<EdgeType>) {
                    if (adjacency_storage_[u][v]) ++deg;
                } else {
                    if (adjacency_storage_[u][v].has_value()) ++deg;
                }
            }
            return deg;
        } else if constexpr (USE_POOL) {
            return node_ranges_[u].second - node_ranges_[u].first;
        } else {
            return adjacency_storage_[u].size();
        }
    }

    constexpr bool has_cycles_helper(NodeIndex node,
                                     FixedVector<bool, MAX_NODES>& visited,
                                     FixedVector<bool, MAX_NODES>& rec_stack) const
    {
        visited[node] = true;
        rec_stack[node] = true;

        for (const auto& edge : neighbors(node))
        {
            NodeIndex neighbor = get_neighbor(edge);
            if (!visited[neighbor] && has_cycles_helper(neighbor, visited, rec_stack))
                return true;
            else if (rec_stack[neighbor])
                return true;
        }

        rec_stack[node] = false;
        return false;
    }
};

}  // namespace fixed_containers