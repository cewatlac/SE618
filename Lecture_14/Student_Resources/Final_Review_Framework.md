# SE618 Final Review Framework

## 1. The decision framework (the backbone of the course)
For any data-structure or system decision:

1. Define the real problem (who suffers, in what system).
2. Quantify the workload (operations, ratios, rates, sizes, distributions, growth).
3. Identify the operations that matter.
4. Identify constraints (latency percentile, memory, update latency, cost).
5. State the guarantees required (exactness, consistency, durability, ordering).
6. Define acceptable failures and errors (and their cost).
7. Generate candidate designs, including a simple baseline.
8. Define the API and contracts (edge cases, errors, lifetimes).
9. Define invariants.
10. Implement the simplest credible baseline.
11. Test correctness (oracles, invariants, fault injection).
12. Form hypotheses (falsifiable, with a decision rule).
13. Design fair experiments (same work, same flags, representative workloads).
14. Measure (repetitions, spread, percentiles, memory).
15. Profile if needed (where does the time/memory go?).
16. Interpret the evidence (within its conditions).
17. State limitations (what the evidence does not show).
18. Choose.
19. Revisit when assumptions change.

## 2. The final examination (60 marks, written)
| Section | Format | Marks |
|---|---|---:|
| A — Scenario-based objective questions | 12 × 1 | 12 |
| B — Analytical engineering questions | 4 × 6 | 24 |
| C — Engineering case studies | 2 × 12 | 24 |

The paper tests reasoning, not recall. Expect to read short scenarios, code fragments, tables and benchmark results.

## 3. Skills you should be able to demonstrate
- Workload analysis, and spotting **missing requirements**
- Choosing between alternatives, and stating when the other one wins
- Writing API contracts and invariants
- Failure analysis (crashes, retries, capacity, silent degradation)
- Test design (oracles, differential and randomized tests, invariants)
- Benchmark interpretation and experimental critique
- Scalability reasoning (100× data, write-heavy shifts, skew)
- Memory trade-offs (bytes per element, pointer overhead, amplification)
- Concurrency reasoning (contention, atomicity, hot keys)
- Probabilistic-error reasoning (what an error costs, capacity drift)
- Limitation analysis

## 4. How to structure an answer
Strong answers usually contain:

**ASSUMPTIONS → DESIGN CHOICE → WHY → ALTERNATIVE → TRADE-OFF → EVIDENCE NEEDED → LIMITATION**

Pattern:
> "Given 95% reads, infrequent updates and sufficient memory, I would choose X because … If the workload changed to frequent updates, Y would become more attractive because … I would validate this by measuring … under … This conclusion assumes …"

Weak answers:
- name a structure without a workload ("use a segment tree, it is O(log n)");
- claim one structure is universally best;
- report a number without conditions or spread;
- ignore memory, failure or concurrency when the scenario mentions them.

If a scenario leaves something out, **state your assumption** and continue. Answers that make their assumptions explicit can earn credit even when a different assumption was intended.

## 5. How to revise
- For each lecture, write one sentence: "This design wins when …, loses when …"
- Re-read each lecture's "what the benchmark does not show" slide.
- Work through `Practice_Cases.md` **in writing**, using the answer pattern above, before discussing them.
