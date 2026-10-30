# Experiment plan (one per claim) — keep it in `docs/evaluation.md`

| Field | Your answer |
|---|---|
| **CLAIM** — what are we trying to show? (one sentence, falsifiable) | |
| **BASELINE** — compared with what? (and why is it a fair baseline?) | |
| **WORKLOAD** — operations, mix, key/value distribution, sizes, source (trace or generator + justification) | |
| **INDEPENDENT VARIABLES** — what we vary (e.g. n, threads, skew) | |
| **METRICS** (dependent variables) — latency percentiles, throughput, memory, build time, error rate… | |
| **CONTROLS** — what stays fixed (compiler + flags, machine, input seeds, warm-up, data layout) | |
| **REPETITIONS** — how many, how summarised (median + spread / CI), how outliers are handled | |
| **THREATS TO VALIDITY** — internal (confounders), external (does it transfer?), construct (is the metric the right one?) | |
| **DECISION RULE** — which result makes us choose design A, and which B? | |

## Reporting template

- **RESULT** (what was measured, with uncertainty): "On workload W, implementation A had median latency 2.4 ms [95% CI 2.3–2.5], B 3.1 ms [3.0–3.3], n = 30 runs each."
- **INTERPRETATION** (why, within this setup): "Under W, A appears preferable because…"
- **GENERALISATION** (limits): "We cannot yet claim A is always faster: we did not test…"

Record the environment with every result: CPU model and core count, OS, compiler + version + flags, commit hash, input seed(s), date.
