// SE618 Lecture 11 — two thread-safe counter maps with the same contract.
// Contract: get(k) returns the current count (0 if absent); increment(k) atomically adds 1;
// put(k, v) sets the count. Each call is linearizable on its own. Sequences of calls are NOT
// atomic: get() followed by put() can lose concurrent updates (see concurrency_test.cpp).
#pragma once
#include <array>
#include <cstdint>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

namespace se618 {

// A. One lock for everything: simple, obviously correct, serialises all threads.
class GlobalLockMap {
public:
    std::uint64_t get(std::uint64_t k) const {
        std::lock_guard<std::mutex> g(m_);
        auto it = map_.find(k);
        return it == map_.end() ? 0 : it->second;
    }
    void increment(std::uint64_t k) { std::lock_guard<std::mutex> g(m_); ++map_[k]; }
    void put(std::uint64_t k, std::uint64_t v) { std::lock_guard<std::mutex> g(m_); map_[k] = v; }
    std::uint64_t total() const {
        std::lock_guard<std::mutex> g(m_);
        std::uint64_t s = 0; for (auto& [k, v] : map_) s += v; return s;
    }
private:
    mutable std::mutex m_;
    std::unordered_map<std::uint64_t, std::uint64_t> map_;
};

// A'. One reader-writer lock: readers proceed in parallel, writers are exclusive.
class GlobalRwLockMap {
public:
    std::uint64_t get(std::uint64_t k) const {
        std::shared_lock<std::shared_mutex> g(m_);
        auto it = map_.find(k);
        return it == map_.end() ? 0 : it->second;
    }
    void increment(std::uint64_t k) { std::unique_lock<std::shared_mutex> g(m_); ++map_[k]; }
    void put(std::uint64_t k, std::uint64_t v) { std::unique_lock<std::shared_mutex> g(m_); map_[k] = v; }
private:
    mutable std::shared_mutex m_;
    std::unordered_map<std::uint64_t, std::uint64_t> map_;
};

// B. Lock striping / sharding: S independent maps, each with its own lock, chosen by hash.
// Each shard is aligned to its own cache line(s) so that locks of different shards do not
// share a cache line (false sharing). Operations on different shards proceed in parallel;
// operations on the SAME shard (e.g. a hot key) still serialise.
template <std::size_t S = 64>
class ShardedMap {
public:
    std::uint64_t get(std::uint64_t k) const {
        const Shard& s = shard(k);
        std::lock_guard<std::mutex> g(s.m);
        auto it = s.map.find(k);
        return it == s.map.end() ? 0 : it->second;
    }
    void increment(std::uint64_t k) { Shard& s = shard(k); std::lock_guard<std::mutex> g(s.m); ++s.map[k]; }
    void put(std::uint64_t k, std::uint64_t v) { Shard& s = shard(k); std::lock_guard<std::mutex> g(s.m); s.map[k] = v; }
    // Not a consistent snapshot: shards are summed one after another while writers may continue.
    std::uint64_t total() const {
        std::uint64_t t = 0;
        for (auto& s : shards_) { std::lock_guard<std::mutex> g(s.m); for (auto& [k, v] : s.map) t += v; }
        return t;
    }
private:
    struct alignas(64) Shard { mutable std::mutex m; std::unordered_map<std::uint64_t, std::uint64_t> map; };
    static std::size_t index(std::uint64_t k) {
        k ^= k >> 33; k *= 0xff51afd7ed558ccdULL; k ^= k >> 33;           // mix: shard choice must not follow key patterns
        return static_cast<std::size_t>(k % S);
    }
    Shard& shard(std::uint64_t k) { return shards_[index(k)]; }
    const Shard& shard(std::uint64_t k) const { return shards_[index(k)]; }
    std::array<Shard, S> shards_;
};

}  // namespace se618
