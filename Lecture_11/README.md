# Lecture 11 — Concurrency, Memory and Hardware-Aware Data Structures · Sat 5 Dec 2026

**Lesson:** sequential correctness and asymptotic complexity are insufficient for scalable concurrent software. Contention, synchronisation, memory layout, caches, false sharing, allocation and skew decide.
**Milestone:** **testing + benchmarking checkpoint due today, 23:59 Cairo** (the last part of the session is a benchmark-quality clinic). Individual Engineering Analysis due Sat 12 Dec.

Slides: `index.html`.

| File (`Student_Resources/`) | Purpose | Build / run |
|---|---|---|
| `concurrent_maps.hpp` | Global-mutex, global reader-writer-lock and 64-way sharded counter maps | header-only |
| `sharded_map_bench.cpp` | 1/2/4/8 threads × uniform vs hot-key; `--rwlock` adds `std::shared_mutex` (can be very slow on MinGW) (~15 s) | `g++ -std=c++20 -O2 -pthread sharded_map_bench.cpp` |
| `false_sharing.cpp` | Packed vs cache-line-padded per-thread counters (optional; needs several cores) (~12 s) | `g++ -std=c++20 -O2 -pthread false_sharing.cpp` |
| `locality_bench.cpp` | Vector vs sequential list vs random list; AoS vs SoA; four working-set sizes (~3 s) | `g++ -std=c++20 -O2 locality_bench.cpp` |
| `concurrency_test.cpp` | Lost-update stress test: atomic API vs `get()`+`put()`, 20 runs per configuration (~3 s) | `g++ -std=c++20 -O2 -pthread concurrency_test.cpp` |
| `EXERCISE.md` | Predict → run → explain for E1–E4 | — |

Instructor results and interpretation: `../../Instructor_Only/Lecture_11/measured_results.md`. There is no solution file: the exercises are experiments.
