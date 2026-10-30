// SE618 Lecture 4 — three autocomplete architectures, same contract, measured.
// g++ -std=c++20 -O2 autocomplete_bench.cpp -o ac && ./ac [num_terms]
//
// Contract: topk(prefix, k) returns up to k terms starting with `prefix`, ordered by
// score descending, ties broken by term ascending (deterministic). Terms are already
// normalised (lowercase ASCII here); normalisation is the caller's job in this demo.
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <new>
#include <queue>
#include <random>
#include <string>
#include <string_view>
#include <vector>

// ---- live-heap accounting: replace global new/delete to count bytes ----
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wmismatched-new-delete"  // new/delete below are a matched pair
#endif
static std::size_t g_live = 0;
void* operator new(std::size_t n) {
    auto* p = static_cast<std::size_t*>(std::malloc(n + 16));
    if (!p) throw std::bad_alloc();
    *p = n; g_live += n;
    return reinterpret_cast<char*>(p) + 16;
}
void operator delete(void* q) noexcept {
    if (!q) return;
    auto* p = reinterpret_cast<std::size_t*>(static_cast<char*>(q) - 16);
    g_live -= *p; std::free(p);
}
void operator delete(void* q, std::size_t) noexcept { operator delete(q); }
// The nothrow forms must use the same header layout (std::stable_sort, for example, uses them).
void* operator new(std::size_t n, const std::nothrow_t&) noexcept { try { return operator new(n); } catch (...) { return nullptr; } }
void operator delete(void* q, const std::nothrow_t&) noexcept { operator delete(q); }

using Id = std::uint32_t;
struct Vocab { std::vector<std::string> term; std::vector<std::uint32_t> score; };  // sorted by term

// Order: score desc, then id asc (ids follow lexical order).
struct Better {
    const Vocab* v;
    bool operator()(Id a, Id b) const { return v->score[a] != v->score[b] ? v->score[a] > v->score[b] : a < b; }
};
static void push_topk(std::vector<Id>& heap, Id id, std::size_t k, const Better& better) {
    if (heap.size() < k) { heap.push_back(id); std::push_heap(heap.begin(), heap.end(), better); }
    else if (better(id, heap.front())) { std::pop_heap(heap.begin(), heap.end(), better); heap.back() = id; std::push_heap(heap.begin(), heap.end(), better); }
}
static std::vector<Id> finish(std::vector<Id> heap, const Better& better) { std::sort(heap.begin(), heap.end(), better); return heap; }

// ---- Design A: sorted array + binary search + scan the matching range ----
struct SortedArray {
    const Vocab& v;
    explicit SortedArray(const Vocab& voc) : v(voc) {}
    std::vector<Id> topk(std::string_view p, std::size_t k) const {
        Better b{&v};
        auto lo = std::lower_bound(v.term.begin(), v.term.end(), p, [](const std::string& t, std::string_view x) { return std::string_view(t) < x; });
        std::vector<Id> heap;
        for (auto it = lo; it != v.term.end() && it->compare(0, p.size(), p) == 0; ++it)
            push_topk(heap, static_cast<Id>(it - v.term.begin()), k, b);
        return finish(std::move(heap), b);
    }
};

// ---- Design B: pointer trie, ordered child map, DFS the subtree per query ----
struct TrieMap {
    struct Node { std::map<char, std::unique_ptr<Node>> kids; Id id = UINT32_MAX; };
    const Vocab& v; Node root;
    explicit TrieMap(const Vocab& voc) : v(voc) {
        for (Id i = 0; i < v.term.size(); ++i) {
            Node* n = &root;
            for (char c : v.term[i]) { auto& ch = n->kids[c]; if (!ch) ch = std::make_unique<Node>(); n = ch.get(); }
            n->id = i;                                   // terminal state
        }
    }
    const Node* walk(std::string_view p) const {
        const Node* n = &root;
        for (char c : p) { auto it = n->kids.find(c); if (it == n->kids.end()) return nullptr; n = it->second.get(); }
        return n;
    }
    std::vector<Id> topk(std::string_view p, std::size_t k) const {
        Better b{&v}; std::vector<Id> heap;
        const Node* start = walk(p);
        if (!start) return {};
        std::vector<const Node*> stack{start};
        while (!stack.empty()) {
            const Node* n = stack.back(); stack.pop_back();
            if (n->id != UINT32_MAX) push_topk(heap, n->id, k, b);
            for (auto& [c, ch] : n->kids) stack.push_back(ch.get());
        }
        return finish(std::move(heap), b);
    }
};

// ---- Design C: trie with precomputed top-k per node (query = walk + copy) ----
struct TrieTopK {
    struct Node { std::map<char, std::unique_ptr<Node>> kids; std::vector<Id> best; };
    const Vocab& v; Node root; std::size_t K;
    TrieTopK(const Vocab& voc, std::size_t k) : v(voc), K(k) {
        for (Id i = 0; i < v.term.size(); ++i) {
            Node* n = &root;
            for (char c : v.term[i]) { auto& ch = n->kids[c]; if (!ch) ch = std::make_unique<Node>(); n = ch.get(); }
            n->best.push_back(i);
        }
        fill(root);
    }
    void fill(Node& n) {                                  // post-order merge of children's lists
        Better b{&v};
        for (auto& [c, ch] : n.kids) { fill(*ch); n.best.insert(n.best.end(), ch->best.begin(), ch->best.end()); }
        std::sort(n.best.begin(), n.best.end(), b);
        if (n.best.size() > K) n.best.resize(K);
        n.best.shrink_to_fit();
    }
    std::vector<Id> topk(std::string_view p, std::size_t k) const {
        const Node* n = &root;
        for (char c : p) { auto it = n->kids.find(c); if (it == n->kids.end()) return {}; n = it->second.get(); }
        return std::vector<Id>(n->best.begin(), n->best.begin() + static_cast<long>(std::min(k, n->best.size())));
    }
};

// ---- synthetic vocabulary: syllable words with Zipf-like scores ----
static Vocab make_vocab(std::size_t n) {
    static const char* syl[] = {"ka","to","ri","an","el","mo","sa","ne","li","or","tu","ba","de","mi","pro","con","ter","ing","ex","al","ph","st","qu","ve","zo","ch"};
    std::mt19937_64 g(4);
    std::vector<std::string> w; w.reserve(n * 2);
    while (w.size() < n * 2) {
        std::string s; int parts = 2 + static_cast<int>(g() % 4);
        for (int i = 0; i < parts; ++i) s += syl[g() % 26];
        w.push_back(s);
    }
    std::sort(w.begin(), w.end()); w.erase(std::unique(w.begin(), w.end()), w.end());
    std::shuffle(w.begin(), w.end(), g); w.resize(std::min(n, w.size()));
    std::vector<std::pair<std::string, std::uint32_t>> ts;           // score ~ 1e6 / rank
    for (std::size_t r = 0; r < w.size(); ++r) ts.emplace_back(w[r], static_cast<std::uint32_t>(1'000'000 / (r + 1)));
    std::sort(ts.begin(), ts.end());
    Vocab v;
    for (auto& [t, s] : ts) { v.term.push_back(t); v.score.push_back(s); }
    return v;
}

template <class D> static double us_per_query(const D& d, const std::vector<std::string>& qs, std::size_t k) {
    std::size_t sink = 0; double best = 1e18;
    for (int r = 0; r < 3; ++r) {
        auto a = std::chrono::steady_clock::now();
        for (auto& q : qs) sink += d.topk(q, k).size();
        auto b = std::chrono::steady_clock::now();
        best = std::min(best, std::chrono::duration<double, std::micro>(b - a).count() / qs.size());
    }
    if (sink == 0) std::puts("");
    return best;
}

int main(int argc, char** argv) {
    std::size_t n = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 200000;
    const std::size_t K = 10;
    std::size_t before = g_live;
    Vocab v = make_vocab(n);
    std::size_t vocab_bytes = g_live - before;

    // Query prefixes: taken from score-weighted terms (popular terms are typed more).
    std::mt19937_64 g(9);
    std::discrete_distribution<std::size_t> pick(v.score.begin(), v.score.end());
    std::vector<std::vector<std::string>> byLen(6);
    for (std::size_t L : {1, 2, 3, 5}) for (int i = 0; i < 2000; ++i) { auto& t = v.term[pick(g)]; byLen[L].push_back(t.substr(0, std::min(L, t.size()))); }

    std::printf("terms=%zu  vocabulary strings=%.1f MB\n", v.term.size(), vocab_bytes / 1e6);
    std::printf("design,build_ms,extra_MB,bytes_per_term,us_len1,us_len2,us_len3,us_len5\n");
    auto report = [&](const char* name, auto make) {
        std::size_t b0 = g_live; auto t0 = std::chrono::steady_clock::now();
        auto d = make();
        double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
        double mb = (g_live - b0) / 1e6;
        std::printf("%s,%.0f,%.1f,%.0f", name, ms, mb, (g_live - b0) / double(v.term.size()));
        for (std::size_t L : {1, 2, 3, 5}) std::printf(",%.2f", us_per_query(*d, byLen[L], K));
        std::printf("\n");
        return d;
    };
    auto A = report("sorted_array", [&] { return std::make_unique<SortedArray>(v); });
    auto B = report("trie_map_dfs", [&] { return std::make_unique<TrieMap>(v); });
    auto C = report("trie_cached_topk", [&] { return std::make_unique<TrieTopK>(v, K); });

    std::size_t mismatches = 0;                          // differential check: identical answers
    for (auto& qs : byLen) for (auto& q : qs) { auto a = A->topk(q, K); if (a != B->topk(q, K) || a != C->topk(q, K)) ++mismatches; }
    std::printf("differential mismatches: %zu\n", mismatches);
    return mismatches != 0;
}
