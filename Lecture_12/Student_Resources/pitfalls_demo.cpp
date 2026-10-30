// SE618 Lecture 12 — five ways a benchmark lies, each shown with real measurements.
// g++ -std=c++20 -O2 pitfalls_demo.cpp -o pit && ./pit          (~10 s)
// Then compare with an unoptimised build:  g++ -std=c++20 -O0 pitfalls_demo.cpp -o pit0 && ./pit0
#include "bench_harness.hpp"

#include <algorithm>
#include <chrono>
#include <numeric>
#include <random>
#include <vector>

using namespace se618;
using Clock = std::chrono::steady_clock;

static std::vector<int> random_ints(std::size_t n, unsigned seed) {
    std::mt19937 g(seed); std::vector<int> v(n); for (auto& x : v) x = static_cast<int>(g()); return v;
}
static void insertion_sort(std::vector<int>& v) {
    for (std::size_t i = 1; i < v.size(); ++i) { int x = v[i]; std::size_t j = i; while (j && v[j - 1] > x) { v[j] = v[j - 1]; --j; } v[j] = x; }
}

int main() {
    std::printf("P1  Dead-code elimination: a loop whose result is never used\n");
    {
        std::vector<long long> v(10'000'000); std::iota(v.begin(), v.end(), 0);
        auto a = Clock::now();
        long long s = 0; for (auto x : v) s += x;                         // result unused below
        auto b = Clock::now();
        long long s2 = 0; auto c = Clock::now(); for (auto x : v) s2 += x; do_not_optimize(s2); auto d = Clock::now();
        std::printf("    result discarded : %8.3f ms   (the compiler may delete the loop)\n", std::chrono::duration<double, std::milli>(b - a).count());
        std::printf("    result used      : %8.3f ms\n\n", std::chrono::duration<double, std::milli>(d - c).count());
        (void)s;
    }

    std::printf("P2  Cold vs warm: the first call pays page faults, allocation and cache misses\n");
    {
        std::vector<double> first_calls;
        auto samples = measure([] { std::vector<int> v(4'000'000, 1); return std::accumulate(v.begin(), v.end(), 0LL); }, 0, 10);
        std::printf("    call 1: %.2f ms   calls 2-10 median: %.2f ms\n\n", samples[0] / 1e6,
                    quantile(std::vector<double>(samples.begin() + 1, samples.end()), 0.5) / 1e6);
    }

    std::printf("P3  Input size decides the winner: insertion sort vs std::sort\n");
    std::printf("    n        insertion_us   std_sort_us\n");
    for (std::size_t n : {16u, 64u, 256u, 4096u}) {
        auto base = random_ints(n, 3);
        auto ins = summarize(measure([&] { auto v = base; insertion_sort(v); return v[0]; }, 3, 51));
        auto std_ = summarize(measure([&] { auto v = base; std::sort(v.begin(), v.end()); return v[0]; }, 3, 51));
        std::printf("    %-8zu %12.2f %13.2f\n", n, ins.median / 1e3, std_.median / 1e3);
    }

    std::printf("\nP4  One run vs a distribution: sorting 200,000 ints, 30 repetitions\n");
    {
        auto base = random_ints(200000, 4);
        auto s = measure([&] { auto v = base; std::sort(v.begin(), v.end()); return v[0]; }, 2, 30);
        std::printf("    a single run could have reported anything from %.2f to %.2f ms\n", *std::min_element(s.begin(), s.end()) / 1e6, *std::max_element(s.begin(), s.end()) / 1e6);
        print_summary("    sort 200k", summarize(s), 1e-6, "ms");
    }

    std::printf("\nP5  Unequal work: 'B is much faster' — but B was given already-sorted data\n");
    {
        auto rnd = random_ints(1'000'000, 5);
        auto sorted = rnd; std::sort(sorted.begin(), sorted.end());
        auto a = summarize(measure([&] { auto v = rnd; std::sort(v.begin(), v.end()); return v[0]; }, 1, 9));
        auto b = summarize(measure([&] { auto v = sorted; std::sort(v.begin(), v.end()); return v[0]; }, 1, 9));
        std::printf("    'A' random input: %.1f ms    'B' sorted input: %.1f ms    ratio %.1fx — same algorithm\n",
                    a.median / 1e6, b.median / 1e6, a.median / b.median);
    }
}
