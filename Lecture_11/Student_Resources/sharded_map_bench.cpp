// SE618 Lecture 11 — global lock vs reader-writer lock vs sharded locks, uniform vs hot-key.
// g++ -std=c++20 -O2 -pthread sharded_map_bench.cpp -o bench && ./bench        (~30 s)
// ./bench --rwlock also measures std::shared_mutex. On some platforms (e.g. MinGW/winpthreads)
// it is orders of magnitude slower than std::mutex and the run takes many minutes.
//
// Each thread performs OPS operations on keys prepared before timing: 90% get, 10% increment.
//   uniform : keys uniform over 100,000 keys
//   hot_key : 90% of operations hit key 0, the rest uniform
// Reported: total throughput (million ops/s, all threads), median of 3 runs.
// Results depend on CPU, core count, OS scheduler, compiler and mutex implementation.
// Thread counts above the number of hardware threads measure oversubscription, not parallelism.
#include "concurrent_maps.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <random>
#include <string>
#include <thread>
#include <vector>

using namespace se618;
using Clock = std::chrono::steady_clock;
constexpr std::size_t OPS = 1'000'000, KEYS = 100'000;

struct Op { std::uint64_t key; bool write; };

static std::vector<Op> ops_for(int t, bool hot) {
    std::mt19937_64 g(1000 + t);
    std::vector<Op> v(OPS);
    for (auto& o : v) {
        o.key = (hot && g() % 10 != 0) ? 0 : g() % KEYS;
        o.write = g() % 10 == 0;
    }
    return v;
}

template <class Map> static double mops(int threads, bool hot) {
    std::vector<double> runs;
    for (int rep = 0; rep < 3; ++rep) {
        Map map;
        for (std::uint64_t k = 0; k < KEYS; ++k) map.put(k, 1);           // prefill: gets hit existing keys
        std::vector<std::vector<Op>> work;
        for (int t = 0; t < threads; ++t) work.push_back(ops_for(t, hot));
        std::vector<std::uint64_t> sink(threads * 8);
        auto a = Clock::now();
        std::vector<std::thread> pool;
        for (int t = 0; t < threads; ++t)
            pool.emplace_back([&, t] {
                std::uint64_t s = 0;
                for (auto& o : work[t]) { if (o.write) map.increment(o.key); else s += map.get(o.key); }
                sink[t * 8] = s;
            });
        for (auto& th : pool) th.join();
        double sec = std::chrono::duration<double>(Clock::now() - a).count();
        runs.push_back(threads * OPS / sec / 1e6);
    }
    std::sort(runs.begin(), runs.end());
    return runs[1];
}

int main(int argc, char** argv) {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    bool rw = argc > 1 && std::string(argv[1]) == "--rwlock";
    unsigned hw = std::thread::hardware_concurrency();
    std::printf("# hardware threads reported: %u; %zu ops/thread, 90%% get / 10%% increment, median of 3\n", hw, OPS);
    std::printf("workload,threads,global_mutex_Mops,sharded64_Mops%s\n", rw ? ",global_rwlock_Mops" : "");
    for (bool hot : {false, true})
        for (int t : {1, 2, 4, 8}) {
            if (hw && static_cast<unsigned>(t) > hw) { std::printf("%s,%d,skipped (only %u hardware threads)\n", hot ? "hot_key" : "uniform", t, hw); continue; }
            std::printf("%s,%d,%.1f,%.1f", hot ? "hot_key" : "uniform", t, mops<GlobalLockMap>(t, hot), mops<ShardedMap<64>>(t, hot));
            if (rw) std::printf(",%.2f", mops<GlobalRwLockMap>(t, hot));
            std::printf("\n");
        }
}
