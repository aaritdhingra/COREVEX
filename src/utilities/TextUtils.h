#pragma once
// PURPOSE: Shared tokenisation for indexing/ranking. Lower-cases,
// splits on non-alphanumerics, optionally drops stop-words.
// Complexity: O(n) in the input length.
#include <cctype>
#include <string>
#include <unordered_set>
#include <vector>
namespace corevex {
inline std::vector<std::string> tokenize(const std::string& text, bool useStopWords = false) {
  static const std::unordered_set<std::string> kStop = {"the","a","an","and","or","of","to","in","is","are","on","for","with","by","at","as"};
  std::vector<std::string> out;
  std::string cur;
  auto flush = [&] { if (!cur.empty()) { if (!useStopWords || !kStop.count(cur)) out.push_back(cur); cur.clear(); } };
  for (char ch : text) {
    if (std::isalnum(static_cast<unsigned char>(ch))) cur.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
    else flush();
  }
  flush();
  return out;
}
}  // namespace corevex
