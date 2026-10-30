// SE618 Lecture 9 — full copy vs delta chain vs path copying, under different histories.
// g++ -std=c++20 -O2 versioned_bench.cpp -o bench && ./bench        (~10 s)
// (Fix the store first, or instructor: -DSOLUTION -I../../../Instructor_Only/Lecture_09)
//
// N = 16,384 slots. Histories: A = 20 versions x 30% of slots changed; B = 1,000 versions x 1 slot.
// Metrics: commit cost per version, live heap after the history (payload bytes, counting allocator),
// read latency for random (version, slot) pairs over ALL versions ("history") and for the newest
// version only ("head"), and the cost/benefit of keeping only the last 10 versions ("trim").
// Reads: 200,000 per measurement, median of 3 repetitions. One machine; results are indicative.
#ifdef SOLUTION
#include "versioned_store_solution.hpp"
#else
#include "versioned_store.hpp"
#endif
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <random>
#include <string>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wmismatched-new-delete"
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif
static std::size_t g_live = 0;
void* operator new(std::size_t n) {
    auto* p = static_cast<std::size_t*>(std::malloc(n + 16));
    if (!p) throw std::bad_alloc();
    *p = n; g_live += n; return reinterpret_cast<char*>(p) + 16;
}
void operator delete(void* q) noexcept { if (!q) return; auto* p = reinterpret_cast<std::size_t*>(static_cast<char*>(q) - 16); g_live -= *p; std::free(p); }
void operator delete(void* q, std::size_t) noexcept { operator delete(q); }
// The nothrow forms must use the same header layout (std::stable_sort, for example, uses them).
void* operator new(std::size_t n, const std::nothrow_t&) noexcept { try { return operator new(n); } catch (...) { return nullptr; } }
void operator delete(void* q, const std::nothrow_t&) noexcept { operator delete(q); }

using namespace se618;
using Clock = std::chrono::steady_clock;
constexpr std::size_t N = 1 << 14;

static std::vector<std::vector<Change>> history(int versions, int changes_per_version) {
    std::mt19937 g(3);
    std::vector<std::vector<Change>> h(versions);
    for (auto& c : h) for (int j = 0; j < changes_per_version; ++j) c.push_back({static_cast<std::uint32_t>(g() % N), static_cast<Value>(g())});
    return h;
}

template <class Store> static double read_ns(const Store& s, Version lo, Version hi, std::size_t reads) {
    std::vector<double> reps;
    for (int r = 0; r < 3; ++r) {
        std::mt19937 g(100 + r);
        std::vector<std::pair<Version, std::size_t>> q(reads);
        for (auto& x : q) x = {static_cast<Version>(lo + g() % (hi - lo + 1)), g() % N};
        Value sink = 0;
        auto a = Clock::now();
        for (auto& [v, i] : q) sink += s.get(v, i);
        auto b = Clock::now();
        if (sink == 42) std::puts("");
        reps.push_back(std::chrono::duration<double, std::nano>(b - a).count() / reads);
    }
    std::sort(reps.begin(), reps.end());
    return reps[1];
}

template <class Store, class Trim>
static void run(const char* hist, const char* name, Store&& make_store, const std::vector<std::vector<Change>>& h, Trim&& trim) {
    std::size_t m0 = g_live;
    auto s = make_store();
    auto t0 = Clock::now();
    Version v = 0;
    for (auto& c : h) v = s.commit(v, c);
    double commit_us = std::chrono::duration<double, std::micro>(Clock::now() - t0).count() / h.size();
    double mb = (g_live - m0) / 1e6;
    double hist_ns = read_ns(s, 0, v, 200000), head_ns = read_ns(s, v, v, 200000);
    auto t1 = Clock::now();
    trim(s, v);                                               // keep only the last 10 versions
    double trim_ms = std::chrono::duration<double, std::milli>(Clock::now() - t1).count();
    std::printf("%s,%s,%zu,%.2f,%.1f,%.1f,%.1f,%.2f,%.1f\n", hist, name, h.size(), commit_us, mb, hist_ns, head_ns, trim_ms, (g_live - m0) / 1e6);
}

int main() {
    std::printf("history,design,versions,commit_us_per_version,heap_MB,read_ns_history,read_ns_head,trim_ms,heap_MB_after_trim\n");
    auto trim_release = [](auto& s, Version last) { for (Version x = 0; x + 10 <= last; ++x) s.release(x); };
    auto trim_delta = [](DeltaChainStore& s, Version last) { for (Version x = 0; x + 10 <= last; ++x) s.retire(x); s.compact(last - 9); };
    for (auto [hist, versions, per] : {std::tuple{"A_few_large", 20, int(N * 3 / 10)}, std::tuple{"B_many_tiny", 1000, 1}}) {
        auto h = history(versions, per);
        run(hist, "full_copy", [] { return FullCopyStore(N); }, h, trim_release);
        run(hist, "delta_chain", [] { return DeltaChainStore(N, 0); }, h, trim_delta);
        run(hist, "delta_ckpt32", [] { return DeltaChainStore(N, 32); }, h, trim_delta);
        run(hist, "path_copy", [] { return PathCopyStore(N); }, h, trim_release);
    }
}
