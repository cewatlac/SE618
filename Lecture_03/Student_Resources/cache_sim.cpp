// SE618 Lecture 3 — which eviction policy is "best"? Depends on the workload.
// g++ -std=c++20 -O2 cache_sim.cpp -o sim && ./sim
// Prints CSV: workload,policy,capacity,hit_rate
#include "cache_policies.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <random>
#include <string>
#include <vector>

using namespace se618;
constexpr Key kScanBase = 1'000'000'000'000ULL;

struct Zipf {  // P(rank r) proportional to 1 / r^s, r = 1..n
    std::vector<double> cdf;
    Zipf(std::size_t n, double s) : cdf(n) {
        double sum = 0;
        for (std::size_t r = 0; r < n; ++r) { sum += 1.0 / std::pow(r + 1.0, s); cdf[r] = sum; }
        for (auto& c : cdf) c /= sum;
    }
    Key operator()(std::mt19937_64& g) const {
        double u = std::uniform_real_distribution<double>(0, 1)(g);
        return static_cast<Key>(std::lower_bound(cdf.begin(), cdf.end(), u) - cdf.begin());
    }
};

static std::vector<Key> make(const std::string& w, std::size_t reqs, std::size_t cap) {
    std::mt19937_64 g(618);
    const std::size_t N = 100000;
    Zipf z(N, 0.9);
    std::vector<Key> t; t.reserve(reqs);
    Key scan_id = kScanBase;
    for (std::size_t i = 0; t.size() < reqs; ++i) {
        if (w == "zipf") t.push_back(z(g));
        else if (w == "zipf+scans") {             // a batch job reads 2 x capacity one-off keys every 10k requests
            if (i % 10000 == 5000) for (std::size_t j = 0; j < 2 * cap; ++j) t.push_back(scan_id++);
            else t.push_back(z(g));
        } else if (w == "shifting") {             // popularity reshuffles every 200k requests (news, trends)
            Key phase = static_cast<Key>(i / 200000);
            t.push_back(z(g) + phase * 7919 % N * 13 % N + phase * N);
        } else if (w == "loop") {                 // cyclic access to 1.2 x capacity keys (iterating a table)
            t.push_back(i % static_cast<std::size_t>(cap * 1.2));
        }
    }
    t.resize(reqs);
    return t;
}

template <class P> static double hit_rate(const std::vector<Key>& t, std::size_t cap) {
    P p(cap);
    // Hit rate on user traffic: skip the cold-start 10% and do not count batch-scan requests.
    std::size_t hits = 0, counted = 0, warm = t.size() / 10;
    for (std::size_t i = 0; i < t.size(); ++i) {
        bool h = p.access(t[i]);
        if (i >= warm && t[i] < kScanBase) { hits += h; ++counted; }
    }
    return double(hits) / double(counted);
}

int main() {
    std::printf("workload,policy,capacity,hit_rate\n");
    for (std::string w : {"zipf", "zipf+scans", "shifting", "loop"}) {
        for (std::size_t cap : {1000, 5000}) {
            auto t = make(w, 1'000'000, cap);
            std::printf("%s,FIFO,%zu,%.3f\n", w.c_str(), cap, hit_rate<FifoPolicy>(t, cap));
            std::printf("%s,LRU,%zu,%.3f\n", w.c_str(), cap, hit_rate<LruPolicy>(t, cap));
            std::printf("%s,LFU,%zu,%.3f\n", w.c_str(), cap, hit_rate<LfuPolicy>(t, cap));
            std::printf("%s,SLRU,%zu,%.3f\n", w.c_str(), cap, hit_rate<SlruPolicy>(t, cap));
            std::printf("%s,Random,%zu,%.3f\n", w.c_str(), cap, hit_rate<RandomPolicy>(t, cap));
        }
    }
}
