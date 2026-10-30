// SE618 Lecture 10 — Count-Min Sketch vs exact counting on a skewed stream.
// g++ -std=c++20 -O2 cms_demo.cpp -o cms && ./cms          (~5 s)
// Stream: 2,000,000 events over 200,000 keys, Zipf-like popularity (weight 1/rank).
#ifdef SOLUTION
#include "bloom_solution.hpp"
#else
#include "bloom.hpp"
#endif
#include <algorithm>
#include <cstdio>
#include <random>
#include <unordered_map>

using namespace se618;

int main() {
    const std::size_t K = 200000, EVENTS = 2000000;
    std::vector<double> w(K); for (std::size_t i = 0; i < K; ++i) w[i] = 1.0 / (i + 1);
    std::discrete_distribution<std::size_t> pick(w.begin(), w.end());
    std::mt19937_64 g(5);
    std::vector<std::uint64_t> stream(EVENTS);
    for (auto& e : stream) e = pick(g);                       // key = popularity rank

    std::unordered_map<std::uint64_t, std::uint64_t> exact;
    for (auto e : stream) ++exact[e];
    std::printf("exact: %zu distinct keys, ~%.1f MB as unordered_map (rough: ~48 B/entry)\n\n", exact.size(), exact.size() * 48 / 1e6);

    std::printf("eps,delta,width,depth,KB,bound_eps_N,max_overestimate,median_rel_err_top100,median_abs_err_tail,frac_tail_err_gt_bound\n");
    for (double eps : {0.01, 0.001, 0.0001}) {
        auto cms = CountMinSketch::for_error(eps, 0.01, 17);
        for (auto e : stream) cms.add(e);
        double bound = eps * cms.total();
        std::uint64_t max_over = 0;
        std::vector<double> rel_top, abs_tail;
        std::size_t over_bound = 0, tail_n = 0;
        for (auto& [key, c] : exact) {
            std::uint64_t est = cms.estimate(key);
            if (est < c) { std::printf("UNDERESTIMATE: impossible for a correct CMS\n"); return 1; }
            max_over = std::max(max_over, est - c);
            if (key < 100) rel_top.push_back(double(est - c) / c);
            else if (key >= 10000) { abs_tail.push_back(double(est - c)); ++tail_n; if (est - c > bound) ++over_bound; }
        }
        auto med = [](std::vector<double>& v) { std::nth_element(v.begin(), v.begin() + v.size() / 2, v.end()); return v[v.size() / 2]; };
        std::printf("%g,0.01,%zu,%zu,%.0f,%.0f,%llu,%.4f,%.0f,%.4f\n", eps, cms.width(), cms.depth(), cms.memory_bytes() / 1024.0,
                    bound, (unsigned long long)max_over, med(rel_top), med(abs_tail), double(over_bound) / tail_n);
    }
}
