# Lecture 8 exercise: design the failure model, then make it pass

Teams, about 40 minutes. The artifact that matters most is the state model and its justification.

## Part A — state model (20 min, on paper)
Workflow: `download → validate → transform → {train, profile}`, `train → evaluate → publish`. `publish` posts results to an external API and is **not idempotent**.

1. **States and transitions:** draw the state machine for one task (PENDING, READY, RUNNING, SUCCEEDED, FAILED, RETRYING, SKIPPED, CANCELLED). Label every transition with its trigger. Which states are terminal?
2. **Ready logic:** when exactly does a task become READY?
3. **Retry policy:** which failures are retried? Limit? Backoff?
4. **Failure propagation:** what happens to `evaluate`, `publish` and `profile` if `train` fails permanently?
5. **Durable state:** what must be written down, and **when** (before or after dispatch)?
6. **Expected behaviour** for each injected fault:

| Fault | Expected behaviour | Why |
|---|---|---|
| Worker crashes while `transform` runs | | |
| `download` fails permanently | | |
| `train` fails transiently twice, then succeeds | | |
| Engine process restarts while `download` runs | | |
| Engine restarts while `publish` runs | | |
| The completion message for `validate` arrives twice | | |
| A late completion from attempt 1 arrives after attempt 2 started | | |

## Part B — implement (about 20 minutes in class, then continue after class)
You are not expected to finish every TODO in class. The main in-class outcomes are the Part A state model, transitions, invariants and failure semantics. Implement one or two failure cases now; making all of S1–S9 pass is continuation work.

`workflow.hpp` has the happy path. Complete TODO 1 (stale/duplicate completions), TODO 2 (retry, backoff, propagation) and TODO 3 (recovery) until `workflow_test.cpp` passes: `g++ -std=c++20 -O1 -g workflow_test.cpp && ./a.out`. Compare the tests' expectations with your table: where do you disagree, and is your version defensible?
