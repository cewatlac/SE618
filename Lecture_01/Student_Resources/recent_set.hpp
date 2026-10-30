// SE618 Lecture 1 — RecentSet as developed on the Part 3 slides, plus the baseline oracle.
// The exercise (time-window design) is in EXERCISE_RecentSet.md; its solution is released after class.
// C++20, header-only. Build tests: g++ -std=c++20 -O2 -Wall -Wextra recent_set_test.cpp
#pragma once
#include <cassert>
#include <cstddef>
#include <functional>
#include <list>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace se618 {

// Result type instead of bool: callers can distinguish "new" from "duplicate"
// and learn which key was evicted (needed for metrics and for debugging data loss).
template <class Key>
struct AddOutcome {
    bool inserted;               // false => key was already present (a duplicate)
    std::optional<Key> evicted;  // set when inserting forced the oldest key out
};

// Count-bounded: remembers the last `capacity` DISTINCT keys by first-insertion order.
// Contract:
//   - capacity > 0, else std::invalid_argument.
//   - add(k) on a present key does NOT refresh its position (dedup semantics, not LRU).
//   - contains() is const and has no side effects.
//   - Invariant: size() <= capacity(); order_ and index_ hold the same key set;
//     index_[k] points at k's node in order_; order_.front() is the oldest.
//   - Not thread-safe. Strong exception guarantee for add() (see comment inside).
template <class Key, class Hash = std::hash<Key>, class Eq = std::equal_to<Key>>
class RecentSet {
public:
    explicit RecentSet(std::size_t capacity) : cap_(capacity) {
        if (cap_ == 0) throw std::invalid_argument("RecentSet capacity must be > 0");
        index_.reserve(cap_);
    }

    AddOutcome<Key> add(const Key& k) {
        if (index_.find(k) != index_.end()) return {false, std::nullopt};
        // Insert into the list first; if the map insert throws, undo the list insert.
        order_.push_back(k);
        try {
            index_.emplace(k, std::prev(order_.end()));
        } catch (...) {
            order_.pop_back();
            throw;
        }
        std::optional<Key> ev;
        if (order_.size() > cap_) {
            ev = std::move(order_.front());
            index_.erase(*ev);
            order_.pop_front();
        }
        return {true, std::move(ev)};
    }

    bool contains(const Key& k) const { return index_.find(k) != index_.end(); }

    bool erase(const Key& k) {
        auto it = index_.find(k);
        if (it == index_.end()) return false;
        order_.erase(it->second);
        index_.erase(it);
        return true;
    }

    std::size_t size() const noexcept { return index_.size(); }
    std::size_t capacity() const noexcept { return cap_; }
    void clear() noexcept { index_.clear(); order_.clear(); }

    // O(n); for tests and debug builds only.
    bool check_invariants() const {
        if (order_.size() != index_.size() || size() > cap_) return false;
        for (auto it = order_.begin(); it != order_.end(); ++it) {
            auto f = index_.find(*it);
            if (f == index_.end() || f->second != it) return false;
        }
        return true;
    }

private:
    std::size_t cap_;
    std::list<Key> order_;
    std::unordered_map<Key, typename std::list<Key>::iterator, Hash, Eq> index_;
};

// Baseline: same contract, linear scan. Kept as the reference model for differential tests
// and as the benchmark baseline. Correctness is easy to see; that is its whole purpose.
template <class Key>
class RecentSetBaseline {
public:
    explicit RecentSetBaseline(std::size_t capacity) : cap_(capacity) {
        if (cap_ == 0) throw std::invalid_argument("capacity must be > 0");
    }
    AddOutcome<Key> add(const Key& k) {
        if (contains(k)) return {false, std::nullopt};
        v_.push_back(k);
        std::optional<Key> ev;
        if (v_.size() > cap_) { ev = v_.front(); v_.erase(v_.begin()); }
        return {true, ev};
    }
    bool contains(const Key& k) const {
        for (const auto& x : v_) if (x == k) return true;
        return false;
    }
    bool erase(const Key& k) {
        for (auto it = v_.begin(); it != v_.end(); ++it)
            if (*it == k) { v_.erase(it); return true; }
        return false;
    }
    std::size_t size() const noexcept { return v_.size(); }
private:
    std::size_t cap_;
    std::vector<Key> v_;
};

}  // namespace se618
