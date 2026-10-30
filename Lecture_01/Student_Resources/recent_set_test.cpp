// SE618 Lecture 1 — tests as executable contract. No framework needed.
// g++ -std=c++20 -O1 -g -Wall -Wextra -fsanitize=address,undefined recent_set_test.cpp && ./a.out
#include "recent_set.hpp"

#include <cstdio>
#include <random>
#include <string>

using namespace se618;
static int failures = 0;
#define CHECK(c) do { if (!(c)) { ++failures; std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); } } while (0)

static void contract_tests() {
    RecentSet<std::string> s(2);
    CHECK(s.add("A").inserted);
    CHECK(!s.add("A").inserted);                   // duplicate is reported, not silently accepted
    CHECK(s.add("B").inserted);
    auto r = s.add("C");                           // exceeds capacity
    CHECK(r.inserted && r.evicted && *r.evicted == "A");
    CHECK(!s.contains("A") && s.contains("B") && s.contains("C"));
    CHECK(s.size() == 2);

    RecentSet<int> d(2);                           // duplicate must NOT refresh position
    d.add(1); d.add(2); d.add(1); d.add(3);
    CHECK(!d.contains(1) && d.contains(2) && d.contains(3));

    CHECK(!s.erase("zzz"));                        // missing key is a normal outcome, not an error
    CHECK(s.erase("B") && !s.contains("B") && s.size() == 1);
    s.clear();
    CHECK(s.size() == 0 && s.check_invariants());

    bool threw = false;
    try { RecentSet<int> z(0); } catch (const std::invalid_argument&) { threw = true; }
    CHECK(threw);
}

// Differential test: random operation sequences against the obviously-correct baseline.
static void differential_test() {
    std::mt19937_64 rng(618);                      // fixed seed => reproducible failure
    for (int trial = 0; trial < 200; ++trial) {
        std::size_t cap = 1 + rng() % 8;
        RecentSet<int> fast(cap);
        RecentSetBaseline<int> ref(cap);
        for (int step = 0; step < 500; ++step) {
            int k = static_cast<int>(rng() % 12);  // small key space => many duplicates and evictions
            switch (rng() % 3) {
                case 0: { auto a = fast.add(k), b = ref.add(k);
                          CHECK(a.inserted == b.inserted && a.evicted == b.evicted); break; }
                case 1: CHECK(fast.contains(k) == ref.contains(k)); break;
                case 2: CHECK(fast.erase(k) == ref.erase(k)); break;
            }
            CHECK(fast.size() == ref.size());
            CHECK(fast.check_invariants());
            if (failures) { std::printf("  trial=%d step=%d cap=%zu key=%d\n", trial, step, cap, k); return; }
        }
    }
}

int main() {
    contract_tests();
    differential_test();
    std::printf(failures ? "%d FAILURE(S)\n" : "all tests passed\n", failures);
    return failures ? 1 : 0;
}
