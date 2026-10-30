# Lecture 10 exercise: plan a Bloom filter, then check the plan

Pairs, about 40 minutes.

## Part A — implement (10 min)
In `bloom.hpp`, complete **TODO 1** `add()` and **TODO 2** `might_contain()` (double hashing: g_i = h1 + i·h2 mod m). Run `bloom_test.cpp` until it passes.

## Part B — plan before measuring (10 min)
A crawler must skip URLs it has already fetched. Expected distinct URLs per day: **n = 100,000**. Target false-positive rate: **p = 1%**.

| Quantity | Your value |
|---|---|
| m (bits) = −n ln p / (ln 2)² | |
| memory in KB | |
| k = (m / n) ln 2, rounded | |
| predicted p for your m, k, n: (1 − e^(−kn/m))^k | |
| predicted p if the real n is 2× the plan | |
| predicted p if the real n is 4× the plan | |

What does a false positive **cost** this crawler? What would a false negative cost?

## Part C — measure (15 min)
Run `g++ -std=c++20 -O2 bloom_lab.cpp -o lab && ./lab`.
1. Do you observe any false negatives? Should you ever?
2. Compare the observed FP rate over 5 seeds with your prediction. How large is the run-to-run spread?
3. Section 2 uses only 1,000 probes per filter. What would you have concluded from a single one of those runs?
4. Section 3 overfills the filter. At what point would you alert, and what signal could production monitor, given that the true n is often unknown?
5. Section 4 varies k for a fixed memory budget. Why is "more hash functions = more accurate" wrong?

## Part D — Count-Min Sketch (optional, 5 min)
Run `cms_demo.cpp`. Why does the sketch never underestimate? For which keys is the error acceptable at ε = 0.01?
