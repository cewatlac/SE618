# Lecture 10 — Probabilistic Data Structures · Sat 28 Nov 2026

**Lesson:** systems sometimes trade certainty for large savings in memory, bandwidth or computation. The question is what kind of error is allowed, how often, and what it costs.
**Upcoming:** **testing + benchmarking checkpoint due Sat 5 Dec 2026, 23:59 Cairo.** Individual Engineering Analysis due Sat 12 Dec.

Slides: `index.html`.

| File (`Student_Resources/`) | Purpose | Build / run |
|---|---|---|
| `bloom.hpp` | Bloom filter (TODO: `add`, `might_contain`) + Count-Min Sketch | header-only |
| `bloom_test.cpp` | No false negatives, FP band, merge, CMS never-underestimate | `g++ -std=c++20 -O1 -g bloom_test.cpp` |
| `bloom_lab.cpp` | Plan vs observed FP over 5 seeds, small-sample variance, over-capacity, varying k (< 1 s) | `g++ -std=c++20 -O2 bloom_lab.cpp` |
| `cms_demo.cpp` | Count-Min Sketch vs exact counts on a Zipf stream at three error budgets (< 1 s) | `g++ -std=c++20 -O2 cms_demo.cpp` |
| `EXERCISE.md` | Implement, plan, measure, interpret | — |

Solution: `../../Instructor_Only/Lecture_10/bloom_solution.hpp` (add `-DSOLUTION -I../../../Instructor_Only/Lecture_10`). Expected output: `../../Instructor_Only/Lecture_10/expected_results.md`.
