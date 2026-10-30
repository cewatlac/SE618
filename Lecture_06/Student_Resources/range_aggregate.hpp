// SE618 Lecture 6 — one contract, four representations.
//
// RangeSum contract (all implementations):
//   - Indices are 0-based; ranges are HALF-OPEN [l, r). Empty range (l == r) sums to 0.
//   - query(l, r) throws std::out_of_range unless 0 <= l <= r <= size().
//   - set(i, v) ASSIGNS a[i] = v (it does not add); throws std::out_of_range unless i < size().
//   - Values and sums are int64_t; the caller guarantees sums fit (overflow is not detected).
//   - Not thread-safe; concurrent query + set requires external synchronisation.
#pragma once
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace se618 {

using Value = std::int64_t;

inline void check_range(std::size_t l, std::size_t r, std::size_t n) {
    if (l > r || r > n) throw std::out_of_range("range [l, r) invalid");
}
inline void check_index(std::size_t i, std::size_t n) {
    if (i >= n) throw std::out_of_range("index out of range");
}

// query O(r - l), set O(1), memory n values.
class NaiveRange {
public:
    explicit NaiveRange(std::vector<Value> a) : a_(std::move(a)) {}
    Value query(std::size_t l, std::size_t r) const {
        check_range(l, r, a_.size());
        Value s = 0;
        for (std::size_t i = l; i < r; ++i) s += a_[i];
        return s;
    }
    void set(std::size_t i, Value v) { check_index(i, a_.size()); a_[i] = v; }
    std::size_t size() const noexcept { return a_.size(); }
    std::size_t memory_bytes() const noexcept { return a_.capacity() * sizeof(Value); }
private:
    std::vector<Value> a_;
};

// query O(1), set O(n) (rewrites the suffix), memory n + 1 values.
class PrefixSumRange {
public:
    explicit PrefixSumRange(const std::vector<Value>& a) : p_(a.size() + 1, 0) {
        for (std::size_t i = 0; i < a.size(); ++i) p_[i + 1] = p_[i] + a[i];
    }
    Value query(std::size_t l, std::size_t r) const { check_range(l, r, size()); return p_[r] - p_[l]; }
    void set(std::size_t i, Value v) {
        check_index(i, size());
        Value delta = v - (p_[i + 1] - p_[i]);
        for (std::size_t j = i + 1; j < p_.size(); ++j) p_[j] += delta;
    }
    std::size_t size() const noexcept { return p_.size() - 1; }
    std::size_t memory_bytes() const noexcept { return p_.capacity() * sizeof(Value); }
private:
    std::vector<Value> p_;   // p_[k] = a[0] + ... + a[k-1]
};

// Fenwick / binary indexed tree: query O(log n), set O(log n), memory 2n values
// (the tree plus a copy of a[], because set() assigns and Fenwick natively only adds).
class FenwickRange {
public:
    explicit FenwickRange(const std::vector<Value>& a) : a_(a), t_(a.size() + 1, 0) {
        for (std::size_t i = 0; i < a.size(); ++i) {                   // O(n) construction
            t_[i + 1] += a[i];
            std::size_t j = (i + 1) + ((i + 1) & (~(i + 1) + 1));
            if (j < t_.size()) t_[j] += t_[i + 1];
        }
    }
    Value query(std::size_t l, std::size_t r) const { check_range(l, r, size()); return prefix(r) - prefix(l); }
    void set(std::size_t i, Value v) {
        check_index(i, size());
        Value delta = v - a_[i];
        a_[i] = v;
        for (std::size_t k = i + 1; k < t_.size(); k += k & (~k + 1)) t_[k] += delta;
    }
    std::size_t size() const noexcept { return a_.size(); }
    std::size_t memory_bytes() const noexcept { return (a_.capacity() + t_.capacity()) * sizeof(Value); }
private:
    Value prefix(std::size_t r) const {                                 // sum of a[0, r)
        Value s = 0;
        for (std::size_t k = r; k > 0; k -= k & (~k + 1)) s += t_[k];
        return s;
    }
    std::vector<Value> a_, t_;
};

// Iterative segment tree over any associative operation with an identity (a monoid):
// query O(log n), set O(log n), memory 2n values. Generalises to min/max/gcd. Fenwick trees
// and prefix sums efficiently support invertible prefix-style aggregates (sum, xor); they are not
// a general solution for arbitrary dynamic range-min/max.
struct SumOp { static Value id() { return 0; } static Value op(Value a, Value b) { return a + b; } };
struct MinOp { static Value id() { return std::numeric_limits<Value>::max(); } static Value op(Value a, Value b) { return a < b ? a : b; } };

template <class Op = SumOp>
class SegmentTreeRange {
public:
    explicit SegmentTreeRange(const std::vector<Value>& a) : n_(a.size()), t_(2 * a.size(), Op::id()) {
        for (std::size_t i = 0; i < n_; ++i) t_[n_ + i] = a[i];
        for (std::size_t i = n_; i-- > 1;) t_[i] = Op::op(t_[2 * i], t_[2 * i + 1]);
    }
    Value query(std::size_t l, std::size_t r) const {
        check_range(l, r, n_);
        Value left = Op::id(), right = Op::id();                          // keep order for non-commutative ops
        for (l += n_, r += n_; l < r; l >>= 1, r >>= 1) {
            if (l & 1) left = Op::op(left, t_[l++]);
            if (r & 1) right = Op::op(t_[--r], right);
        }
        return Op::op(left, right);
    }
    void set(std::size_t i, Value v) {
        check_index(i, n_);
        for (t_[i += n_] = v; i > 1; i >>= 1) t_[i >> 1] = Op::op(t_[i & ~std::size_t{1}], t_[i | 1]);
    }
    std::size_t size() const noexcept { return n_; }
    std::size_t memory_bytes() const noexcept { return t_.capacity() * sizeof(Value); }
private:
    std::size_t n_;
    std::vector<Value> t_;   // t_[n_ + i] = a[i]; t_[k] = op(t_[2k], t_[2k+1])
};

}  // namespace se618
