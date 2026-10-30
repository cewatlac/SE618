# Lecture 11 experiments: contention, sharing and locality

Pairs. Each experiment has a **predict → run → explain** step. Write the prediction first.

## E1 — Global lock vs sharded locks (20 min)
`concurrent_maps.hpp`: `GlobalLockMap` (one mutex), `ShardedMap<64>` (64 independently locked shards).
Predict the throughput at 1, 2, 4 and 8 threads for each design under (a) uniform keys and (b) a hot key receiving 90% of operations. Then run:
`g++ -std=c++20 -O2 -pthread sharded_map_bench.cpp -o bench && ./bench`
- Why does the global lock get *slower* as threads are added?
- Why does sharding not help for the hot-key workload?
- Why can sharding be slower with one thread?

## E2 — False sharing (optional; needs ≥ 4 cores, 10 min)
`g++ -std=c++20 -O2 -pthread false_sharing.cpp -o fs && ./fs`
Each thread touches only its own counter. Explain why the packed layout still does not scale. How would you detect this in a real program?

## E3 — Locality (10 min)
`g++ -std=c++20 -O2 locality_bench.cpp -o loc && ./loc`
The same logical work (sum n values) over a vector, a list in memory order and a list in random memory order, at growing sizes. At which working-set size does each layout change behaviour, and why? What does the AoS vs SoA comparison add?

## E4 — What a stress test proves (10 min)
`g++ -std=c++20 -O2 -pthread concurrency_test.cpp -o ct && ./ct`
Every call of `get()`/`put()` is thread-safe, yet `put(k, get(k) + 1)` loses updates. With 100,000 keys, how many of the 20 runs showed the bug? What does "20 runs passed" prove? (On Linux/macOS, also run the ThreadSanitizer build shown in the file header. Why is it silent here?)

Record the machine (CPU, cores, OS, compiler) next to every number you report.
