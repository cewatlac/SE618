# Lecture 5 — Search Systems and Inverted Indexes · Sat 24 Oct 2026

**Lesson:** indexes trade storage cost and update complexity for query performance.
**Upcoming:** architecture / design / API document due **Sat 31 Oct 2026, 23:59 Cairo**. Written midterm Sat 7 Nov.

Slides: `index.html`.

| File (`Student_Resources/`) | Purpose | Build / run |
|---|---|---|
| `inverted_index.hpp` | Starter `InvertedIndex` (TODO: `intersect`, `remove_document`) + `LinearScan` baseline | header-only |
| `index_test.cpp` | Contract + differential tests (fail until the TODOs are done) | `g++ -std=c++20 -O1 -g index_test.cpp` |
| `index_bench.cpp` | Scan vs index on one shared 48-query workload × 3 reps: build time, heap memory, median latency per query class at 1k/10k/50k docs (~30 s) | `g++ -std=c++20 -O2 index_bench.cpp` |
| `search_demo.cpp`, `sample_docs.txt` | Tiny CLI over 12 sample documents | `g++ -std=c++20 search_demo.cpp -o demo && ./demo "posting lists"` |
| `EXERCISE.md` | Implementation + hypothesis-first experiment | — |

Solution: `../../Instructor_Only/Lecture_05/inverted_index_solution.hpp` (release after class if you choose). To test it, add `-DSOLUTION -I../../../Instructor_Only/Lecture_05`.
