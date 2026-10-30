// SE618 Lecture 5 — tests for the inverted index (contract + differential vs LinearScan).
// g++ -std=c++20 -O1 -g -fsanitize=address,undefined index_test.cpp && ./a.out
// Instructor: add -DSOLUTION -I../../../Instructor_Only/Lecture_05 to test the solution.
#ifdef SOLUTION
#include "inverted_index_solution.hpp"
#else
#include "inverted_index.hpp"
#endif
#include <cstdio>
#include <random>

using namespace se618;
static int fails = 0;
#define CHECK(c) do { if (!(c)) { ++fails; std::printf("FAIL line %d: %s\n", __LINE__, #c); } } while (0)
using V = std::vector<DocId>;

int main() {
    CHECK((analyze("The Cache-Stampede, in 2026!") == std::vector<std::string>{"cache", "stampede", "2026"}));

    InvertedIndex ix;
    CHECK(ix.add_document(1, "Redis cache eviction policy"));
    CHECK(ix.add_document(2, "LRU cache with TTL expiry"));
    CHECK(ix.add_document(3, "Inverted index for search"));
    CHECK(!ix.add_document(2, "duplicate id is rejected"));
    CHECK(ix.search_or("cache index") == (V{1, 2, 3}));
    CHECK(ix.search_and("cache") == (V{1, 2}));                    // single term
    CHECK(ix.search_and("CACHE lru") == (V{2}));                   // TODO 1: intersection
    CHECK(ix.search_and("cache missingterm").empty());
    CHECK(ix.search_and_not("cache", "ttl") == (V{1}));
    CHECK(ix.document_frequency("cache") == 2);

    CHECK(ix.remove_document(2));                                  // TODO 2: removal
    CHECK(!ix.remove_document(2));
    CHECK(ix.search_and("cache") == (V{1}));
    CHECK(ix.document_frequency("lru") == 0);
    CHECK(ix.check_invariants());
    CHECK(ix.update_document(1, "bloom filter"));
    CHECK(ix.search_and("redis").empty() && ix.search_and("bloom") == (V{1}));
    CHECK(ix.check_invariants());

    // Differential: random docs over a tiny vocabulary, random add/remove, compare AND results.
    const char* words[] = {"alpha", "beta", "gamma", "delta", "eps", "zeta"};
    std::mt19937 g(5);
    for (int trial = 0; trial < 50 && !fails; ++trial) {
        InvertedIndex a; LinearScan ref; std::vector<std::pair<DocId, std::string>> live;
        for (DocId id = 0; id < 60; ++id) {
            std::string t; for (int k = 0; k < 4; ++k) { t += words[g() % 6]; t += ' '; }
            DocId d = (id * 37) % 60;                            // non-monotonic ids exercise sorted insertion
            a.add_document(d, t); live.push_back({d, t});
        }
        for (int k = 0; k < 15; ++k) { auto i = g() % live.size(); a.remove_document(live[i].first); live.erase(live.begin() + i); }
        for (auto& [d, t] : live) ref.add_document(d, t);
        for (int q = 0; q < 30; ++q) {
            std::string query = std::string(words[g() % 6]) + " " + words[g() % 6];
            if (a.search_and(query) != ref.search_and(query)) { ++fails; std::printf("FAIL differential trial=%d query='%s'\n", trial, query.c_str()); break; }
        }
        CHECK(a.check_invariants());
    }
    std::printf(fails ? "%d failure(s)\n" : "all tests passed\n", fails);
    return fails != 0;
}
