# Lecture 5 exercise: finish the index, then measure what it costs

Pairs, about 40 minutes.

## Part A — implementation (20 min)
`inverted_index.hpp` contains a working analyzer, `add_document`, OR / AND-NOT queries, TF-IDF ranking and `check_invariants()`. Complete:

1. **TODO 1 `intersect()`**: posting-list intersection used by AND queries. Both inputs are sorted by document id. Target O(|a| + |b|).
2. **TODO 2 `remove_document()`**: after removal, every invariant in `check_invariants()` must still hold.

Build and run: `g++ -std=c++20 -O1 -g -fsanitize=address,undefined index_test.cpp && ./a.out`. The differential test compares your index against `LinearScan`, the obviously-correct baseline. `search_demo.cpp` runs queries over `sample_docs.txt`.

## Part B — experiment (20 min)
Before running `index_bench.cpp`, write down your hypothesis for 1,000, 10,000 and 50,000 documents:

| Quantity | Linear scan | Inverted index | Your predicted ratio |
|---|---|---|---|
| Build time | | | |
| Heap memory | | | |
| Query latency, rare terms | | | |
| Query latency, common terms | | | |

Then run `g++ -std=c++20 -O2 index_bench.cpp -o bench && ./bench` and answer:
- What did the faster search cost, in build time and memory?
- Why are common-term AND queries so much slower than rare-term ones?
- Both designs run the same prepared query sequence. Why does that matter? What would change if the index side repeated only three queries?
- At what corpus size, query rate and update rate would you still ship the linear scan?
- What does this benchmark **not** show?
