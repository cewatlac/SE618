# Lecture 06 — Range Queries and Workload-Driven Design

Date: Sat 31 Oct 2026

[Open lecture slides](index.html) | [Back to course library](../index.html)

## Student resources

- [`EXERCISE.md`](Student_Resources/EXERCISE.md) — Deployment decision per workload, then measurement
- [`range_aggregate.hpp`](Student_Resources/range_aggregate.hpp) — One contract, four representations: naive, prefix sums, Fenwick tree, segment tree (sum and min)
- [`range_bench.cpp`](Student_Resources/range_bench.cpp) — Workloads A–D: build cost, ns/op, memory (~10–20 s)
- [`range_test.cpp`](Student_Resources/range_test.cpp) — Contract + differential tests across all representations
