# Lecture 9 — Persistent Data Structures and Versioned Systems · Sat 21 Nov 2026

**Lesson:** immutability and versioning simplify reasoning, rollback, auditing and concurrency, but move cost into memory, storage, compaction and lifecycle management.
**Milestone:** **working implementation / prototype checkpoint due today, 23:59 Cairo** (the last part of the session covers what "working prototype" means). Testing + benchmarking checkpoint Sat 5 Dec.

Slides: `index.html`.

| File (`Student_Resources/`) | Purpose | Build / run |
|---|---|---|
| `versioned_store.hpp` | Full copy, delta chain (+ checkpoints, compaction) and path-copying stores behind one contract; one violates it | header-only |
| `versioned_test.cpp` | Snapshot-immutability (differential vs oracle over a branching history), release and compaction tests | `g++ -std=c++20 -O1 -g versioned_test.cpp` |
| `versioned_bench.cpp` | Two histories × four designs: commit cost, memory, old/newest read latency, trimming cost (~5 s) | `g++ -std=c++20 -O2 versioned_bench.cpp` |
| `EXERCISE.md` | Debug the snapshot violation; predict-then-measure | — |

Solution: `../../Instructor_Only/Lecture_09/versioned_store_solution.hpp` (add `-DSOLUTION -I../../../Instructor_Only/Lecture_09`). Measured results and answers: `../../Instructor_Only/Lecture_09/answers.md`.
