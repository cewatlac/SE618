// SE618 Lecture 9 — contract tests for the three versioned stores.
// g++ -std=c++20 -O1 -g versioned_test.cpp && ./a.out
// Instructor: add -DSOLUTION -I../../../Instructor_Only/Lecture_09 to test the solution.
#ifdef SOLUTION
#include "versioned_store_solution.hpp"
#else
#include "versioned_store.hpp"
#endif
#include <cstdio>
#include <exception>
#include <random>
#include <string>

using namespace se618;
static int fails = 0;
#define CHECK(c, ...) do { if (!(c)) { ++fails; std::printf("FAIL %s line %d: %s\n", name, __LINE__, #c); } } while (0)

// Random branching history; every version is checked against an independent oracle (full
// copies kept by the test) AFTER the whole history has been built: snapshots must not change.
template <class Store> static void history_is_immutable(const char* name, Store store) {
    const std::size_t N = store.size();
    std::mt19937 g(9);
    std::vector<std::vector<Value>> oracle{std::vector<Value>(N, 0)};
    std::vector<Version> ids{0};
    for (int step = 0; step < 300; ++step) {
        std::size_t p = (g() % 4 == 0) ? g() % ids.size() : ids.size() - 1;   // mostly linear, some branching
        std::vector<Change> ch;
        int k = 1 + static_cast<int>(g() % 5);
        for (int j = 0; j < k; ++j) ch.push_back({static_cast<std::uint32_t>(g() % N), static_cast<Value>(g() % 1000)});
        auto next = oracle[p];
        for (auto& [i, x] : ch) next[i] = x;                                  // later pairs win
        ids.push_back(store.commit(ids[p], ch));
        oracle.push_back(next);
    }
    int bad = 0;
    for (std::size_t v = 0; v < ids.size() && !bad; ++v)
        for (std::size_t i = 0; i < N; ++i)
            if (store.get(ids[v], i) != oracle[v][i]) {
                std::printf("FAIL %s: version %u slot %zu is %lld, expected %lld (history changed)\n",
                            name, ids[v], i, (long long)store.get(ids[v], i), (long long)oracle[v][i]);
                ++bad; break;
            }
    fails += bad;
}

template <class Store> static void release_semantics(const char* name, Store store) {
    Version a = store.commit(0, {{1, 10}});
    Version b = store.commit(a, {{2, 20}});
    CHECK(store.release(b));
    bool threw = false;
    try { (void)store.get(b, 0); } catch (const std::out_of_range&) { threw = true; }
    CHECK(threw);
    CHECK(store.get(a, 1) == 10 && store.get(a, 2) == 0);
}

int main() {
    const std::size_t N = 1000;   // not a power of 32: exercises partial radix-tree coverage
    auto guard = [](const char* n, auto f) { try { f(); } catch (const std::exception& e) { ++fails; std::printf("FAIL %s: exception %s\n", n, e.what()); } };
    guard("full_copy", [&] { history_is_immutable("full_copy", FullCopyStore(N)); });
    guard("delta_chain", [&] { history_is_immutable("delta_chain", DeltaChainStore(N, 0)); });
    guard("delta_chain_ckpt", [&] { history_is_immutable("delta_chain_ckpt", DeltaChainStore(N, 16)); });
    guard("path_copy", [&] { history_is_immutable("path_copy", PathCopyStore(N)); });
    guard("full_copy", [&] { release_semantics("full_copy", FullCopyStore(N)); });
    guard("path_copy", [&] { release_semantics("path_copy", PathCopyStore(N)); });
    guard("delta_chain", [&] {
        const char* name = "delta_chain";
        DeltaChainStore d(N, 0);
        Version a = d.commit(0, {{1, 10}});
        Version b = d.commit(a, {{2, 20}});
        CHECK(!d.release(a));                         // a still has a live child: cannot be dropped
        CHECK(d.retire(a));                           // ...but it can be marked as no longer needed
        Version c = d.commit(b, {{3, 30}});
        CHECK(d.compact(c) == 0);                     // c becomes a full base; a is still needed by b
        CHECK(d.get(c, 1) == 10 && d.get(c, 2) == 20 && d.get(c, 3) == 30);
        CHECK(d.get(b, 2) == 20);                     // b was never retired: still readable
        CHECK(d.release(b));                          // b is now a leaf; releasing it frees retired a too
        bool gone = false;
        try { (void)d.get(a, 1); } catch (const std::out_of_range&) { gone = true; }
        CHECK(gone && d.get(c, 1) == 10);
    });
    std::printf(fails ? "%d failure(s)\n" : "all tests passed\n", fails);
    return fails != 0;
}
