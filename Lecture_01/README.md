# Lecture 1 — Data Structures as Software Products · Sat 26 Sep 2026

**Lesson:** a structure is judged by its contract, evidence and failure behavior, not by its name.
**Milestone:** course + project brief released. Covers the full assessment structure and all deadlines (Part 6).

Slides: `index.html` (38 slides; instructor notes on `N`).

| File (`Student_Resources/`) | Purpose | Build / run |
|---|---|---|
| `recent_set.hpp` | `RecentSet` (count-bounded, as on the Part 3 slides) and `RecentSetBaseline` (oracle) | header-only |
| `EXERCISE_RecentSet.md` | Part 4 design-exercise brief | — |
| `recent_set_test.cpp` | Contract and differential tests | `g++ -std=c++20 -O1 -g -fsanitize=address,undefined recent_set_test.cpp` |
| `recent_set_bench.cpp` | Vector vs list+map benchmark, to be critiqued | `g++ -std=c++20 -O2 recent_set_bench.cpp` |

Exercise solution (`recent_window.hpp` + `recent_window_test.cpp`) is in `../../Instructor_Only/Lecture_01/` — release after class if you choose.
