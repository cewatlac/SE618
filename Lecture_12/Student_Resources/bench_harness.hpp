// SE618 Lecture 12 — a small, honest benchmark harness you may reuse in your project
// (acknowledge it in ACKNOWLEDGEMENTS.md).
//
//   auto samples = se618::measure([&] { return work(); }, /*warmup=*/3, /*reps=*/30);
//   auto s = se618::summarize(samples);                  // median, p10/p90, bootstrap 95% CI of median
//   se618::write_csv("results/raw_sort.csv", "sort_1e6", samples);   // keep raw data
//
// - Each sample is one timed call of f (use a batch inside f for very short operations).
// - f's return value is passed through do_not_optimize so the work cannot be removed.
// - Warm-up calls are run and discarded; reported timings are "warm".
// - The CI is a percentile bootstrap of the median: an intuitive uncertainty band, not a proof.
//   It describes random variation only; it does NOT remove systematic bias (unequal workloads,
//   compiler flags, thermal effects, an unrepresentative workload).
#pragma once
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <random>
#include <string>
#include <vector>

namespace se618 {

template <class T> inline void do_not_optimize(const T& value) {
#if defined(__GNUC__) || defined(__clang__)
    asm volatile("" : : "r,m"(value) : "memory");
#else
    static volatile const void* sink; sink = &value;
#endif
}

template <class F> std::vector<double> measure(F&& f, int warmup, int reps) {
    for (int i = 0; i < warmup; ++i) { auto r = f(); do_not_optimize(r); }
    std::vector<double> ns;
    ns.reserve(static_cast<std::size_t>(reps));
    for (int i = 0; i < reps; ++i) {
        auto a = std::chrono::steady_clock::now();
        auto r = f();
        auto b = std::chrono::steady_clock::now();
        do_not_optimize(r);
        ns.push_back(std::chrono::duration<double, std::nano>(b - a).count());
    }
    return ns;
}

struct Summary { std::size_t n; double min, p10, median, p90, max, mean, ci_lo, ci_hi; };

inline double quantile(std::vector<double> v, double q) {           // nearest-rank on a sorted copy
    std::sort(v.begin(), v.end());
    return v[static_cast<std::size_t>(std::lround(q * static_cast<double>(v.size() - 1)))];
}

inline Summary summarize(const std::vector<double>& s, int bootstrap = 2000, unsigned seed = 1) {
    Summary r{};
    r.n = s.size();
    r.min = *std::min_element(s.begin(), s.end());
    r.max = *std::max_element(s.begin(), s.end());
    r.p10 = quantile(s, 0.10); r.median = quantile(s, 0.5); r.p90 = quantile(s, 0.90);
    double sum = 0; for (double x : s) sum += x; r.mean = sum / static_cast<double>(s.size());
    std::mt19937 g(seed);
    std::uniform_int_distribution<std::size_t> pick(0, s.size() - 1);
    std::vector<double> meds, re(s.size());
    for (int b = 0; b < bootstrap; ++b) { for (auto& x : re) x = s[pick(g)]; meds.push_back(quantile(re, 0.5)); }
    r.ci_lo = quantile(meds, 0.025); r.ci_hi = quantile(meds, 0.975);
    return r;
}

inline void print_summary(const char* label, const Summary& s, double scale = 1e-3, const char* unit = "us") {
    std::printf("%-28s n=%zu median=%.2f%s [95%% CI %.2f-%.2f] p10=%.2f p90=%.2f min=%.2f max=%.2f mean=%.2f\n",
                label, s.n, s.median * scale, unit, s.ci_lo * scale, s.ci_hi * scale, s.p10 * scale, s.p90 * scale,
                s.min * scale, s.max * scale, s.mean * scale);
}

inline bool write_csv(const std::string& path, const std::string& label, const std::vector<double>& ns) {
    std::FILE* f = std::fopen(path.c_str(), "a");
    if (!f) return false;
    for (std::size_t i = 0; i < ns.size(); ++i) std::fprintf(f, "%s,%zu,%.1f\n", label.c_str(), i, ns[i]);
    std::fclose(f);
    return true;
}

}  // namespace se618
