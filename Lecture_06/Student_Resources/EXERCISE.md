# Lecture 6 exercise: one interface, which implementation?

Teams, about 40 minutes. Every implementation in `range_aggregate.hpp` honours the same contract: 0-based, half-open `[l, r)`, `set` assigns, `int64` sums, invalid ranges throw.

## Part A — decide before measuring (20 min)
For each workload, choose the representation you would **deploy** and justify it using at least three of: query cost, update cost, construction cost, memory, implementation and review risk, generality (min/max?), concurrency.

| Workload | Description | Your choice | Justification | What would change your mind? |
|---|---|---|---|---|
| A | n ≈ 1M, 99% range-sum queries, 1% point updates | | | |
| B | n ≈ 1M, 50% queries, 50% point updates | | | |
| C | n ≈ 1M, built once per night from a batch file, then read-only | | | |
| D | n ≈ 1,000 config values, 50/50 mix, maintained by a team new to the codebase | | | |
| E (discuss) | Same as B, but the dashboard also needs range **minimum** | | | |

"X is O(log n)" is not a justification on its own.

## Part B — measure (15 min)
`g++ -std=c++20 -O2 range_bench.cpp -o bench && ./bench`. Compare the table with your decisions. Where were you wrong, and why? What does the benchmark **not** show?

`range_test.cpp` runs all representations against the naive oracle, including the min segment tree.
