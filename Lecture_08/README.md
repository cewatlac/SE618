# Lecture 8 — Workflow Engines, DAGs, Failure and Recovery · Sat 14 Nov 2026

**Lesson:** failure semantics and recovery behavior matter as much as successful execution.
**Upcoming:** **working implementation / prototype checkpoint due Sat 21 Nov 2026, 23:59 Cairo.** Engineering Analysis Assignment released today (due Sat 12 Dec).

Slides: `index.html`.

| File (`Student_Resources/`) | Purpose | Build / run |
|---|---|---|
| `workflow.hpp` | Workflow engine: happy path works; TODOs for stale messages, retries/propagation, crash recovery | header-only |
| `workflow_test.cpp` | Nine failure-injection scenarios (S1–S9) | `g++ -std=c++20 -O1 -g workflow_test.cpp` |
| `EXERCISE.md` | State-model design sheet + fault-injection cases | — |

Solution: `../../Instructor_Only/Lecture_08/workflow_solution.hpp` (to test it, add `-DSOLUTION -I../../../Instructor_Only/Lecture_08`). Model state table: `../../Instructor_Only/Lecture_08/state_model_answer.md`.
