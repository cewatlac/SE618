// SE618 Lecture 5 — starter InvertedIndex. Your tasks (see EXERCISE.md):
//   TODO 1: intersect()        — posting-list intersection for AND queries
//   TODO 2: remove_document()  — keep every invariant true after removal
#pragma once
#include <algorithm>
#include <cctype>
#include <iterator>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace se618 {

using DocId = std::uint32_t;

// One analyzer for BOTH indexing and querying (same contract as Lecture 4's normaliser):
// ASCII lower-casing, split on non-alphanumerics, drop a small stop-word list.
inline std::vector<std::string> analyze(std::string_view text) {
    static const std::unordered_set<std::string> stop = {"a", "an", "and", "the", "of", "to", "in", "is", "or", "for", "on"};
    std::vector<std::string> out;
    std::string cur;
    auto flush = [&] { if (!cur.empty() && !stop.count(cur)) out.push_back(cur); cur.clear(); };
    for (char ch : text) {
        unsigned char c = static_cast<unsigned char>(ch);
        if (std::isalnum(c)) cur.push_back(static_cast<char>(std::tolower(c)));
        else flush();
    }
    flush();
    return out;
}

struct Posting { DocId doc; std::uint32_t tf; };   // term frequency within doc

// Contract
//  - add_document: false (and no change) if id already present. Searchable immediately on return.
//  - remove_document: false if absent. After return, id appears in no posting list.
//  - update_document: replace = remove + add; returns false if id absent. Not atomic w.r.t.
//    concurrent readers (single-threaded component).
//  - search_*: result doc ids in ascending order, no duplicates.
// Invariants (check_invariants): every posting refers to a live document; posting lists are
//  strictly increasing by doc id (sorted, no duplicates); forward and inverted views agree;
//  no empty posting lists.
class InvertedIndex {
public:
    bool add_document(DocId id, std::string_view text) {
        if (forward_.count(id)) return false;
        std::unordered_map<std::string, std::uint32_t> tf;
        for (auto& t : analyze(text)) ++tf[t];
        auto& terms = forward_[id];
        for (auto& [term, n] : tf) {
            auto& pl = postings_[term];
            auto it = std::lower_bound(pl.begin(), pl.end(), id, [](const Posting& p, DocId d) { return p.doc < d; });
            pl.insert(it, Posting{id, n});          // O(1) amortised if ids arrive in increasing order
            terms.push_back(term);
        }
        doc_len_[id] = static_cast<std::uint32_t>(tf.size());
        return true;
    }

    bool remove_document(DocId id) {
        // TODO 2: remove `id` from every posting list it appears in, drop empty posting lists,
        // and erase its forward/metadata entries. Return false if `id` is not indexed.
        // Hint: which structure tells you the terms of `id` without scanning the vocabulary?
        (void)id;
        return false;
    }

    bool update_document(DocId id, std::string_view text) {
        if (!remove_document(id)) return false;
        add_document(id, text);
        return true;
    }

    std::vector<DocId> search_and(std::string_view query) const {
        auto terms = analyze(query);
        if (terms.empty()) return {};
        std::vector<const std::vector<Posting>*> lists;
        for (auto& t : terms) {
            auto p = postings_.find(t);
            if (p == postings_.end()) return {};
            lists.push_back(&p->second);
        }
        std::sort(lists.begin(), lists.end(), [](auto* a, auto* b) { return a->size() < b->size(); });  // rarest first
        std::vector<DocId> acc;
        for (auto& p : *lists[0]) acc.push_back(p.doc);
        for (std::size_t i = 1; i < lists.size() && !acc.empty(); ++i) acc = intersect(acc, *lists[i]);
        return acc;
    }

    std::vector<DocId> search_or(std::string_view query) const {
        std::vector<DocId> acc;
        for (auto& t : analyze(query)) {
            auto p = postings_.find(t);
            if (p == postings_.end()) continue;
            std::vector<DocId> merged;
            auto a = acc.begin();
            for (auto& q : p->second) {
                while (a != acc.end() && *a < q.doc) merged.push_back(*a++);
                if (a != acc.end() && *a == q.doc) ++a;
                merged.push_back(q.doc);
            }
            merged.insert(merged.end(), a, acc.end());
            acc.swap(merged);
        }
        return acc;
    }

    // Documents matching all of `must` and none of `must_not`.
    std::vector<DocId> search_and_not(std::string_view must, std::string_view must_not) const {
        auto acc = search_and(must);
        auto excl = search_or(must_not);
        std::vector<DocId> out;
        std::set_difference(acc.begin(), acc.end(), excl.begin(), excl.end(), std::back_inserter(out));
        return out;
    }

    // OR-semantics TF-IDF top-k: score = sum over query terms of (1 + log tf) * log(N / df).
    std::vector<std::pair<DocId, double>> search_ranked(std::string_view query, std::size_t k) const {
        std::unordered_map<DocId, double> score;
        double N = static_cast<double>(forward_.size());
        for (auto& t : analyze(query)) {
            auto p = postings_.find(t);
            if (p == postings_.end()) continue;
            double idf = std::log(N / static_cast<double>(p->second.size()));
            for (auto& q : p->second) score[q.doc] += (1.0 + std::log(static_cast<double>(q.tf))) * idf;
        }
        std::vector<std::pair<DocId, double>> r(score.begin(), score.end());
        auto better = [](auto& a, auto& b) { return a.second != b.second ? a.second > b.second : a.first < b.first; };
        std::size_t m = std::min(k, r.size());
        std::partial_sort(r.begin(), r.begin() + static_cast<long>(m), r.end(), better);
        r.resize(m);
        return r;
    }

    std::size_t document_frequency(std::string_view term) const {
        auto t = analyze(term);
        if (t.size() != 1) return 0;
        auto p = postings_.find(t[0]);
        return p == postings_.end() ? 0 : p->second.size();
    }
    std::size_t size() const noexcept { return forward_.size(); }
    std::size_t vocabulary_size() const noexcept { return postings_.size(); }

    bool check_invariants() const {
        std::size_t total = 0;
        for (auto& [term, pl] : postings_) {
            if (pl.empty()) return false;
            for (std::size_t i = 0; i < pl.size(); ++i) {
                if (i && pl[i - 1].doc >= pl[i].doc) return false;           // sorted, no duplicates
                auto f = forward_.find(pl[i].doc);
                if (f == forward_.end()) return false;                       // no dangling postings
                if (std::find(f->second.begin(), f->second.end(), term) == f->second.end()) return false;
            }
            total += pl.size();
        }
        std::size_t fwd = 0;
        for (auto& [d, terms] : forward_) fwd += terms.size();
        return total == fwd && doc_len_.size() == forward_.size();
    }

private:
    static std::vector<DocId> intersect(const std::vector<DocId>& a, const std::vector<Posting>& b) {
        // TODO 1: return the doc ids present in both inputs, ascending, without duplicates.
        // Both inputs are sorted by doc id. Target: O(|a| + |b|), no hashing, no sorting.
        (void)a; (void)b;
        return {};
    }

    std::unordered_map<std::string, std::vector<Posting>> postings_;   // inverted: term -> sorted postings
    std::unordered_map<DocId, std::vector<std::string>> forward_;      // forward: doc -> its distinct terms
    std::unordered_map<DocId, std::uint32_t> doc_len_;                 // distinct-term count (ranking metadata)
};

// Baseline: re-analyses every stored document on every query. No build cost beyond storing text.
class LinearScan {
public:
    bool add_document(DocId id, std::string_view text) {
        auto it = std::lower_bound(docs_.begin(), docs_.end(), id, [](auto& p, DocId d) { return p.first < d; });
        if (it != docs_.end() && it->first == id) return false;
        docs_.insert(it, {id, std::string(text)});
        return true;
    }
    std::vector<DocId> search_and(std::string_view query) const {
        auto q = analyze(query);
        std::vector<DocId> out;
        if (q.empty()) return out;
        for (auto& [id, text] : docs_) {
            auto toks = analyze(text);
            bool all = true;
            for (auto& t : q) if (std::find(toks.begin(), toks.end(), t) == toks.end()) { all = false; break; }
            if (all) out.push_back(id);
        }
        return out;
    }
    std::size_t size() const noexcept { return docs_.size(); }
private:
    std::vector<std::pair<DocId, std::string>> docs_;
};

}  // namespace se618
