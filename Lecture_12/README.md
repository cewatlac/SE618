# Lecture 12 — Benchmarking, Profiling and Experimental Methodology

Date: Sat 12 Dec 2026

[Open lecture slides](index.html) | [Back to course library](../index.html)

## Student resources

- [`EXERCISE.md`](Student_Resources/EXERCISE.md) — E1–E4
- [`experiment_plan_template.md`](Student_Resources/experiment_plan_template.md) — CLAIM / BASELINE / WORKLOAD / … / DECISION + result/interpretation/generalisation
- [`flawed_reports.md`](Student_Resources/flawed_reports.md) — Three flawed benchmark reports to critique
- [`bench_harness.hpp`](Student_Resources/bench_harness.hpp) — Reusable mini harness: warm-up, repetitions, do-not-optimise, median/p10/p90, bootstrap CI, raw CSV
- [`pitfalls_demo.cpp`](Student_Resources/pitfalls_demo.cpp) — P1–P5: dead code, cold/warm, input size, single run vs distribution, unequal work (~2 s)
- [`profile_me.cpp`](Student_Resources/profile_me.cpp) — Guess-then-profile: a log-analytics job; phase timers or gprof/perf
