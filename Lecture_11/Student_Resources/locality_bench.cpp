// SE618 Lecture 11 — memory layout: the same logical work (sum n int64 values) over different layouts.
// g++ -std=c++20 -O2 locality_bench.cpp -o loc && ./loc        (~20 s)
//
// Layouts: contiguous vector; linked list whose nodes sit in memory in list order ("sequential
// list": pointer chasing, but prefetch-friendly); linked list whose nodes are in shuffled memory
// order ("random list": every hop is a likely cache miss once the list exceeds the caches).
// Also: array of structures (64-byte particle) vs structure of arrays, summing one field.
// Metric: ns per element, median of 5 passes. Working-set sizes span L1/L2 -> beyond LLC.
// One machine: the cache sizes, prefetchers and memory latency of YOUR machine change the numbers.
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <numeric>
#include <random>
#include <vector>

using Clock = std::chrono::steady_clock;
struct Node { std::int64_t value; Node* next; };

template <class F> static double ns_per_elem(std::size_t n, F&& pass) {
    std::vector<double> t;
    volatile std::int64_t sink = 0;
    for (int r = 0; r < 5; ++r) {
        auto a = Clock::now(); sink = sink + pass(); auto b = Clock::now();
        t.push_back(std::chrono::duration<double, std::nano>(b - a).count() / n);
    }
    std::sort(t.begin(), t.end());
    return t[2];
}

struct Particle { double x, y, z, vx, vy, vz, mass; std::int64_t id; };   // 64 bytes
struct Particles { std::vector<double> x, y, z, vx, vy, vz, mass; std::vector<std::int64_t> id; };

int main() {
    std::printf("elements,working_set_KB,vector_ns,seq_list_ns,random_list_ns,aos_mass_ns,soa_mass_ns\n");
    for (std::size_t n : {1u << 10, 1u << 14, 1u << 18, 1u << 22}) {
        std::vector<std::int64_t> v(n);
        std::iota(v.begin(), v.end(), 0);
        std::vector<Node> pool(n);                          // nodes live in one block ...
        std::vector<std::size_t> order(n);
        std::iota(order.begin(), order.end(), 0);
        auto link = [&](const std::vector<std::size_t>& ord) {
            for (std::size_t i = 0; i < n; ++i) { pool[ord[i]].value = static_cast<std::int64_t>(i); pool[ord[i]].next = i + 1 < n ? &pool[ord[i + 1]] : nullptr; }
            return &pool[ord[0]];
        };
        auto walk = [](Node* h) { std::int64_t s = 0; for (; h; h = h->next) s += h->value; return s; };
        double vec = ns_per_elem(n, [&] { std::int64_t s = 0; for (auto x : v) s += x; return s; });
        Node* seq = link(order);                            // ... linked in memory order
        double seql = ns_per_elem(n, [&] { return walk(seq); });
        std::shuffle(order.begin(), order.end(), std::mt19937_64(1));
        Node* rnd = link(order);                            // ... linked in random memory order
        double rndl = ns_per_elem(n, [&] { return walk(rnd); });
        std::vector<Particle> aos(n);
        Particles soa; soa.mass.resize(n); soa.x.resize(n); soa.y.resize(n); soa.z.resize(n);
        soa.vx.resize(n); soa.vy.resize(n); soa.vz.resize(n); soa.id.resize(n);
        for (std::size_t i = 0; i < n; ++i) { aos[i].mass = soa.mass[i] = static_cast<double>(i % 7); }
        double a = ns_per_elem(n, [&] { double s = 0; for (auto& p : aos) s += p.mass; return static_cast<std::int64_t>(s); });
        double b = ns_per_elem(n, [&] { double s = 0; for (auto m : soa.mass) s += m; return static_cast<std::int64_t>(s); });
        std::printf("%zu,%zu,%.2f,%.2f,%.2f,%.2f,%.2f\n", n, n * sizeof(Node) / 1024, vec, seql, rndl, a, b);
    }
}
