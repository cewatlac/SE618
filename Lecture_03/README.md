# Lecture 03 — Caching Systems: LRU, LFU and Workloads

Date: Sat 10 Oct 2026

[Open lecture slides](index.html) | [Back to course library](../index.html)

## Student resources

- [`buggy_lru.hpp`](Student_Resources/buggy_lru.hpp) — get/put LRU with two defects (Activity A)
- [`cache_policies.hpp`](Student_Resources/cache_policies.hpp) — FIFO, LRU, O(1) LFU, SLRU, Random (membership simulation)
- [`cache_sim.cpp`](Student_Resources/cache_sim.cpp) — Hit rates for 5 policies × 4 workloads × 2 capacities (~5 s)
- [`lru_test.cpp`](Student_Resources/lru_test.cpp) — Recency test + differential test vs an O(n) reference
