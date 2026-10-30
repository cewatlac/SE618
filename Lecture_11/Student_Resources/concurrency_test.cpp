// SE618 Lecture 11 — stress tests: what they can and cannot tell you.
// g++ -std=c++20 -O2 -pthread concurrency_test.cpp -o ct && ./ct
// On Linux/macOS also try: g++ -std=c++20 -O1 -g -fsanitize=thread -pthread concurrency_test.cpp
//
// Invariant checked: after T threads each perform N increments of random keys, the sum of all
// counts equals T * N (no lost updates).
//   atomic API    : map.increment(k)                      — one locked operation
//   compound API  : v = map.get(k); map.put(k, v + 1)     — two locked operations, NOT atomic
// Every call is individually thread-safe, so there is no data race (and ThreadSanitizer is
// silent); the compound version still violates the invariant. Thread safety of each call does
// not make a sequence of calls atomic.
#include "concurrent_maps.hpp"

#include <cstdio>
#include <random>
#include <thread>
#include <vector>

using namespace se618;

template <class Map, class Inc> static std::uint64_t lost(int threads, std::size_t n, std::size_t keys, Inc&& inc, std::uint64_t seed) {
    Map map;
    std::vector<std::thread> pool;
    for (int t = 0; t < threads; ++t)
        pool.emplace_back([&, t] {
            std::mt19937_64 g(seed * 100 + t);
            for (std::size_t i = 0; i < n; ++i) inc(map, g() % keys);
        });
    for (auto& th : pool) th.join();
    return threads * n - map.total();
}

int main() {
    auto atomic_inc = [](auto& m, std::uint64_t k) { m.increment(k); };
    auto compound_inc = [](auto& m, std::uint64_t k) { m.put(k, m.get(k) + 1); };
    int fails = 0;
    std::printf("design,api,threads,keys,runs,runs_with_lost_updates,max_lost\n");
    for (std::size_t keys : {std::size_t{16}, std::size_t{100000}}) {
        for (int threads : {1, 4, 8}) {
            int bad_a = 0, bad_c = 0; std::uint64_t max_a = 0, max_c = 0;
            for (std::uint64_t run = 1; run <= 20; ++run) {
                auto la = lost<ShardedMap<64>>(threads, 50000, keys, atomic_inc, run);
                auto lc = lost<ShardedMap<64>>(threads, 50000, keys, compound_inc, run);
                bad_a += la != 0; bad_c += lc != 0;
                max_a = std::max(max_a, la); max_c = std::max(max_c, lc);
            }
            std::printf("sharded64,increment(),%d,%zu,20,%d,%llu\n", threads, keys, bad_a, (unsigned long long)max_a);
            std::printf("sharded64,get()+put(),%d,%zu,20,%d,%llu\n", threads, keys, bad_c, (unsigned long long)max_c);
            if (bad_a) ++fails;                                   // the atomic API must never lose updates
        }
    }
    std::printf(fails ? "FAIL: atomic API lost updates\n" : "atomic API: no lost updates observed (evidence, not proof)\n");
    return fails != 0;
}
