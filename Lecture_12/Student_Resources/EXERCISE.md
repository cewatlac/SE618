# Lecture 12 exercises

## E1 — Guess, then profile (15 min, pairs)
`profile_me.cpp` parses 300,000 log lines, aggregates per endpoint and sorts the endpoints.
1. **Before running anything**, write down which phase you think dominates and its approximate share: generating input · parsing · hash-map updates · sorting.
2. Benchmark: `g++ -std=c++20 -O2 profile_me.cpp -o prof && ./prof`.
3. Profile: `g++ -std=c++20 -O2 -DPHASE_TIMERS profile_me.cpp -o prof && ./prof` (or `gprof`, `perf`, VTune; see the file header).
4. Was your guess right? Propose one change, and state what you would re-measure to confirm it helped.

## E2 — Pitfalls demo (10 min)
`g++ -std=c++20 -O2 pitfalls_demo.cpp -o pit && ./pit`, then again with `-O0`. For each of P1–P5, write one sentence: what would a naive report have claimed, and why is it wrong?

## E3 — Critique three reports (25 min, groups)
`flawed_reports.md`: for each report — what can be concluded, what cannot, and how would you redesign the experiment?

## E4 — Your project's experiment (30 min, teams)
Fill `experiment_plan_template.md` for your project's **main claim**. Separate RESULT from INTERPRETATION from GENERALISATION. You may use `bench_harness.hpp` (median, p10/p90, bootstrap CI, raw CSV) in your project; acknowledge it.
