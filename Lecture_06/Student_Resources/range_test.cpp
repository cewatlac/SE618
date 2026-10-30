// SE618 Lecture 6 — differential tests: every representation must honour the same contract.
// g++ -std=c++20 -O1 -g -fsanitize=address,undefined range_test.cpp && ./a.out
#include "range_aggregate.hpp"

#include <algorithm>
#include <cstdio>
#include <random>

using namespace se618;
static int fails = 0;
#define CHECK(c) do { if (!(c)) { ++fails; std::printf("FAIL line %d: %s\n", __LINE__, #c); } } while (0)

template <class R> static void contract(const char* name) {
    R r(std::vector<Value>{5, -2, 7, 0, 3});
    bool ok = r.query(0, 5) == 13 && r.query(1, 3) == 5 && r.query(2, 2) == 0;
    r.set(1, 10);
    ok = ok && r.query(0, 2) == 15 && r.query(1, 2) == 10;
    int threw = 0;
    try { r.query(3, 2); } catch (const std::out_of_range&) { ++threw; }
    try { r.query(0, 6); } catch (const std::out_of_range&) { ++threw; }
    try { r.set(5, 1); } catch (const std::out_of_range&) { ++threw; }
    if (!ok || threw != 3) { ++fails; std::printf("FAIL contract: %s\n", name); }
}

int main() {
    contract<NaiveRange>("naive");
    contract<PrefixSumRange>("prefix");
    contract<FenwickRange>("fenwick");
    contract<SegmentTreeRange<>>("segtree");

    std::mt19937_64 g(6);
    for (int trial = 0; trial < 100 && !fails; ++trial) {
        std::size_t n = 1 + g() % 40;                                  // includes n = 1 and non-powers of two
        std::vector<Value> a(n);
        for (auto& x : a) x = static_cast<Value>(g() % 2001) - 1000;
        NaiveRange ref(a); PrefixSumRange p(a); FenwickRange f(a); SegmentTreeRange<> s(a); SegmentTreeRange<MinOp> m(a);
        std::vector<Value> shadow = a;
        for (int step = 0; step < 200; ++step) {
            if (g() % 3 == 0) {
                std::size_t i = g() % n; Value v = static_cast<Value>(g() % 2001) - 1000;
                ref.set(i, v); p.set(i, v); f.set(i, v); s.set(i, v); m.set(i, v); shadow[i] = v;
            } else {
                std::size_t l = g() % (n + 1), r = g() % (n + 1); if (l > r) std::swap(l, r);
                Value e = ref.query(l, r);
                CHECK(p.query(l, r) == e); CHECK(f.query(l, r) == e); CHECK(s.query(l, r) == e);
                Value mn = l == r ? MinOp::id() : *std::min_element(shadow.begin() + l, shadow.begin() + r);
                CHECK(m.query(l, r) == mn);
            }
            if (fails) { std::printf("  trial=%d step=%d n=%zu\n", trial, step, n); break; }
        }
    }
    std::printf(fails ? "%d failure(s)\n" : "all tests passed\n", fails);
    return fails != 0;
}
