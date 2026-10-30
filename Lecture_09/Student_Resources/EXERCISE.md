# Lecture 9 exercise: versioned settings store

Teams, about 40 minutes. `versioned_store.hpp` contains three implementations of the same contract (see the header comment):

- **FullCopyStore**: every version is a complete copy.
- **DeltaChainStore**: every version stores only its changes and a parent link; optional checkpoint every k versions; `compact()` cuts history.
- **PathCopyStore**: a 32-ary tree; a commit copies only the paths to the changed slots (structural sharing).

## Part A — find the snapshot violation (15 min)
`g++ -std=c++20 -O1 -g versioned_test.cpp && ./a.out` reports that one store changes history. Find the cause, explain *why* it was probably written that way, and fix it. Then say which invariant was broken and how a code reviewer could have spotted it.

## Part B — predict, then measure (25 min)
Before running `versioned_bench.cpp`, predict for each store and each history (A: 20 versions × 30% of slots changed; B: 1,000 versions × 1 slot changed):

| Store | Commit cost | Memory | Read an old version | Read the newest | Cost to keep only the last 10 |
|---|---|---|---|---|---|
| full copy | | | | | |
| delta chain (no checkpoints) | | | | | |
| delta chain (checkpoint every 32) | | | | | |
| path copy | | | | | |

Run `g++ -std=c++20 -O2 versioned_bench.cpp -o bench && ./bench`, then answer:
1. For each store, under which history or read pattern does it become unattractive? Why?
2. Why does path copying barely help in history A?
3. `DeltaChainStore::release(v)` refuses when `v` has children. Is that a good API? What alternatives exist?
4. Workloads C (many reads of old versions) and D (mostly reads of the newest version): which store would you deploy for each?
