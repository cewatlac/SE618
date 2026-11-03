# Lecture 11 — Concurrency, Memory and Hardware-Aware Data Structures

Date: Sat 5 Dec 2026

[Open lecture slides](index.html) | [Back to course library](../index.html)

## Student resources

- [`EXERCISE.md`](Student_Resources/EXERCISE.md) — Predict → run → explain for E1–E4
- [`concurrency_test.cpp`](Student_Resources/concurrency_test.cpp) — Lost-update stress test: atomic API vs `get()`+`put()`, 20 runs per configuration (~3 s)
- [`concurrent_maps.hpp`](Student_Resources/concurrent_maps.hpp) — Global-mutex, global reader-writer-lock and 64-way sharded counter maps
- [`false_sharing.cpp`](Student_Resources/false_sharing.cpp) — Packed vs cache-line-padded per-thread counters (optional; needs several cores) (~12 s)
- [`locality_bench.cpp`](Student_Resources/locality_bench.cpp) — Vector vs sequential list vs random list; AoS vs SoA; four working-set sizes (~3 s)
- [`sharded_map_bench.cpp`](Student_Resources/sharded_map_bench.cpp) — 1/2/4/8 threads × uniform vs hot-key; `--rwlock` adds `std::shared_mutex` (can be very slow on MinGW) (~15 s)
