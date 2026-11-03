# Lecture 09 — Persistent Data Structures and Versioned Systems

Date: Sat 21 Nov 2026

[Open lecture slides](index.html) | [Back to course library](../index.html)

## Student resources

- [`EXERCISE.md`](Student_Resources/EXERCISE.md) — Debug the snapshot violation; predict-then-measure
- [`versioned_bench.cpp`](Student_Resources/versioned_bench.cpp) — Two histories × four designs: commit cost, memory, old/newest read latency, trimming cost (~5 s)
- [`versioned_store.hpp`](Student_Resources/versioned_store.hpp) — Full copy, delta chain (+ checkpoints, compaction) and path-copying stores behind one contract; one violates it
- [`versioned_test.cpp`](Student_Resources/versioned_test.cpp) — Snapshot-immutability (differential vs oracle over a branching history), release and compaction tests
