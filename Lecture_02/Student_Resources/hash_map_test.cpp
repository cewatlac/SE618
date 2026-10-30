// SE618 Lecture 2 — differential test: FlatMap vs std::unordered_map (the oracle).
// g++ -std=c++20 -O1 -g -fsanitize=address,undefined hash_map_test.cpp && ./a.out
// Instructor: add -DFIXED -I../../../Instructor_Only/Lecture_02 to test the solution.
#ifdef FIXED
#include "fixed_hash_map.hpp"
#else
#include "flawed_hash_map.hpp"
#endif
#include <cstdio>
#include <random>
#include <unordered_map>

using se618::FlatMap;

static bool smoke() {                       // the tests most people write first — they pass
    FlatMap<int, int> m;
    for (int i = 0; i < 1000; ++i) m.insert_or_assign(i, i * i);
    for (int i = 0; i < 1000; ++i) if (!m.find(i) || *m.find(i) != i * i) return false;
    for (int i = 0; i < 1000; i += 2) m.erase(i);
    for (int i = 0; i < 1000; ++i) if (m.contains(i) != (i % 2 == 1)) return false;
    return m.size() == 500;
}

// Random operations on a SMALL key space so collisions, tombstones and overwrites are frequent.
static bool differential(std::uint64_t seed, int steps, int keyspace) {
    std::mt19937_64 rng(seed);
    FlatMap<std::uint64_t, int> m(8);
    std::unordered_map<std::uint64_t, int> ref;
    for (int step = 0; step < steps; ++step) {
        std::uint64_t k = rng() % keyspace;
        int op = static_cast<int>(rng() % 3), v = static_cast<int>(rng() % 1000);
        bool ok = true;
        if (op == 0) { bool a = m.insert_or_assign(k, v); bool b = ref.insert_or_assign(k, v).second; ok = a == b; }
        else if (op == 1) { bool a = m.erase(k); bool b = ref.erase(k) == 1; ok = a == b; }
        else { int* p = m.find(k); auto it = ref.find(k); ok = (p == nullptr) == (it == ref.end()) && (!p || *p == it->second); }
        ok = ok && m.size() == ref.size();
        if (!ok) { std::printf("  mismatch: seed=%llu step=%d op=%d key=%llu size=%zu ref=%zu\n",
                               (unsigned long long)seed, step, op, (unsigned long long)k, m.size(), ref.size()); return false; }
    }
    return true;
}

int main() {
    std::printf("smoke test:        %s\n", smoke() ? "pass" : "FAIL");
    int bad = 0;
    for (std::uint64_t seed = 1; seed <= 300 && bad < 3; ++seed) bad += !differential(seed, 2000, 24);
    std::printf("differential test: %s\n", bad ? "FAIL" : "pass");
    return bad ? 1 : 0;
}
