// ============================================================================
// SECTION — ADS TESTS (build se pehle, alag command se chalte hain)
// ----------------------------------------------------------------------------
// Kya hai:
//   Ye test koi GUI / Qt WebEngine nahi chahiye. Sirf C++ standard
//   library + src/ads/* headers chahiye, isliye ye test full browser
//   build se pehle, sirf MSVC (cl) se chal jaata hai.
// Kya check karta hai (har CHECK fail hone par program rukega, exit != 0):
//   Trie: search + prefix suggestions
//   HashTable: insert/find/remove + rehash ke baad bhi data sahi
//   BinaryHeap: Top-K descending order me
//   EvidenceGraph: BFS me source -> claim pahunche
//   AvlTree: inorder sorted ho, height log n ke aas-paas ho
//   InvertedIndex: AND search + phrase search
//   Bm25Ranker: relevant doc sabse upar aaye
// Ye test GUI ke bugs (theme, AI, tabs) ko nahi pakadta — wo sirf
// full build + app chala kar hi check honge. Isliye sequence ye hai:
//   1) test_ads.bat  (pehle, fast)
//   2) build_core.bat (baad me, full app)
// ============================================================================
#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "../../src/ads/balanced_tree/AvlTree.h"
#include "../../src/ads/graph/EvidenceGraph.h"
#include "../../src/ads/hash_table/HashTable.h"
#include "../../src/ads/heap/BinaryHeap.h"
#include "../../src/ads/inverted_index/InvertedIndex.h"
#include "../../src/ads/trie/Trie.h"
#include "../../src/ranking/Bm25Ranker.h"

#define CHECK(cond) do { \
    if (!(cond)) { std::cerr << "FAIL: " << #cond << "  (line " << __LINE__ << ")\n"; return 1; } \
    std::cout << "PASS: " << #cond << "\n"; } while (0)

int main() {
    using namespace corevex;

    // --- Trie ------------------------------------------------------------
    Trie trie;
    for (const char* w : {"search", "secure", "server", "browser"}) trie.insert(w);
    CHECK(trie.search("search"));
    CHECK(!trie.search("sea"));
    CHECK(trie.startsWith("sea"));
    // "sea" sirf "search" se match karta hai ("secure" sec-, "server" ser- hai).
    CHECK(trie.suggestions("sea", 10).size() == 1);
    const auto sugg = trie.suggestions("se", 10);
    CHECK(sugg.size() == 3);  // search, secure, server

    // --- HashTable ---------------------------------------------------------
    HashTable<std::string, int> ht;
    for (int i = 0; i < 100; ++i) ht.insert("term" + std::to_string(i), i);
    CHECK(ht.size() == 100);
    CHECK(ht.find("term42") && *ht.find("term42") == 42);
    CHECK(ht.remove("term42") && !ht.contains("term42"));

    // --- BinaryHeap Top-K --------------------------------------------------
    const auto top = BinaryHeap<std::pair<double, int>>::topK({{1.2, 1}, {4.5, 2}, {9.0, 4}}, 2);
    CHECK(top.size() == 2 && top[0].second == 4 && top[1].second == 2);

    // --- EvidenceGraph -----------------------------------------------------
    EvidenceGraph g;
    const int src = g.addNode(EvidenceGraph::NodeType::Source, "source");
    const int claim = g.addNode(EvidenceGraph::NodeType::Claim, "claim");
    g.addEdge(src, claim, EvidenceGraph::EdgeType::Supports);
    const auto bfs = g.bfs(src);
    CHECK(bfs.size() == 2 && bfs[0] == src && bfs[1] == claim);

    // --- AvlTree -----------------------------------------------------------
    AvlTree<int, std::string> avl;
    for (int k : {10, 5, 15, 3, 7, 12, 18, 1, 20}) avl.insert(k, "doc");
    const auto inorder = avl.inorder();
    CHECK(inorder.size() == 9);
    CHECK(std::is_sorted(inorder.begin(), inorder.end(),
                         [](const auto& a, const auto& b) { return a.first < b.first; }));
    CHECK(avl.height() <= 5);  // 9 nodes, balanced height <= 4-5
    CHECK(avl.rangeQuery(5, 15).size() == 5);

    // --- InvertedIndex + BM25 ----------------------------------------------
    InvertedIndex idx;
    idx.addDocument(1, "corevex browser search engine fast");
    idx.addDocument(2, "corevex search engine local index");
    idx.addDocument(3, "cooking recipes pasta tomato");
    CHECK(idx.searchAnd("corevex search").size() == 2);
    CHECK(idx.phraseSearch("search engine").size() == 2);
    CHECK(idx.phraseSearch("pasta tomato").size() == 1);
    Bm25Ranker ranker;
    const auto ranked = ranker.rank(idx, "corevex search", 2);
    CHECK(ranked.size() == 2);
    CHECK(ranked[0].score > 0);

    std::cout << "\nALL ADS TESTS PASSED\n";
    return 0;
}
