// SE618 Lecture 3 — eviction policies for hit-rate simulation.
// Each policy models cache *membership* only: access(k) returns true on a hit and
// otherwise admits k (evicting if full). Values are irrelevant to hit rate.
#pragma once
#include <cstddef>
#include <cstdint>
#include <list>
#include <random>
#include <unordered_map>
#include <vector>

namespace se618 {
using Key = std::uint64_t;

class FifoPolicy {  // evicts in insertion order; hits do not change order
public:
    explicit FifoPolicy(std::size_t cap) : cap_(cap) {}
    bool access(Key k) {
        if (pos_.count(k)) return true;
        if (q_.size() == cap_) { pos_.erase(q_.front()); q_.pop_front(); }
        q_.push_back(k); pos_[k] = std::prev(q_.end());
        return false;
    }
private:
    std::size_t cap_;
    std::list<Key> q_;
    std::unordered_map<Key, std::list<Key>::iterator> pos_;
};

// Invariants: map and list hold the same keys; front = most recently used; size <= cap.
class LruPolicy {
public:
    explicit LruPolicy(std::size_t cap) : cap_(cap) {}
    bool access(Key k) {
        auto it = pos_.find(k);
        if (it != pos_.end()) { order_.splice(order_.begin(), order_, it->second); return true; }
        if (order_.size() == cap_) { pos_.erase(order_.back()); order_.pop_back(); }
        order_.push_front(k); pos_[k] = order_.begin();
        return false;
    }
private:
    std::size_t cap_;
    std::list<Key> order_;
    std::unordered_map<Key, std::list<Key>::iterator> pos_;
};

// O(1) LFU: one LRU-ordered list per frequency; min_freq_ tracks the eviction bucket.
// Invariants: every key is in exactly the list of its frequency; min_freq_ is the smallest
// non-empty frequency whenever the cache is non-empty.
class LfuPolicy {
public:
    explicit LfuPolicy(std::size_t cap) : cap_(cap) {}
    bool access(Key k) {
        auto it = entry_.find(k);
        if (it != entry_.end()) { bump(it->second, k); return true; }
        if (entry_.size() == cap_) {
            auto& victims = by_freq_[min_freq_];
            entry_.erase(victims.back());
            victims.pop_back();
            if (victims.empty()) by_freq_.erase(min_freq_);
        }
        auto& l1 = by_freq_[1];
        l1.push_front(k);
        entry_[k] = {1, l1.begin()};
        min_freq_ = 1;
        return false;
    }
private:
    struct Entry { std::uint64_t freq; std::list<Key>::iterator it; };
    void bump(Entry& e, Key k) {
        auto& from = by_freq_[e.freq];
        from.erase(e.it);
        if (from.empty()) { by_freq_.erase(e.freq); if (min_freq_ == e.freq) ++min_freq_; }
        ++e.freq;
        auto& to = by_freq_[e.freq];
        to.push_front(k);
        e.it = to.begin();
    }
    std::size_t cap_;
    std::uint64_t min_freq_ = 0;
    std::unordered_map<Key, Entry> entry_;
    std::unordered_map<std::uint64_t, std::list<Key>> by_freq_;
};

// Segmented LRU: new keys enter a small probationary segment; a second hit promotes them to
// the protected segment. One-time keys (scans) never displace the protected working set.
class SlruPolicy {
public:
    explicit SlruPolicy(std::size_t cap, double protected_share = 0.8)
        : prot_cap_(static_cast<std::size_t>(cap * protected_share)), prob_cap_(cap - prot_cap_) {}
    bool access(Key k) {
        auto it = pos_.find(k);
        if (it != pos_.end()) {
            if (it->second.prot) { prot_.splice(prot_.begin(), prot_, it->second.it); return true; }
            prob_.erase(it->second.it);                       // promote
            if (prot_.size() == prot_cap_) {                  // demote protected LRU to probation MRU
                Key d = prot_.back(); prot_.pop_back();
                prob_.push_front(d); pos_[d] = {false, prob_.begin()};
            }
            prot_.push_front(k); it->second = {true, prot_.begin()};
            return true;
        }
        if (prob_.size() >= prob_cap_) { pos_.erase(prob_.back()); prob_.pop_back(); }
        prob_.push_front(k); pos_[k] = {false, prob_.begin()};
        return false;
    }
private:
    struct Where { bool prot; std::list<Key>::iterator it; };
    std::size_t prot_cap_, prob_cap_;
    std::list<Key> prob_, prot_;
    std::unordered_map<Key, Where> pos_;
};

// Random eviction: no recency or frequency state at all.
class RandomPolicy {
public:
    explicit RandomPolicy(std::size_t cap) : cap_(cap) {}
    bool access(Key k) {
        if (idx_.count(k)) return true;
        if (keys_.size() == cap_) {
            std::size_t v = rng_() % keys_.size();
            idx_.erase(keys_[v]);
            keys_[v] = keys_.back(); idx_[keys_[v]] = v; keys_.pop_back();
        }
        idx_[k] = keys_.size(); keys_.push_back(k);
        return false;
    }
private:
    std::size_t cap_;
    std::vector<Key> keys_;
    std::unordered_map<Key, std::size_t> idx_;
    std::mt19937_64 rng_{1};
};

}  // namespace se618
