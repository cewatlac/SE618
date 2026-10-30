// SE618 Lecture 10 — Bloom filter lab: plan, verify, measure, overfill.
// g++ -std=c++20 -O2 bloom_lab.cpp -o lab && ./lab        (~15 s)
// Instructor: add -DSOLUTION -I../../../Instructor_Only/Lecture_10
// Write down n, m, k and the predicted p BEFORE running (EXERCISE.md).
#ifdef SOLUTION
#include "bloom_solution.hpp"
#else
#include "bloom.hpp"
#endif
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <random>

using namespace se618;

// Inserts n keys drawn from seed `s`, checks all of them (false negatives), then queries `probes`
// keys that were never inserted and returns the observed false-positive rate.
static double run_once(BloomFilter bf, std::size_t n, std::size_t probes, std::uint64_t s, std::size_t& false_negatives) {
    std::mt19937_64 g(s);
    std::vector<std::uint64_t> keys(n);
    for (auto& k : keys) k = g() | 1;                   // inserted keys are odd ...
    for (auto k : keys) bf.add(k);
    for (auto k : keys) if (!bf.might_contain(k)) ++false_negatives;
    std::size_t fp = 0;
    for (std::size_t i = 0; i < probes; ++i) if (bf.might_contain(g() & ~std::uint64_t{1})) ++fp;   // ... probes even: never inserted
    return static_cast<double>(fp) / static_cast<double>(probes);
}

int main() {
    const std::size_t n = 100000, probes = 1000000;
    const double target = 0.01;
    auto plan = BloomFilter::for_capacity(n, target);
    std::size_t m = plan.bits(); int k = plan.hashes();
    std::printf("PLAN  n=%zu  target p=%.3f  ->  m=%zu bits (%.1f KB, %.2f bits/element)  k=%d  predicted p=%.5f\n\n",
                n, target, m, m / 8192.0, double(m) / n, k, BloomFilter::predicted_fp(m, k, n));

    std::printf("1) Five independent seeds at the planned n (%zu probes each)\n", probes);
    std::printf("seed,false_negatives,observed_fp,95%%_interval\n");
    for (std::uint64_t s = 1; s <= 5; ++s) {
        std::size_t fn = 0;
        double p = run_once(BloomFilter(m, k, s), n, probes, s, fn);
        double half = 1.96 * std::sqrt(p * (1 - p) / probes);   // normal approximation for a proportion
        std::printf("%llu,%zu,%.5f,[%.5f, %.5f]\n", (unsigned long long)s, fn, p, p - half, p + half);
    }

    std::printf("\n2) Small samples: five fresh filters at the planned n, each measured with only 1,000 probes\n");
    for (std::uint64_t s = 11; s <= 15; ++s) { std::size_t fn = 0; std::printf("%.4f ", run_once(BloomFilter(m, k, 1), n, 1000, s, fn)); }
    std::printf("\n\n3) Capacity exceeded: same m and k, more elements than planned\n");
    std::printf("inserted,x_planned,fill_ratio,estimated_fp_from_fill,predicted_fp,observed_fp\n");
    for (double f : {0.5, 1.0, 2.0, 4.0, 10.0}) {
        std::size_t ni = static_cast<std::size_t>(n * f), fn = 0;
        BloomFilter bf(m, k, 7);
        std::mt19937_64 g(7);
        for (std::size_t i = 0; i < ni; ++i) bf.add(g() | 1);
        std::size_t fp = 0;
        for (std::size_t i = 0; i < 200000; ++i) if (bf.might_contain(g() & ~std::uint64_t{1})) ++fp;
        std::printf("%zu,%.1f,%.3f,%.5f,%.5f,%.5f\n", ni, f, bf.fill_ratio(), bf.estimated_fp_now(),
                    BloomFilter::predicted_fp(m, k, ni), fp / 200000.0);
        (void)fn;
    }

    std::printf("\n4) Same memory m, different k (n = planned)\n");
    std::printf("k,predicted_fp,observed_fp\n");
    for (int kk : {1, 2, 4, k, 12, 20}) {
        std::size_t fn = 0;
        std::printf("%d,%.5f,%.5f\n", kk, BloomFilter::predicted_fp(m, kk, n), run_once(BloomFilter(m, kk, 3), n, 300000, 3, fn));
    }
}
