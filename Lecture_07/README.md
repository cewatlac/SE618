# Lecture 7 — Written Midterm + Dependency Graph Engineering · Sat 7 Nov 2026

**Session:** 00:00–01:30 individual written midterm · 01:30–01:40 break · 01:40–03:00 Dependency Graph Engineering (~80 min).
**Lesson:** diagnosability and deterministic behavior are engineering properties.
**Upcoming:** Engineering Analysis Assignment released Sat 14 Nov; prototype checkpoint Sat 21 Nov.

Slides: `index.html` (the teaching part only; the midterm paper is controlled separately).

| File (`Student_Resources/`) | Purpose | Build / run |
|---|---|---|
| `resolver.hpp` | Improved `resolve()` contract + the original naive implementation | header-only |
| `resolver_test.cpp` | Behaviour specification (fails on the naive implementation) | `g++ -std=c++20 -O1 -g resolver_test.cpp` |
| `EXERCISE.md` | API/diagnostics decisions + implementation | — |

Solution: `../../Instructor_Only/Lecture_07/resolver_solution.hpp` (to test it, add `-DSOLUTION -I../../../Instructor_Only/Lecture_07`).
