// SE618 Lecture 7 — dependency resolver. The API below is the IMPROVED contract;
// the implementation is the original naive one. Your task: make the implementation honour
// the contract (see EXERCISE.md). resolver_test.cpp specifies the expected behaviour.
//
// Contract of resolve():
//   1. order lists every resolvable package, each after all of its dependencies.
//   2. Deterministic: among packages that are ready at the same time, the lexicographically
//      smallest name comes first. The result does not depend on input order.
//   3. ok == false iff there is at least one ERROR diagnostic.
//        Errors  : MissingDependency, SelfDependency, Cycle, DuplicatePackage
//        Warnings: DuplicateDependency (ignored after the first occurrence)
//   4. Partial result: on error, order still contains every package whose dependencies can
//      all be resolved; packages on/after a cycle or missing dependency are left out.
//   5. A Cycle diagnostic carries the path, e.g. {"a","b","c","a"}, starting at the
//      lexicographically smallest package on the cycle. One diagnostic per cycle found.
//   6. Diagnostics are sorted by (kind, node) so output is stable across runs.
#pragma once
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace se618 {

struct Package { std::string name; std::vector<std::string> deps; };

enum class Kind { MissingDependency, SelfDependency, Cycle, DuplicatePackage, DuplicateDependency };
inline bool is_error(Kind k) { return k != Kind::DuplicateDependency; }

struct Diagnostic {
    Kind kind;
    std::string node;               // package the diagnostic is about
    std::vector<std::string> path;  // Cycle: a -> b -> c -> a ; MissingDependency: {package, missing}
    std::string message;            // human-readable, one line
};

struct Resolution {
    bool ok = true;
    std::vector<std::string> order;
    std::vector<Diagnostic> diagnostics;
};

// ---------------- ORIGINAL NAIVE IMPLEMENTATION (replace me) ----------------
inline Resolution resolve(const std::vector<Package>& pkgs) {
    std::unordered_map<std::string, std::vector<std::string>> g;
    for (auto& p : pkgs) {
        auto& d = g[p.name];                       // duplicate package: silently overwritten/merged
        d.insert(d.end(), p.deps.begin(), p.deps.end());
    }
    for (auto& p : pkgs) for (auto& d : p.deps) g[d];   // missing dependency: silently invented
    Resolution res;
    std::unordered_set<std::string> done, active;
    std::function<void(const std::string&)> visit = [&](const std::string& n) {
        if (done.count(n)) return;
        if (active.count(n)) throw std::runtime_error("cycle");
        active.insert(n);
        for (auto& d : g[n]) visit(d);
        active.erase(n); done.insert(n);
        res.order.push_back(n);
    };
    try {
        for (auto& [name, deps] : g) visit(name);        // hash-map iteration order: unspecified
    } catch (const std::runtime_error&) {
        return Resolution{false, {}, {Diagnostic{Kind::Cycle, "", {}, "cycle detected"}}};
    }
    return res;
}

}  // namespace se618
