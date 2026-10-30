// SE618 Lecture 7 — behaviour specification for resolve().
// g++ -std=c++20 -O1 -g resolver_test.cpp && ./a.out
// Instructor: add -DSOLUTION -I../../../Instructor_Only/Lecture_07 to test the solution.
#ifdef SOLUTION
#include "resolver_solution.hpp"
#else
#include "resolver.hpp"
#endif
#include <algorithm>
#include <cstdio>
#include <random>

using namespace se618;
using S = std::vector<std::string>;
static int fails = 0;
#define CHECK(c) do { if (!(c)) { ++fails; std::printf("FAIL line %d: %s\n", __LINE__, #c); } } while (0)

static const Diagnostic* find(const Resolution& r, Kind k) {
    for (auto& d : r.diagnostics) if (d.kind == k) return &d;
    return nullptr;
}

int main() {
    // 1. Valid graph with several valid orders -> the one deterministic order.
    std::vector<Package> app = {{"app", {"log", "net"}}, {"net", {"tls", "log"}}, {"tls", {}}, {"log", {}}, {"cli", {"log"}}};
    auto r = resolve(app);
    CHECK(r.ok && r.diagnostics.empty());
    CHECK((r.order == S{"log", "cli", "tls", "net", "app"}));

    // 2. Same packages, shuffled input -> identical output.
    std::mt19937 g(7);
    for (int i = 0; i < 20; ++i) {
        auto shuffled = app;
        std::shuffle(shuffled.begin(), shuffled.end(), g);
        if (resolve(shuffled).order != r.order) { ++fails; std::printf("FAIL: order depends on input order\n"); break; }
    }

    // 3. Missing dependency is an error, reported with the path; unaffected packages still resolve.
    r = resolve({{"api", {"db", "cache"}}, {"db", {}}, {"web", {"api"}}, {"util", {}}});
    auto* m = find(r, Kind::MissingDependency);
    CHECK(!r.ok && m && m->node == "api" && (m->path == S{"api", "cache"}));
    CHECK((r.order == S{"db", "util"}));

    // 4. Cycle diagnostic names the cycle; the rest of the graph still resolves.
    r = resolve({{"b", {"c"}}, {"c", {"a"}}, {"a", {"b"}}, {"x", {}}, {"y", {"x"}}, {"z", {"a"}}});
    auto* c = find(r, Kind::Cycle);
    CHECK(!r.ok && c && (c->path == S{"a", "b", "c", "a"}));
    CHECK((r.order == S{"x", "y"}));

    // 5. Self-dependency is an error; duplicate dependency is only a warning.
    r = resolve({{"a", {"a"}}});
    CHECK(!r.ok && find(r, Kind::SelfDependency));
    r = resolve({{"a", {"b", "b"}}, {"b", {}}});
    CHECK(r.ok && find(r, Kind::DuplicateDependency) && (r.order == S{"b", "a"}));

    // 6. Duplicate package declaration is an error, not a silent merge.
    r = resolve({{"a", {}}, {"a", {"b"}}, {"b", {}}});
    CHECK(!r.ok && find(r, Kind::DuplicatePackage));

    std::printf(fails ? "%d failure(s)\n" : "all tests passed\n", fails);
    return fails != 0;
}
