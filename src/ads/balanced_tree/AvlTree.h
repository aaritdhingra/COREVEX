#pragma once
// PURPOSE: Custom AVL tree for ordered metadata + range queries.
// Insert/search/remove O(log n) when balanced; rotations restore the
// invariant |height(L)-height(R)| <= 1. rotationCount is real, measured
// by the Lab/benchmarks — never a theoretical claim presented as data.
#include <algorithm>
#include <optional>
#include <utility>
#include <vector>
namespace corevex {
template <typename K, typename V>
class AvlTree {
  struct Node { K key; V value; Node *l = nullptr, *r = nullptr; int h = 1; };
 public:
  ~AvlTree() { clear(root_); }
  void insert(const K& k, const V& v) { root_ = insertRec(root_, k, v); }
  std::optional<V> search(const K& k) const { Node* n = root_; while (n) { if (k == n->key) return n->value; n = k < n->key ? n->l : n->r; } return std::nullopt; }
  void remove(const K& k) { root_ = removeRec(root_, k); }
  std::vector<std::pair<K, V>> inorder() const { std::vector<std::pair<K, V>> out; inorderRec(root_, out); return out; }
  std::vector<std::pair<K, V>> rangeQuery(const K& lo, const K& hi) const { std::vector<std::pair<K, V>> out; rangeRec(root_, lo, hi, out); return out; }
  int height() const { return h(root_); }
  size_t size() const { return size_; }
  size_t rotationCount() const { return rotations_; }
 private:
  static int h(Node* n) { return n ? n->h : 0; }
  static int bal(Node* n) { return n ? h(n->l) - h(n->r) : 0; }
  static void upd(Node* n) { n->h = 1 + std::max(h(n->l), h(n->r)); }
  Node* rotR(Node* y) { Node* x = y->l; y->l = x->r; x->r = y; upd(y); upd(x); ++rotations_; return x; }
  Node* rotL(Node* x) { Node* y = x->r; x->r = y->l; y->l = x; upd(x); upd(y); ++rotations_; return y; }
  Node* rebalance(Node* n) { upd(n); int b = bal(n); if (b > 1 && bal(n->l) >= 0) return rotR(n); if (b > 1) { n->l = rotL(n->l); return rotR(n); } if (b < -1 && bal(n->r) <= 0) return rotL(n); if (b < -1) { n->r = rotR(n->r); return rotL(n); } return n; }
  Node* insertRec(Node* n, const K& k, const V& v) { if (!n) { ++size_; return new Node{k, v}; } if (k == n->key) { n->value = v; return n; } if (k < n->key) n->l = insertRec(n->l, k, v); else n->r = insertRec(n->r, k, v); return rebalance(n); }
  Node* minNode(Node* n) { while (n->l) n = n->l; return n; }
  Node* removeRec(Node* n, const K& k) { if (!n) return nullptr; if (k < n->key) n->l = removeRec(n->l, k); else if (k > n->key) n->r = removeRec(n->r, k); else { if (!n->l || !n->r) { Node* t = n->l ? n->l : n->r; delete n; --size_; return t; } Node* m = minNode(n->r); n->key = m->key; n->value = m->value; n->r = removeRec(n->r, m->key); } return rebalance(n); }
  static void inorderRec(Node* n, std::vector<std::pair<K, V>>& out) { if (!n) return; inorderRec(n->l, out); out.emplace_back(n->key, n->value); inorderRec(n->r, out); }
  static void rangeRec(Node* n, const K& lo, const K& hi, std::vector<std::pair<K, V>>& out) { if (!n) return; if (lo < n->key) rangeRec(n->l, lo, hi, out); if (!(n->key < lo) && !(hi < n->key)) out.emplace_back(n->key, n->value); if (n->key < hi) rangeRec(n->r, lo, hi, out); }
  static void clear(Node* n) { if (!n) return; clear(n->l); clear(n->r); delete n; }
  Node* root_ = nullptr; size_t size_ = 0; size_t rotations_ = 0;
};
}  // namespace corevex
