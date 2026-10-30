// SE618 Lecture 10 — correctness tests for the Bloom filter (run before the lab).
// g++ -std=c++20 -O1 -g bloom_test.cpp && ./a.out
#ifdef SOLUTION
#include "bloom_solution.hpp"
#else
#include "bloom.hpp"
#endif
#include <cstdio>
#include <random>
#include <string>

using namespace se618;
static int fails = 0;
#define CHECK(c) do { if (!(c)) { ++fails; std::printf("FAIL line %d: %s\n", __LINE__, #c); } } while (0)

int main() {
    BloomFilter bf = BloomFilter::for_capacity(1000, 0.01);
    CHECK(bf.hashes() == 7 && bf.bits() >= 9585 && bf.bits() <= 9586);    // m = -n ln p / (ln 2)^2
    std::mt19937_64 g(1);
    std::vector<std::uint64_t> keys(1000);
    for (auto& k : keys) { k = g(); bf.add(k); }
    std::size_t fn = 0;
    for (auto k : keys) fn += !bf.might_contain(k);
    CHECK(fn == 0);                                                     // no false negatives, ever
    std::size_t fp = 0;
    for (int i = 0; i < 100000; ++i) fp += bf.might_contain(g());
    CHECK(fp > 300 && fp < 2000);                                       // ~1% expected; wide band avoids flaky failures
    bf.add(std::string("https://example.com/a"));
    CHECK(bf.might_contain(std::string("https://example.com/a")));
    CHECK(bf.fill_ratio() > 0.4 && bf.fill_ratio() < 0.6);              // ~0.5 at the design point

    BloomFilter a(4096, 3, 9), b(4096, 3, 9), c(4096, 4, 9);
    a.add(std::uint64_t{1}); b.add(std::uint64_t{2});
    CHECK(a.merge(b) && a.might_contain(std::uint64_t{1}) && a.might_contain(std::uint64_t{2}));
    CHECK(!a.merge(c));                                                 // different parameters: refuse

    CountMinSketch cms(100, 4);
    for (int i = 0; i < 50; ++i) cms.add(42);
    for (std::uint64_t i = 0; i < 1000; ++i) cms.add(i + 1000);
    CHECK(cms.estimate(42) >= 50);                                       // never underestimates
    std::printf(fails ? "%d failure(s)\n" : "all tests passed\n", fails);
    return fails != 0;
}
