# Lecture 12 — Benchmarking, Profiling and Experimental Methodology · Sat 12 Dec 2026

**Lesson:** performance claims are empirical claims; they need credible experimental design. Measurement ≠ benchmark ≠ evidence ≠ interpretation ≠ conclusion.
**Deadline:** **Individual Engineering Analysis Assignment due today, 23:59 Cairo** (5 marks; about 1500–2000 words). Final repository freeze Sat 19 Dec, 09:00 Cairo; defense session 1 the same day.

Slides: `index.html`.

| File (`Student_Resources/`) | Purpose | Build / run |
|---|---|---|
| `bench_harness.hpp` | Reusable mini harness: warm-up, repetitions, do-not-optimise, median/p10/p90, bootstrap CI, raw CSV | header-only |
| `pitfalls_demo.cpp` | P1–P5: dead code, cold/warm, input size, single run vs distribution, unequal work (~2 s) | `g++ -std=c++20 -O2 pitfalls_demo.cpp` (and `-O0`) |
| `profile_me.cpp` | Guess-then-profile: a log-analytics job; phase timers or gprof/perf | `g++ -std=c++20 -O2 [-DPHASE_TIMERS] profile_me.cpp` |
| `flawed_reports.md` | Three flawed benchmark reports to critique | — |
| `experiment_plan_template.md` | CLAIM / BASELINE / WORKLOAD / … / DECISION + result/interpretation/generalisation | — |
| `EXERCISE.md` | E1–E4 | — |

Instructor: `../../Instructor_Only/Lecture_12/` — `profile_me_fixed.cpp` (after acting on the profile), `answers.md` (report critiques, measured outputs).
