// SE618 Lecture 5 — experiment: linear scan vs inverted index. Write your hypothesis FIRST.
// g++ -std=c++20 -O2 index_bench.cpp -o bench && ./bench          (~1 min)
// (Needs a completed inverted_index.hpp. Instructor: -DSOLUTION -I../../../Instructor_Only/Lecture_05)
//
// Methodology
//  - One prepared query workload (48 queries: rare single terms, common single terms, 2- and
//    3-term AND queries mixing rare/common) is generated BEFORE timing and used, in the same
//    order, for both designs. Answers of both designs are compared (must be identical).
//  - 3 repetitions of the whole workload; every query is timed individually; we report the
//    MEDIAN per-query latency per class over all repetitions (plus the overall 90th percentile).
//  - Memory = live heap bytes allocated while building each component (payload only, allocator
//    overhead excluded). The scan stores the ORIGINAL TEXT; the index stores postings, a forward
//    term list and metadata but NOT the text. This compares these two search components as
//    implemented here — not the storage of a complete production search system.
#ifdef SOLUTION
#include "inverted_index_solution.hpp"
#else
#include "inverted_index.hpp"
#endif
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <new>
#include <random>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wmismatched-new-delete"  // matched pair below
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif
static std::size_t g_live = 0;
void* operator new(std::size_t n) {
    auto* p = static_cast<std::size_t*>(std::malloc(n + 16));
    if (!p) throw std::bad_alloc();
    *p = n; g_live += n; return reinterpret_cast<char*>(p) + 16;
}
void operator delete(void* q) noexcept { if (!q) return; auto* p = reinterpret_cast<std::size_t*>(static_cast<char*>(q) - 16); g_live -= *p; std::free(p); }
void operator delete(void* q, std::size_t) noexcept { operator delete(q); }
// The nothrow forms must use the same header layout (std::stable_sort, for example, uses them).
void* operator new(std::size_t n, const std::nothrow_t&) noexcept { try { return operator new(n); } catch (...) { return nullptr; } }
void operator delete(void* q, const std::nothrow_t&) noexcept { operator delete(q); }

using namespace se618;
using Clock = std::chrono::steady_clock;
constexpr int kVocab = 5000, kWordsPerDoc = 60, kReps = 3;

// Synthetic corpus: Zipf-like word frequencies (rank r has weight 1/r).
static std::vector<std::string> corpus(std::size_t n) {
    std::mt19937_64 g(11);
    std::vector<double> w(kVocab); for (int i = 0; i < kVocab; ++i) w[i] = 1.0 / (i + 1);
    std::discrete_distribution<int> pick(w.begin(), w.end());
    std::vector<std::string> docs(n);
    for (auto& d : docs) for (int k = 0; k < kWordsPerDoc; ++k) { d += "w" + std::to_string(pick(g)); d += ' '; }
    return docs;
}

struct Query { std::string text; std::string cls; };
static std::vector<Query> workload() {                 // fixed seed: identical for every run and design
    std::mt19937 g(2026);
    auto rare = [&] { return "w" + std::to_string(500 + g() % 4500); };   // ranks 500..4999
    auto common = [&] { return "w" + std::to_string(g() % 20); };         // ranks 0..19
    std::vector<Query> q;
    for (int i = 0; i < 12; ++i) q.push_back({rare(), "1_rare"});
    for (int i = 0; i < 12; ++i) q.push_back({common(), "1_common"});
    for (int i = 0; i < 8; ++i) q.push_back({rare() + " " + common(), "2_mixed"});
    for (int i = 0; i < 8; ++i) q.push_back({common() + " " + common(), "2_common"});
    for (int i = 0; i < 8; ++i) q.push_back({common() + " " + common() + " " + rare(), "3_mixed"});
    std::shuffle(q.begin(), q.end(), g);               // interleave classes: no class runs cache-hot in a block
    return q;
}

template <class Search>
static std::map<std::string, std::vector<double>> run(Search&& s, const std::vector<Query>& qs, std::vector<std::vector<DocId>>& answers) {
    std::map<std::string, std::vector<double>> us;
    answers.assign(qs.size(), {});
    for (int rep = 0; rep < kReps; ++rep)
        for (std::size_t i = 0; i < qs.size(); ++i) {
            auto a = Clock::now();
            auto r = s(qs[i].text);
            auto b = Clock::now();
            double t = std::chrono::duration<double, std::micro>(b - a).count();
            us[qs[i].cls].push_back(t); us["all"].push_back(t);
            answers[i] = std::move(r);                 // results are used: cannot be optimised away
        }
    return us;
}
static double pct(std::vector<double> v, double p) {
    std::sort(v.begin(), v.end());
    return v[static_cast<std::size_t>(p * (v.size() - 1))];
}

int main() {
    auto qs = workload();
    std::printf("# %zu queries x %d reps, same sequence for both designs; values = median us per query (p90 overall)\n", qs.size(), kReps);
    std::printf("design,docs,build_ms,heap_MB,1_rare,1_common,2_mixed,2_common,3_mixed,all_median,all_p90\n");
    for (std::size_t n : {1000, 10000, 50000}) {
        auto docs = corpus(n);
        std::vector<std::vector<DocId>> ans_scan, ans_index;
        auto report = [&](const char* name, double ms, double mb, std::map<std::string, std::vector<double>>& us) {
            std::printf("%s,%zu,%.1f,%.1f", name, n, ms, mb);
            for (auto c : {"1_rare", "1_common", "2_mixed", "2_common", "3_mixed"}) std::printf(",%.2f", pct(us[c], 0.5));
            std::printf(",%.2f,%.2f\n", pct(us["all"], 0.5), pct(us["all"], 0.9));
        };
        {
            std::size_t m0 = g_live; auto t0 = Clock::now();
            LinearScan s; for (std::size_t i = 0; i < n; ++i) s.add_document(static_cast<DocId>(i), docs[i]);
            double ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
            double mb = (g_live - m0) / 1e6;
            auto us = run([&](const std::string& q) { return s.search_and(q); }, qs, ans_scan);
            report("linear_scan", ms, mb, us);
        }
        {
            std::size_t m0 = g_live; auto t0 = Clock::now();
            InvertedIndex ix; for (std::size_t i = 0; i < n; ++i) ix.add_document(static_cast<DocId>(i), docs[i]);
            double ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
            double mb = (g_live - m0) / 1e6;
            auto us = run([&](const std::string& q) { return ix.search_and(q); }, qs, ans_index);
            report("inverted_index", ms, mb, us);
        }
        if (ans_scan != ans_index) std::printf("ERROR: designs disagree at n=%zu\n", n);
    }
}
