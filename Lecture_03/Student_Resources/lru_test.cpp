// SE618 Lecture 3 — tests for buggy_lru.hpp.
// g++ -std=c++20 -O1 -g -fsanitize=address,undefined lru_test.cpp && ./a.out
// Instructor: add -DFIXED -I../../../Instructor_Only/Lecture_03 to test the solution.
#ifdef FIXED
#include "lru_cache.hpp"
#else
#include "buggy_lru.hpp"
#endif
#include <cstdio>
#include <random>
#include <vector>

using se618::LruCache;

// Reference model: obviously correct, O(n) per operation. Tracks last-use time per key.
struct RefLru {
    struct E { int k, v; long t; };
    std::size_t cap; long clock = 0; std::vector<E> e;
    std::optional<int> get(int k) {
        for (auto& x : e) if (x.k == k) { x.t = ++clock; return x.v; }
        return std::nullopt;
    }
    void put(int k, int v) {
        for (auto& x : e) if (x.k == k) { x.v = v; x.t = ++clock; return; }
        if (e.size() == cap) {
            std::size_t lru = 0;
            for (std::size_t i = 1; i < e.size(); ++i) if (e[i].t < e[lru].t) lru = i;
            e.erase(e.begin() + static_cast<long>(lru));
        }
        e.push_back({k, v, ++clock});
    }
};

static int fails = 0;
#define CHECK(c, ...) do { if (!(c)) { ++fails; std::printf("FAIL line %d: %s  ", __LINE__, #c); std::printf(__VA_ARGS__); std::printf("\n"); } } while (0)

int main() {
    {   // the example everyone writes: passes
        LruCache<int, int> c(2);
        c.put(1, 10); c.put(2, 20); c.put(3, 30);
        CHECK(!c.get(1) && c.get(2) == 20 && c.get(3) == 30, "basic eviction");
    }
    {   // recency: a read must protect a key from eviction
        LruCache<int, int> c(2);
        c.put(1, 10); c.put(2, 20); c.get(1); c.put(3, 30);
        CHECK(c.get(1).has_value(), "key 1 was read last-but-one, key 2 should have been evicted");
    }
    for (std::uint64_t seed = 1; seed <= 200 && fails < 3; ++seed) {   // differential
        std::mt19937_64 g(seed);
        std::size_t cap = 1 + g() % 4;
        LruCache<int, int> c(cap);
        RefLru r{cap, 0, {}};
        int before = fails;
        for (int step = 0; step < 300; ++step) {
            int k = static_cast<int>(g() % 6), v = static_cast<int>(g() % 100);
            if (g() % 2) { c.put(k, v); r.put(k, v); }
            else { auto a = c.get(k), b = r.get(k); CHECK(a == b, "seed=%llu step=%d get(%d)", (unsigned long long)seed, step, k); }
            CHECK(c.size() == r.e.size(), "seed=%llu step=%d size %zu vs %zu", (unsigned long long)seed, step, c.size(), r.e.size());
            if (fails > before) break;
        }
    }
    std::printf(fails ? "%d failure(s)\n" : "all tests passed\n", fails);
    return fails != 0;
}
