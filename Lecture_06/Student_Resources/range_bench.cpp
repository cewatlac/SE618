// SE618 Lecture 6 — same interface, four workloads. Decide first, measure second.
// g++ -std=c++20 -O2 range_bench.cpp -o bench && ./bench        (~20 s)
// Output CSV: workload,impl,n,ops,build_ms,ns_per_op,memory_MB
#include "range_aggregate.hpp"

#include <chrono>
#include <cstdio>
#include <random>
#include <string>

using namespace se618;
using Clock = std::chrono::steady_clock;

struct Op { bool is_query; std::size_t a, b; Value v; };

static std::vector<Op> ops_for(std::size_t n, std::size_t count, double query_share, std::uint64_t seed) {
    std::mt19937_64 g(seed);
    std::bernoulli_distribution q(query_share);
    std::vector<Op> ops(count);
    for (auto& o : ops) {
        o.is_query = q(g);
        std::size_t l = g() % (n + 1), r = g() % (n + 1);
        if (l > r) std::swap(l, r);
        o.a = o.is_query ? l : g() % n; o.b = r; o.v = static_cast<Value>(g() % 1000);
    }
    return ops;
}

template <class R> static void run(const char* wl, const char* name, const std::vector<Value>& a, const std::vector<Op>& ops) {
    auto t0 = Clock::now();
    R r(a);
    auto t1 = Clock::now();
    Value sink = 0;
    for (auto& o : ops) { if (o.is_query) sink += r.query(o.a, o.b); else r.set(o.a, o.v); }
    auto t2 = Clock::now();
    std::printf("%s,%s,%zu,%zu,%.2f,%.1f,%.1f%s\n", wl, name, a.size(), ops.size(),
                std::chrono::duration<double, std::milli>(t1 - t0).count(),
                std::chrono::duration<double, std::nano>(t2 - t1).count() / ops.size(),
                r.memory_bytes() / 1e6, sink == 1 ? " " : "");
}

static void workload(const char* wl, std::size_t n, std::size_t count, double qshare) {
    std::vector<Value> a(n);
    std::mt19937_64 g(1);
    for (auto& x : a) x = static_cast<Value>(g() % 1000);
    auto ops = ops_for(n, count, qshare, 2);
    run<NaiveRange>(wl, "naive", a, ops);
    run<PrefixSumRange>(wl, "prefix_sum", a, ops);
    run<FenwickRange>(wl, "fenwick", a, ops);
    run<SegmentTreeRange<>>(wl, "segment_tree", a, ops);
}

int main() {
    std::printf("workload,impl,n,ops,build_ms,ns_per_op,memory_MB\n");
    const std::size_t N = 1u << 20;
    workload("A_read_heavy_99pct_query", N, 20000, 0.99);
    workload("B_mixed_50pct_query", N, 20000, 0.50);
    workload("C_immutable_query_only", N, 20000, 1.00);
    workload("D_small_n1000_50pct_query", 1000, 1000000, 0.50);
}
