# Lecture 05 — Search Systems and Inverted Indexes

Date: Sat 24 Oct 2026

[Open lecture slides](index.html) | [Back to course library](../index.html)

## Student resources

- [`EXERCISE.md`](Student_Resources/EXERCISE.md) — Implementation + hypothesis-first experiment
- [`index_bench.cpp`](Student_Resources/index_bench.cpp) — Scan vs index on one shared 48-query workload × 3 reps: build time, heap memory, median latency per query class at 1k/10k/50k docs (~30 s)
- [`index_test.cpp`](Student_Resources/index_test.cpp) — Contract + differential tests (fail until the TODOs are done)
- [`inverted_index.hpp`](Student_Resources/inverted_index.hpp) — Starter `InvertedIndex` (TODO: `intersect`, `remove_document`) + `LinearScan` baseline
- [`sample_docs.txt`](Student_Resources/sample_docs.txt)
- [`search_demo.cpp`](Student_Resources/search_demo.cpp)
