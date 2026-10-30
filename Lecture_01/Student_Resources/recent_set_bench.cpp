// SE618 Lecture 1 — a deliberately simple benchmark harness, to be critiqued in class.
// g++ -std=c++20 -O2 recent_set_bench.cpp -o bench && ./bench
// Prints CSV: design,capacity,dup_rate,ns_per_op,checksum
#include "recent_set.hpp"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>

template <class Set>
static double run(std::size_t cap, double dup_rate, std::size_t ops, std::uint64_t& checksum) {
    std::mt19937_64 rng(42);
    std::bernoulli_distribution dup(dup_rate);
    std::vector<std::uint64_t> keys(ops);
    std::uint64_t next = 1;
    for (auto& k : keys)  // a "duplicate" re-sends one of the last `cap` distinct IDs
        k = (dup(rng) && next > 1) ? next - 1 - rng() % std::min<std::uint64_t>(next - 1, cap) : next++;
    Set s(cap);
    auto t0 = std::chrono::steady_clock::now();
    for (auto k : keys) checksum += s.add(k).inserted;   // use the result so the loop is not optimised away
    auto t1 = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::nano>(t1 - t0).count() / ops;
}

int main() {
    std::printf("design,capacity,dup_rate,ns_per_op,checksum\n");
    for (std::size_t cap : {16, 256, 4096, 65536}) {
        for (double d : {0.01, 0.30}) {
            std::size_t ops = 200000;
            std::uint64_t c1 = 0, c2 = 0;
            double fast = run<se618::RecentSet<std::uint64_t>>(cap, d, ops, c1);
            std::printf("list+map,%zu,%.2f,%.1f,%llu\n", cap, d, fast, (unsigned long long)c1);
            if (cap <= 4096) {  // the baseline is O(capacity) per op; skip the largest size
                double base = run<se618::RecentSetBaseline<std::uint64_t>>(cap, d, ops, c2);
                std::printf("vector,%zu,%.2f,%.1f,%llu\n", cap, d, base, (unsigned long long)c2);
            }
        }
    }
}
