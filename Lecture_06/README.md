# Lecture 6 — Range Queries and Workload-Driven Design · Sat 31 Oct 2026

**Lesson:** there is no universally best range-query structure; derive the representation from the workload.
**Milestone:** **architecture / design / API document due today, 23:59 Cairo** (the last ~20 minutes are a design-document clinic). Written midterm next Saturday, 7 Nov.

Slides: `index.html`.

| File (`Student_Resources/`) | Purpose | Build / run |
|---|---|---|
| `range_aggregate.hpp` | One contract, four representations: naive, prefix sums, Fenwick tree, segment tree (sum and min) | header-only |
| `range_test.cpp` | Contract + differential tests across all representations | `g++ -std=c++20 -O1 -g range_test.cpp` |
| `range_bench.cpp` | Workloads A–D: build cost, ns/op, memory (~10–20 s) | `g++ -std=c++20 -O2 range_bench.cpp` |
| `EXERCISE.md` | Deployment decision per workload, then measurement | — |

Model answers: `../../Instructor_Only/Lecture_06/workload_decisions.md`.
