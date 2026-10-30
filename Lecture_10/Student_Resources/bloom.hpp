// SE618 Lecture 10 — BloomFilter (TODO: add, might_contain) and CountMinSketch (complete).
#pragma once
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace se618 {

inline std::uint64_t mix64(std::uint64_t x) {        // splitmix64 finaliser (see Lecture 2)
    x ^= x >> 30; x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27; x *= 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}
inline std::uint64_t hash_bytes(std::string_view s) {   // FNV-1a, then mixed
    std::uint64_t h = 1469598103934665603ULL;
    for (unsigned char c : s) { h ^= c; h *= 1099511628211ULL; }
    return mix64(h);
}

// Contract
//  - add(x) then might_contain(x) is always true (no false negatives) while the filter is intact.
//  - might_contain(y) for a never-added y is true with probability ~ predicted_fp(m, k, n).
//  - No deletion. Capacity is not enforced: past the planned n the FP rate keeps rising.
//  - Parameters (m, k, seed) are part of the persisted format: two filters merge only if equal.
class BloomFilter {
public:
    BloomFilter(std::size_t m_bits, int k, std::uint64_t seed = 0)
        : m_(m_bits), k_(k), seed_(seed), words_((m_bits + 63) / 64, 0) {
        if (m_bits == 0 || k <= 0) throw std::invalid_argument("m and k must be positive");
    }
    // Plan for n elements at target false-positive rate p.
    static BloomFilter for_capacity(std::size_t n, double p, std::uint64_t seed = 0) {
        double m = -static_cast<double>(n) * std::log(p) / (std::log(2.0) * std::log(2.0));
        int k = std::max(1, static_cast<int>(std::lround(m / static_cast<double>(n) * std::log(2.0))));
        return BloomFilter(static_cast<std::size_t>(std::ceil(m)), k, seed);
    }
    static double predicted_fp(std::size_t m, int k, std::size_t n) {
        return std::pow(1.0 - std::exp(-static_cast<double>(k) * static_cast<double>(n) / static_cast<double>(m)), k);
    }

    // TODO 1: set the k bit positions g_i = (h1 + i * h2) mod m, i = 0..k-1, where
    //   h1 = mix64(key ^ seed_) and h2 = mix64(h1) | 1   (double hashing; h2 odd).
    // Also count the insertion in n_.
    void add(std::uint64_t key) {
        (void)key;
        ++n_;
    }
    // TODO 2: true iff all k positions are set. Must never return false for an added key.
    bool might_contain(std::uint64_t key) const {
        (void)key;
        return false;
    }
    void add(std::string_view s) { add(hash_bytes(s)); }
    bool might_contain(std::string_view s) const { return might_contain(hash_bytes(s)); }

    // Monitoring: the fraction of set bits tells you the CURRENT FP rate (~ fill^k), even if
    // the count of inserted items is unknown or wrong.
    double fill_ratio() const {
        std::size_t ones = 0;
        for (auto w : words_) ones += static_cast<std::size_t>(__builtin_popcountll(w));
        return static_cast<double>(ones) / static_cast<double>(m_);
    }
    double estimated_fp_now() const { return std::pow(fill_ratio(), k_); }

    bool merge(const BloomFilter& o) {                 // union: same parameters required
        if (o.m_ != m_ || o.k_ != k_ || o.seed_ != seed_) return false;
        for (std::size_t i = 0; i < words_.size(); ++i) words_[i] |= o.words_[i];
        n_ += o.n_;
        return true;
    }
    std::size_t bits() const noexcept { return m_; }
    int hashes() const noexcept { return k_; }
    std::size_t inserted() const noexcept { return n_; }
    std::size_t memory_bytes() const noexcept { return words_.size() * 8; }

private:
    void set(std::uint64_t b) { words_[b >> 6] |= std::uint64_t{1} << (b & 63); }
    bool test(std::uint64_t b) const { return (words_[b >> 6] >> (b & 63)) & 1; }
    std::size_t m_;
    int k_;
    std::uint64_t seed_;
    std::vector<std::uint64_t> words_;
    std::size_t n_ = 0;
};

// Count-Min Sketch: frequency estimates for a stream in fixed memory.
// With nonnegative updates, estimate(x) >= true count (never under). Under the standard
// assumptions (pairwise-independent row hashes), width w = ceil(e / eps) and depth
// d = ceil(ln(1 / delta)) give estimate(x) <= true + eps * N with probability >= 1 - delta,
// N = total count added. The mixed seeded hashes used here are a practical choice: measured
// behaviour is empirical evidence, not a proof that those assumptions hold.
class CountMinSketch {
public:
    CountMinSketch(std::size_t width, std::size_t depth, std::uint64_t seed = 0)
        : w_(width), d_(depth), seed_(seed), t_(width * depth, 0) {}
    static CountMinSketch for_error(double eps, double delta, std::uint64_t seed = 0) {
        return CountMinSketch(static_cast<std::size_t>(std::ceil(std::exp(1.0) / eps)),
                              static_cast<std::size_t>(std::ceil(std::log(1.0 / delta))), seed);
    }
    void add(std::uint64_t key, std::uint64_t c = 1) {
        for (std::size_t r = 0; r < d_; ++r) t_[r * w_ + col(key, r)] += c;
        total_ += c;
    }
    std::uint64_t estimate(std::uint64_t key) const {
        std::uint64_t best = UINT64_MAX;
        for (std::size_t r = 0; r < d_; ++r) best = std::min(best, t_[r * w_ + col(key, r)]);
        return best;
    }
    std::uint64_t total() const noexcept { return total_; }
    std::size_t width() const noexcept { return w_; }
    std::size_t depth() const noexcept { return d_; }
    std::size_t memory_bytes() const noexcept { return t_.size() * 8; }
private:
    std::size_t col(std::uint64_t key, std::size_t r) const { return mix64(key ^ (seed_ + 0x9e3779b97f4a7c15ULL * (r + 1))) % w_; }
    std::size_t w_, d_;
    std::uint64_t seed_;
    std::vector<std::uint64_t> t_;
    std::uint64_t total_ = 0;
};

}  // namespace se618
