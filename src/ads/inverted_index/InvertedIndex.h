#pragma once
// PURPOSE: Custom inverted index. Term dictionary is the custom
// HashTable (not std::unordered_map for the index itself).
// Posting = (docId, term frequency, positions) for phrase search.
// Lookup O(1) average for the term + work proportional to its posting
// list; indexing O(tokens) per document.
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>
#include "../hash_table/HashTable.h"
#include "../../utilities/TextUtils.h"
namespace corevex {
struct Posting { int docId = 0; int freq = 0; std::vector<int> positions; };
class InvertedIndex {
 public:
  void addDocument(int docId, const std::string& text) {
    removeDocument(docId);
    auto tokens = tokenize(text);
    docs_[docId] = {text, static_cast<int>(tokens.size())};
    for (int pos = 0; pos < static_cast<int>(tokens.size()); ++pos) {
      const std::string& term = tokens[pos];
      if (!index_.contains(term)) index_.insert(term, {});
      auto* list = index_.find(term);
      Posting* p = nullptr;
      for (auto& q : *list) if (q.docId == docId) p = &q;
      if (!p) { list->push_back({docId, 0, {}}); p = &list->back(); }
      ++p->freq; p->positions.push_back(pos);
    }
  }
  void removeDocument(int docId) {
    auto it = docs_.find(docId); if (it == docs_.end()) return;
    for (const auto& term : tokenize(it->second.text)) {
      auto* list = index_.find(term); if (!list) continue;
      list->erase(std::remove_if(list->begin(), list->end(), [&](const Posting& p){ return p.docId == docId; }), list->end());
      if (list->empty()) index_.remove(term);
    }
    docs_.erase(it);
  }
  std::vector<int> searchAnd(const std::string& query) const {
    auto terms = tokenize(query); if (terms.empty()) return {};
    std::vector<int> out;
    const auto* first = index_.find(terms[0]); if (!first) return {};
    for (const auto& p : *first) out.push_back(p.docId);
    for (size_t i = 1; i < terms.size(); ++i) {
      const auto* list = index_.find(terms[i]); if (!list) return {};
      std::vector<int> ids; for (const auto& p : *list) ids.push_back(p.docId);
      std::vector<int> next; std::sort(out.begin(), out.end()); std::sort(ids.begin(), ids.end());
      std::set_intersection(out.begin(), out.end(), ids.begin(), ids.end(), std::back_inserter(next)); out = next;
      if (out.empty()) return out;
    }
    return out;
  }
  std::vector<int> phraseSearch(const std::string& phrase) const {
    auto terms = tokenize(phrase); if (terms.empty()) return {};
    auto candidates = searchAnd(phrase); std::vector<int> out;
    for (int doc : candidates) {
      const auto* first = index_.find(terms[0]); if (!first) continue;
      for (const auto& p : *first) if (p.docId == doc) {
        for (int pos : p.positions) {
          bool ok = true;
          for (size_t i = 1; i < terms.size(); ++i) {
            const auto* list = index_.find(terms[i]); bool found = false;
            if (list) for (const auto& q : *list) if (q.docId == doc) for (int pp : q.positions) if (pp == pos + static_cast<int>(i)) found = true;
            if (!found) { ok = false; break; }
          }
          if (ok) { out.push_back(doc); break; }
        }
      }
    }
    return out;
  }
  const std::vector<Posting>* postings(const std::string& term) const { return index_.find(term); }
  int docCount() const { return static_cast<int>(docs_.size()); }
  size_t termCount() const { return index_.size(); }
  int docLength(int docId) const { auto it = docs_.find(docId); return it == docs_.end() ? 0 : it->second.length; }
  double avgDocLength() const { if (docs_.empty()) return 0; double s = 0; for (const auto& kv : docs_) s += kv.second.length; return s / docs_.size(); }
 private:
  struct Doc { std::string text; int length = 0; };
  HashTable<std::string, std::vector<Posting>> index_;
  std::unordered_map<int, Doc> docs_;
};
}  // namespace corevex
