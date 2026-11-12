#pragma once
// PURPOSE: Graph of sources / claims / topics. Adjacency is built from
// the edge list on demand; BFS/DFS traverse only recorded edges.
// No semantic edge is invented: an edge exists only if code added it
// from an explicit rule or a recorded relationship.
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>
namespace corevex {
class EvidenceGraph {
 public:
  enum class NodeType { Source, Claim, Topic };
  enum class EdgeType { Supports, Contradicts, Mentions, RelatedTo };
  struct Node { int id; NodeType type; std::string label; };
  struct Edge { int from, to; EdgeType type; };
  int addNode(NodeType t, std::string label) { int id = static_cast<int>(nodes_.size()); nodes_.push_back({id, t, std::move(label)}); return id; }
  void addEdge(int from, int to, EdgeType t) { edges_.push_back({from, to, t}); }
  const std::vector<Node>& nodes() const { return nodes_; }
  const std::vector<Edge>& edges() const { return edges_; }
  std::vector<int> neighbours(int id) const {
    std::vector<int> out; for (const auto& e : edges_) if (e.from == id) out.push_back(e.to); return out;
  }
  std::vector<int> bfs(int start) const { // O(V+E)
    std::vector<int> order; std::vector<char> seen(nodes_.size(), 0); std::queue<int> q;
    if (start < 0 || start >= static_cast<int>(nodes_.size())) return order;
    seen[start] = 1; q.push(start);
    while (!q.empty()) { int u = q.front(); q.pop(); order.push_back(u); for (int v : neighbours(u)) if (!seen[v]) { seen[v] = 1; q.push(v); } }
    return order;
  }
  std::vector<int> dfs(int start) const {
    std::vector<int> order; std::vector<char> seen(nodes_.size(), 0); dfsRec(start, seen, order); return order;
  }
 private:
  void dfsRec(int u, std::vector<char>& seen, std::vector<int>& order) const {
    if (u < 0 || u >= static_cast<int>(nodes_.size()) || seen[u]) return;
    seen[u] = 1; order.push_back(u); for (int v : neighbours(u)) dfsRec(v, seen, order);
  }
  std::vector<Node> nodes_; std::vector<Edge> edges_;
};
}  // namespace corevex
