#pragma once
// PURPOSE: Custom binary min-heap (array-backed). Used for Top-K:
// keep K largest scores by evicting the smallest when size exceeds K.
// push/pop O(log n), top O(1), heapify O(n).
#include <algorithm>
#include <vector>
namespace corevex {
template <typename T>
class BinaryHeap {
 public:
  void push(const T& v) { data_.push_back(v); siftUp(data_.size() - 1); }
  const T& top() const { return data_.front(); }
  T pop() { T v = data_.front(); data_.front() = data_.back(); data_.pop_back(); if (!data_.empty()) siftDown(0); return v; }
  bool empty() const { return data_.empty(); }
  size_t size() const { return data_.size(); }
  void heapify(std::vector<T> v) { data_ = std::move(v); for (size_t i = data_.size() / 2; i-- > 0;) siftDown(i); }
  // Top-K largest, returned in descending order.
  static std::vector<T> topK(std::vector<T> items, size_t k) {
    BinaryHeap<T> h; for (const auto& it : items) { h.push(it); if (h.size() > k) h.pop(); }
    std::vector<T> out; while (!h.empty()) out.push_back(h.pop()); std::reverse(out.begin(), out.end()); return out;
  }
 private:
  void siftUp(size_t i) { while (i > 0) { size_t p = (i - 1) / 2; if (!(data_[i] < data_[p])) break; std::swap(data_[i], data_[p]); i = p; } }
  void siftDown(size_t i) { for (;;) { size_t l = 2 * i + 1, r = l + 1, m = i; if (l < data_.size() && data_[l] < data_[m]) m = l; if (r < data_.size() && data_[r] < data_[m]) m = r; if (m == i) break; std::swap(data_[i], data_[m]); i = m; } }
  std::vector<T> data_;
};
}  // namespace corevex
