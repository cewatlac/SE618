// SE618 Lecture 2 — benchmark FlatMap vs std::unordered_map under different workloads.
// g++ -std=c++20 -O2 hash_map_bench.cpp -o bench && ./bench            (flawed map)
// Instructor: add -DFIXED -I../../../Instructor_Only/Lecture_02 for the fixed map.
// Output CSV: workload,impl,n,ns_per_op   (median of 5 repetitions)
#ifdef FIXED
#include "fixed_hash_map.hpp"
#else
#include "flawed_hash_map.hpp"
#endif
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

using Clock = std::chrono::steady_clock;
static volatile std::uint64_t sink;

template <class F> static double median_ns(F&& body, std::size_t ops) {
    std::vector<double> t;
    for (int r = 0; r < 5; ++r) {
        auto a = Clock::now(); body(); auto b = Clock::now();
        t.push_back(std::chrono::duration<double, std::nano>(b - a).count() / ops);
    }
    std::sort(t.begin(), t.end());
    return t[2];
}

struct StdMap {
    std::unordered_map<std::uint64_t, std::uint64_t> m;
    void put(std::uint64_t k, std::uint64_t v) { m.insert_or_assign(k, v); }
    std::uint64_t get(std::uint64_t k) { auto it = m.find(k); return it == m.end() ? 0 : it->second; }
    void del(std::uint64_t k) { m.erase(k); }
};
struct Flat {
    se618::FlatMap<std::uint64_t, std::uint64_t> m;
    void put(std::uint64_t k, std::uint64_t v) { m.insert_or_assign(k, v); }
    std::uint64_t get(std::uint64_t k) { auto* p = m.find(k); return p ? *p : 0; }
    void del(std::uint64_t k) { m.erase(k); }
};

template <class M> static void run(const char* impl, std::size_t n) {
    std::mt19937_64 rng(7);
    std::vector<std::uint64_t> rnd(n), strided(n), miss(n);
    for (auto& k : rnd) k = rng();
    for (std::size_t i = 0; i < n; ++i) strided[i] = static_cast<std::uint64_t>(i) << 12;  // IDs sharing low bits
    for (auto& k : miss) k = rng() | 1ULL << 63;

    auto fill = [&](M& m, const std::vector<std::uint64_t>& ks) { for (auto k : ks) m.put(k, k); };
    std::printf("insert_random,%s,%zu,%.1f\n", impl, n, median_ns([&] { M m; fill(m, rnd); sink = sink + m.get(rnd[0]); }, n));
    { M m; fill(m, rnd);
      std::printf("lookup_hit,%s,%zu,%.1f\n", impl, n, median_ns([&] { std::uint64_t s = 0; for (auto k : rnd) s += m.get(k); sink = sink + s; }, n));
      std::printf("lookup_miss,%s,%zu,%.1f\n", impl, n, median_ns([&] { std::uint64_t s = 0; for (auto k : miss) s += m.get(k); sink = sink + s; }, n)); }
    { M m; fill(m, strided);
      std::printf("lookup_strided_keys,%s,%zu,%.1f\n", impl, n, median_ns([&] { std::uint64_t s = 0; for (auto k : strided) s += m.get(k); sink = sink + s; }, n)); }
    { M m; fill(m, rnd);     // churn: size stays n while keys are replaced (sessions, caches, dedup windows)
      for (std::size_t i = 0; i < 8 * n; ++i) { std::size_t j = i % n; m.del(rnd[j]); rnd[j] = rng(); m.put(rnd[j], i); }
      std::printf("lookup_miss_after_churn,%s,%zu,%.1f\n", impl, n, median_ns([&] { std::uint64_t s = 0; for (auto k : miss) s += m.get(k); sink = sink + s; }, n)); }
}

int main() {
    std::printf("workload,impl,n,ns_per_op\n");
    for (std::size_t n : {1u << 12, 1u << 16}) {
        run<StdMap>("std::unordered_map", n);
        run<Flat>("FlatMap", n);
    }
}
