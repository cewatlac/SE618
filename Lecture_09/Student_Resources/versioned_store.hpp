// SE618 Lecture 9 — three versioned stores behind one contract.
// Exercise (see EXERCISE.md): one store violates the snapshot contract under a common workload.
// Find it with versioned_test.cpp, explain the mechanism, and fix it.
//
// Versioned array of N int64 slots (e.g. settings, document paragraphs, feature flags).
// Contract (all stores):
//   - Version 0 exists at construction: all slots 0.
//   - commit(parent, changes) creates a NEW version (ids increase: 1, 2, ...). parent must be live.
//     changes = (slot, value) pairs; later pairs for the same slot win.
//   - A version never changes after creation (snapshot semantics), whatever happens later.
//   - get(v, i) throws std::out_of_range if v is unknown or released, or i >= N.
//   - release(v) makes v inaccessible and lets its memory be reclaimed. Returns false (and does
//     nothing) where the representation cannot release v on its own (DeltaChain: v has live children).
//   - Branching is allowed: several versions may share a parent.
//   - Not thread-safe for concurrent commits; reads of existing versions never observe changes.
#pragma once
#include <algorithm>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace se618 {

using Version = std::uint32_t;
using Value = std::int64_t;
using Change = std::pair<std::uint32_t, Value>;

inline std::vector<Change> normalise(std::vector<Change> c, std::size_t n) {   // sort by slot, last write wins
    for (auto& [i, v] : c) if (i >= n) throw std::out_of_range("slot out of range");
    std::stable_sort(c.begin(), c.end(), [](auto& a, auto& b) { return a.first < b.first; });
    std::vector<Change> out;
    for (auto& ch : c) { if (!out.empty() && out.back().first == ch.first) out.back() = ch; else out.push_back(ch); }
    out.shrink_to_fit();                          // deltas are stored: do not keep growth slack
    return out;
}

// A. Full copy per version: O(N) commit and memory per version, O(1) read of any version.
class FullCopyStore {
public:
    explicit FullCopyStore(std::size_t n) : n_(n) { v_.push_back(std::make_shared<const std::vector<Value>>(n, 0)); }
    Version commit(Version parent, std::vector<Change> changes) {
        auto next = std::make_shared<std::vector<Value>>(*live(parent));        // the full copy
        for (auto& [i, x] : normalise(std::move(changes), n_)) (*next)[i] = x;
        v_.push_back(std::move(next));
        return static_cast<Version>(v_.size() - 1);
    }
    Value get(Version v, std::size_t i) const { if (i >= n_) throw std::out_of_range("slot"); return (*live(v))[i]; }
    bool release(Version v) { live(v); v_[v].reset(); return true; }
    std::size_t size() const noexcept { return n_; }
private:
    const std::shared_ptr<const std::vector<Value>>& live(Version v) const {
        if (v >= v_.size() || !v_[v]) throw std::out_of_range("unknown or released version");
        return v_[v];
    }
    std::size_t n_;
    std::vector<std::shared_ptr<const std::vector<Value>>> v_;
};

// B. Delta chain: each version stores only its changes + a parent link. Every `every`-th version
// in a chain also stores a full checkpoint, bounding the walk for reads of old versions.
// every = 0 means "never checkpoint" (only version 0 is a full base).
class DeltaChainStore {
public:
    DeltaChainStore(std::size_t n, std::uint32_t every) : n_(n), every_(every) {
        nodes_.push_back(Node{kNone, 0, {}, std::make_shared<const std::vector<Value>>(n, 0), 0, true, false});
    }
    Version commit(Version parent, std::vector<Change> changes) {
        const Node& p = live(parent);
        Node node{parent, p.depth + 1, normalise(std::move(changes), n_), nullptr, 0, true, false};
        Version id = static_cast<Version>(nodes_.size());
        nodes_.push_back(std::move(node));
        ++nodes_[parent].children;
        if (every_ && nodes_[id].depth % every_ == 0) checkpoint(id);
        return id;
    }
    Value get(Version v, std::size_t i) const {
        if (i >= n_) throw std::out_of_range("slot");
        live(v);
        for (Version cur = v;; cur = nodes_[cur].parent) {           // walk towards the base
            const Node& n = nodes_[cur];
            if (n.full) return (*n.full)[i];
            auto it = std::lower_bound(n.delta.begin(), n.delta.end(), static_cast<std::uint32_t>(i),
                                       [](const Change& c, std::uint32_t k) { return c.first < k; });
            if (it != n.delta.end() && it->first == i) return it->second;
        }
    }
    // A version with live children cannot be dropped: its delta is needed to reconstruct them.
    bool release(Version v) {
        Node& n = const_cast<Node&>(live(v));
        if (n.children) return false;
        Version p = n.parent;
        drop(v);
        if (p != kNone) drop_if_unused(p);           // cascade: retired ancestors nobody needs any more
        return true;
    }
    // Compaction: materialise v as a full base, cut its link to history, and drop every ancestor
    // that no other live version needs. Cost O(N + chain length). Returns versions reclaimed.
    std::size_t compact(Version v) {
        checkpoint(v);
        Version p = nodes_[v].parent;
        nodes_[v].parent = kNone;
        std::size_t reclaimed = 0;
        if (p != kNone) { --nodes_[p].children; reclaimed = drop_if_unused(p); }
        return reclaimed;
    }
    std::size_t size() const noexcept { return n_; }
private:
    static constexpr Version kNone = 0xFFFFFFFFu;
    struct Node {
        Version parent; std::uint32_t depth; std::vector<Change> delta;
        std::shared_ptr<const std::vector<Value>> full; std::uint32_t children; bool alive; bool released;
    };
    const Node& live(Version v) const {
        if (v >= nodes_.size() || !nodes_[v].alive) throw std::out_of_range("unknown or released version");
        return nodes_[v];
    }
    void checkpoint(Version v) {                    // O(N + total size of deltas on the path)
        std::vector<Version> path;
        Version cur = v;
        while (!nodes_[cur].full) { path.push_back(cur); cur = nodes_[cur].parent; }
        auto arr = std::make_shared<std::vector<Value>>(*nodes_[cur].full);
        for (auto it = path.rbegin(); it != path.rend(); ++it)
            for (auto& [i, x] : nodes_[*it].delta) (*arr)[i] = x;
        nodes_[v].full = std::move(arr);
        nodes_[v].delta.clear(); nodes_[v].delta.shrink_to_fit();
    }
    void drop(Version v) {
        Node& n = nodes_[v];
        n.alive = false; n.delta.clear(); n.delta.shrink_to_fit(); n.full.reset();
        if (n.parent != kNone) { --nodes_[n.parent].children; }
    }
    // An ancestor that is only an internal link (released by the user earlier) can be reclaimed.
    // Here: ancestors are reclaimed only if they have no remaining children AND were released.
    std::size_t drop_if_unused(Version v) {
        std::size_t k = 0;
        while (v != kNone && nodes_[v].children == 0 && nodes_[v].released) {
            Version p = nodes_[v].parent;
            drop(v); ++k; v = p;
        }
        return k;
    }
public:
    // Mark a version as no longer needed by users, even if it still has children; its storage is
    // reclaimed later by compact() once no descendant depends on it.
    bool retire(Version v) { Node& n = const_cast<Node&>(live(v)); n.released = true; return true; }
private:
    std::size_t n_;
    std::uint32_t every_;
    std::vector<Node> nodes_;
};

// C. Path copying (structural sharing): a 32-ary radix tree over the N slots. A commit copies
// only the nodes on the paths to the changed slots; everything else is shared with the parent.
// Reclamation is by reference counting: releasing a version frees nodes no other version shares.
class PathCopyStore {
    struct Node { std::vector<std::shared_ptr<const Node>> kids; std::vector<Value> vals; };
    using Ptr = std::shared_ptr<const Node>;
public:
    explicit PathCopyStore(std::size_t n) : n_(n) {
        while ((std::size_t{32} << (5 * levels_)) < n) ++levels_;          // levels_ inner levels above the leaves
        roots_.push_back(build(levels_));
    }
    Version commit(Version parent, std::vector<Change> changes) {
        auto c = normalise(std::move(changes), n_);
        const Ptr& base = live(parent);
        if (parent + 1 == roots_.size() && !c.empty()) {
            // Fast path: committing on top of the newest version is the common case, and path
            // copying costs allocations. Update the newest tree in place and share its root.
            for (auto& [i, x] : c) {
                Node* n = const_cast<Node*>(base.get());
                for (int lvl = levels_; lvl > 0; --lvl) n = const_cast<Node*>(n->kids[(i >> (5 * lvl)) & 31].get());
                n->vals[i & 31] = x;
            }
            roots_.push_back(base);
            return static_cast<Version>(roots_.size() - 1);
        }
        roots_.push_back(c.empty() ? base : apply(base, levels_, c.data(), c.data() + c.size()));
        return static_cast<Version>(roots_.size() - 1);
    }
    Value get(Version v, std::size_t i) const {
        if (i >= n_) throw std::out_of_range("slot");
        const Node* n = live(v).get();
        for (int lvl = levels_; lvl > 0; --lvl) n = n->kids[(i >> (5 * lvl)) & 31].get();
        return n->vals[i & 31];
    }
    bool release(Version v) { live(v); roots_[v].reset(); return true; }
    std::size_t size() const noexcept { return n_; }
private:
    const Ptr& live(Version v) const {
        if (v >= roots_.size() || !roots_[v]) throw std::out_of_range("unknown or released version");
        return roots_[v];
    }
    static Ptr build(int lvl) {
        auto n = std::make_shared<Node>();
        if (lvl == 0) n->vals.assign(32, 0);
        else { auto child = build(lvl - 1); n->kids.assign(32, child); }   // all-zero subtrees shared
        return n;
    }
    // Copy this node once, then recurse into each child that has changes. Old nodes are never touched.
    static Ptr apply(const Ptr& node, int lvl, const Change* b, const Change* e) {
        auto copy = std::make_shared<Node>(*node);
        if (lvl == 0) { for (auto p = b; p != e; ++p) copy->vals[p->first & 31] = p->second; return copy; }
        for (auto p = b; p != e;) {
            std::uint32_t idx = (p->first >> (5 * lvl)) & 31;
            auto q = p;
            while (q != e && ((q->first >> (5 * lvl)) & 31) == idx) ++q;
            copy->kids[idx] = apply(copy->kids[idx], lvl - 1, p, q);
            p = q;
        }
        return copy;
    }
    std::size_t n_;
    int levels_ = 0;
    std::vector<Ptr> roots_;
};

}  // namespace se618
