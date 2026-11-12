#pragma once
// PURPOSE: BM25 ranking over an InvertedIndex, Top-K via the custom
// BinaryHeap (min-heap eviction). k1=1.5, b=0.75 are the standard
// defaults; changing b raises/lowers the length penalty, k1 controls
// term-frequency saturation. Scores are ranking signals, not relevance
// guarantees. Complexity: O(queryTerms * postings + N log K).
#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <vector>
#include "../ads/heap/BinaryHeap.h"
#include "../ads/inverted_index/InvertedIndex.h"
#include "../utilities/TextUtils.h"
namespace corevex {
struct RankedDoc { double score; int docId; bool operator<(const RankedDoc& o) const { return score < o.score; } };
class Bm25Ranker {
 public:
  Bm25Ranker(double k1 = 1.5, double b = 0.75) : k1_(k1), b_(b) {}
  std::vector<RankedDoc> rank(const InvertedIndex& index, const std::string& query, size_t k = 10) const {
    auto terms = tokenize(query); const int n = index.docCount(); const double avg = index.avgDocLength() > 0 ? index.avgDocLength() : 1.0;
    std::vector<RankedDoc> all;
    // Score every candidate doc that matches at least one term, then Top-K.
    // (For a demo-scale index this is explicit; a production index would
    // score posting lists directly without materialising 'all'.)
    std::vector<int> candidates;
    for (const auto& t : terms) { const auto* list = index.postings(t); if (list) for (const auto& p : *list) candidates.push_back(p.docId); }
    std::sort(candidates.begin(), candidates.end()); candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());
    for (int doc : candidates) {
      double score = 0;
      for (const auto& t : terms) {
        const auto* list = index.postings(t); if (!list) continue;
        int freq = 0; for (const auto& p : *list) if (p.docId == doc) freq = p.freq;
        if (!freq) continue;
        double idf = std::log(1.0 + (n - static_cast<double>(list->size()) + 0.5) / (static_cast<double>(list->size()) + 0.5));
        double dl = index.docLength(doc);
        score += idf * (freq * (k1_ + 1)) / (freq + k1_ * (1 - b_ + b_ * dl / avg));
      }
      all.push_back({score, doc});
    }
    return BinaryHeap<RankedDoc>::topK(all, k);
  }
 private:
  double k1_, b_;
};
}  // namespace corevex
