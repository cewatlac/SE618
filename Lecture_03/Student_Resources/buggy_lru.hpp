// SE618 Lecture 3 — debugging activity. A get/put LRU cache with two defects.
// Contract (intended):
//   - capacity > 0; size() <= capacity() after every call.
//   - get(k): hit returns the value and makes k most-recently-used; miss returns nullopt.
//   - put(k, v): inserts or overwrites; k becomes most-recently-used; if a NEW key
//     would exceed capacity, the least-recently-used key is evicted first.
//   - stats(): hits, misses, evictions are exact.
#pragma once
#include <cstddef>
#include <list>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace se618 {

struct CacheStats { std::size_t hits = 0, misses = 0, evictions = 0; };

template <class K, class V>
class LruCache {
public:
    explicit LruCache(std::size_t capacity) : cap_(capacity) {
        if (cap_ == 0) throw std::invalid_argument("capacity must be > 0");
    }

    std::optional<V> get(const K& k) {
        auto it = index_.find(k);
        if (it == index_.end()) { ++stats_.misses; return std::nullopt; }
        ++stats_.hits;
        return it->second->second;
    }

    void put(const K& k, V v) {
        if (items_.size() == cap_) {
            index_.erase(items_.back().first);
            items_.pop_back();
            ++stats_.evictions;
        }
        items_.emplace_front(k, std::move(v));
        index_[k] = items_.begin();
    }

    std::size_t size() const noexcept { return index_.size(); }
    std::size_t capacity() const noexcept { return cap_; }
    const CacheStats& stats() const noexcept { return stats_; }

private:
    std::size_t cap_;
    std::list<std::pair<K, V>> items_;   // front = most recently used
    std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> index_;
    CacheStats stats_;
};

}  // namespace se618
