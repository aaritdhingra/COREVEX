#pragma once
// PURPOSE: Custom hash table: separate chaining, power-of-two-free
// bucket growth, load-factor-triggered rehash.
// Complexity: average O(1) insert/find/remove; worst O(n) on a bad hash.
#include <functional>
#include <list>
#include <utility>
#include <vector>
namespace corevex {
template <typename K, typename V>
class HashTable {
 public:
  explicit HashTable(size_t buckets = 16) : buckets_(buckets) {}
  void insert(const K& key, const V& value) {
    if ((size_ + 1) * 4 > buckets_.size() * 3) rehash(buckets_.size() * 2);
    auto& chain = buckets_[bucket(key)];
    for (auto& kv : chain) if (kv.first == key) { kv.second = value; return; }
    chain.emplace_back(key, value); ++size_;
  }
  V* find(const K& key) {
    if (buckets_.empty()) return nullptr;
    for (auto& kv : buckets_[bucket(key)]) if (kv.first == key) return &kv.second;
    return nullptr;
  }
  const V* find(const K& key) const {
    if (buckets_.empty()) return nullptr;
    for (const auto& kv : buckets_[bucket(key)]) if (kv.first == key) return &kv.second;
    return nullptr;
  }
  bool contains(const K& key) const { return find(key) != nullptr; }
  bool remove(const K& key) {
    auto& chain = buckets_[bucket(key)];
    for (auto it = chain.begin(); it != chain.end(); ++it)
      if (it->first == key) { chain.erase(it); --size_; return true; }
    return false;
  }
  size_t size() const { return size_; }
  size_t bucketCount() const { return buckets_.size(); }
  double loadFactor() const { return buckets_.empty() ? 0.0 : static_cast<double>(size_) / buckets_.size(); }
  size_t collisionCount() const { // buckets holding >1 entry
    size_t n = 0; for (const auto& c : buckets_) if (c.size() > 1) n += c.size() - 1; return n;
  }
 private:
  size_t bucket(const K& key) const { return std::hash<K>{}(key) % buckets_.size(); }
  void rehash(size_t n) {
    std::vector<std::list<std::pair<K, V>>> old = std::move(buckets_);
    buckets_.assign(n, {}); size_ = 0;
    for (auto& chain : old) for (auto& kv : chain) insert(kv.first, kv.second);
  }
  std::vector<std::list<std::pair<K, V>>> buckets_;
  size_t size_ = 0;
};
}  // namespace corevex
