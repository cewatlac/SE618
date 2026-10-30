// SE618 Lecture 11 — false sharing: independent counters on one cache line vs padded counters.
// g++ -std=c++20 -O2 -pthread false_sharing.cpp -o fs && ./fs        (~10 s)
//
// Each thread increments ONLY ITS OWN counter (no logical sharing at all). In the packed layout
// the counters are adjacent 8-byte atomics, so up to 8 of them share one 64-byte cache line;
// the cache line bounces between cores on every write. In the padded layout each counter has its
// own cache line. Throughput = total increments per second, median of 3 runs.
// Optional experiment: on machines with few cores the effect shrinks or disappears; results
// depend on CPU, number of physical cores, OS scheduler and compiler.
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>
#include <vector>

constexpr std::uint64_t ITERS = 20'000'000;

struct Packed { std::atomic<std::uint64_t> c[16]; };
struct alignas(64) PaddedCounter { std::atomic<std::uint64_t> v; };
struct Padded { PaddedCounter c[16]; };
static_assert(sizeof(Padded) == 16 * 64, "each padded counter occupies one cache line");

template <class Get> static double run(int threads, Get&& counter) {
    std::vector<double> r;
    for (int rep = 0; rep < 3; ++rep) {
        auto a = std::chrono::steady_clock::now();
        std::vector<std::thread> pool;
        for (int t = 0; t < threads; ++t)
            pool.emplace_back([&, t] {
                auto& c = counter(t);
                for (std::uint64_t i = 0; i < ITERS; ++i) c.fetch_add(1, std::memory_order_relaxed);
            });
        for (auto& th : pool) th.join();
        double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - a).count();
        r.push_back(threads * ITERS / s / 1e6);
    }
    std::sort(r.begin(), r.end());
    return r[1];
}

int main() {
    unsigned hw = std::thread::hardware_concurrency();
    std::printf("# hardware threads: %u; %llu increments per thread; million increments/s, median of 3\n", hw, (unsigned long long)ITERS);
    std::printf("threads,packed_Minc_per_s,padded_Minc_per_s,padded_over_packed\n");
    static Packed packed{};
    static Padded padded{};
    for (int t : {1, 2, 4, 8}) {
        if (hw && static_cast<unsigned>(t) > hw) { std::printf("%d,skipped\n", t); continue; }
        double p = run(t, [&](int i) -> std::atomic<std::uint64_t>& { return packed.c[i]; });
        double q = run(t, [&](int i) -> std::atomic<std::uint64_t>& { return padded.c[i].v; });
        std::printf("%d,%.0f,%.0f,%.1fx\n", t, p, q, q / p);
    }
}
