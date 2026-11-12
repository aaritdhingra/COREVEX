#pragma once
// PURPOSE: Custom Trie for autocomplete / prefix lookup.
// Insert/search O(L), prefix suggestions O(L + results) — no full-tree scan
// for the prefix itself (only the subtree under it is walked).
// Memory trade-off: one node per distinct prefix; fine for terms, not for
// storing whole documents.
#include <map>
#include <memory>
#include <string>
#include <vector>
namespace corevex {
class Trie {
  struct Node { std::map<char, std::unique_ptr<Node>> next; bool end = false; };
 public:
  void insert(const std::string& word) {
    Node* n = &root_; for (char c : word) { auto& p = n->next[c]; if (!p) p = std::make_unique<Node>(); n = p.get(); } n->end = true; ++words_;
  }
  bool search(const std::string& word) const {
    const Node* n = findNode(word); return n && n->end;
  }
  bool startsWith(const std::string& prefix) const { return findNode(prefix) != nullptr; }
  std::vector<std::string> suggestions(const std::string& prefix, size_t limit = 10) const {
    std::vector<std::string> out; const Node* n = findNode(prefix);
    if (n) collect(n, prefix, out, limit); return out;
  }
  bool remove(const std::string& word) { bool removed = false; removeRec(&root_, word, 0, removed); if (removed) --words_; return removed; }
  size_t wordCount() const { return words_; }
 private:
  const Node* findNode(const std::string& s) const {
    const Node* n = &root_; for (char c : s) { auto it = n->next.find(c); if (it == n->next.end()) return nullptr; n = it->second.get(); } return n;
  }
  static void collect(const Node* n, std::string cur, std::vector<std::string>& out, size_t limit) {
    if (out.size() >= limit) return; if (n->end) out.push_back(cur);
    for (const auto& kv : n->next) { if (out.size() >= limit) return; collect(kv.second.get(), cur + kv.first, out, limit); }
  }
  static bool removeRec(Node* n, const std::string& w, size_t i, bool& removed) {
    if (i == w.size()) { if (!n->end) return false; n->end = false; removed = true; return n->next.empty(); }
    auto it = n->next.find(w[i]); if (it == n->next.end()) return false;
    bool prune = removeRec(it->second.get(), w, i + 1, removed);
    if (prune) n->next.erase(it); return removed && n->next.empty() && !n->end;
  }
  Node root_; size_t words_ = 0;
};
}  // namespace corevex
