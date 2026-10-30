# Lecture 4 — Tries and Autocomplete Systems · Sat 17 Oct 2026

**Lesson:** attractive complexity can lose to memory, locality, update cost and workload.
**Milestone:** **project proposal due today, 23:59 Cairo** — Part 6 is a proposal clinic.

Slides: `index.html` (28 slides).

| File (`Student_Resources/`) | Purpose | Build / run |
|---|---|---|
| `autocomplete_bench.cpp` | Sorted array vs pointer trie + DFS vs trie with cached top-k; counting allocator for memory; differential check across designs | `g++ -std=c++20 -O2 autocomplete_bench.cpp -o ac && ./ac [terms]` (200k ≈ 20 s; `./ac 50000` ≈ 5 s) |
