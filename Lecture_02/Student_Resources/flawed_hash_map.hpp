// SE618 Lecture 2 — debugging activity. This open-addressing map passes the obvious tests
// and is still wrong. Find the defects with tests, not by staring.
// Hint: some defects are about correctness, some only about performance.
#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

namespace se618 {

template <class K, class V, class Hash = std::hash<K>>
class FlatMap {
    enum class State : std::uint8_t { Empty, Full, Tomb };
    struct Slot { State st = State::Empty; K key{}; V val{}; };

public:
    explicit FlatMap(std::size_t initial_capacity = 16) : slots_(round_up(initial_capacity)) {}

    // Inserts or overwrites. Returns true if a new key was inserted.
    bool insert_or_assign(const K& k, const V& v) {
        if ((size_ + 1) * 4 > slots_.size() * 3) grow();          // max load factor 0.75
        std::size_t i = index(k);
        for (;;) {
            Slot& s = slots_[i];
            if (s.st == State::Empty || s.st == State::Tomb) {
                s.st = State::Full; s.key = k; s.val = v; ++size_;
                return true;
            }
            if (s.key == k) { s.val = v; return false; }
            i = (i + 1) & mask();
        }
    }

    // Pointer is valid until the next insert_or_assign or erase.
    V* find(const K& k) {
        std::size_t i = index(k);
        for (std::size_t n = 0; n < slots_.size(); ++n) {
            Slot& s = slots_[i];
            if (s.st == State::Empty) return nullptr;
            if (s.st == State::Full && s.key == k) return &s.val;
            i = (i + 1) & mask();
        }
        return nullptr;
    }
    bool contains(const K& k) { return find(k) != nullptr; }

    bool erase(const K& k) {
        std::size_t i = index(k);
        for (std::size_t n = 0; n < slots_.size(); ++n) {
            Slot& s = slots_[i];
            if (s.st == State::Empty) return false;
            if (s.st == State::Full && s.key == k) {
                s.st = State::Tomb;   // keep the probe chain intact
                --size_;
                return true;
            }
            i = (i + 1) & mask();
        }
        return false;
    }

    std::size_t size() const noexcept { return size_; }
    std::size_t capacity() const noexcept { return slots_.size(); }

    template <class F> void for_each(F&& f) const {
        for (const Slot& s : slots_) if (s.st == State::Full) f(s.key, s.val);
    }

private:
    static std::size_t round_up(std::size_t n) { std::size_t c = 8; while (c < n) c <<= 1; return c; }
    std::size_t mask() const { return slots_.size() - 1; }
    std::size_t index(const K& k) const { return Hash{}(k) & mask(); }

    void grow() {
        std::vector<Slot> old = std::move(slots_);
        slots_.assign(old.size() * 2, Slot{});
        size_ = 0;
        for (Slot& s : old) if (s.st == State::Full) insert_or_assign(s.key, s.val);
    }

    std::vector<Slot> slots_;
    std::size_t size_ = 0;
};

}  // namespace se618
