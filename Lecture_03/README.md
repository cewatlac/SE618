# Lecture 3 — Caching Systems: LRU, LFU and Workloads · Sat 10 Oct 2026

**Lesson:** no eviction policy is universally best; the workload and product semantics decide.
**Milestone:** teams and topics settled; proposal due next week (17 Oct).

Slides: `index.html` (29 slides).

| File (`Student_Resources/`) | Purpose | Build / run |
|---|---|---|
| `cache_policies.hpp` | FIFO, LRU, O(1) LFU, SLRU, Random (membership simulation) | header-only |
| `cache_sim.cpp` | Hit rates for 5 policies × 4 workloads × 2 capacities (~5 s) | `g++ -std=c++20 -O2 cache_sim.cpp` |
| `buggy_lru.hpp` | get/put LRU with two defects (Activity A) | header-only |
| `lru_test.cpp` | Recency test + differential test vs an O(n) reference | `g++ -std=c++20 -O1 -g lru_test.cpp` |

Solution: `../../Instructor_Only/Lecture_03/lru_cache.hpp` (release after class if you choose). To test it, add `-DFIXED -I../../../Instructor_Only/Lecture_03` to the build line.
