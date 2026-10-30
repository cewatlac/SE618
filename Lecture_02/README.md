# Lecture 2 — Production Hash Maps · Sat 03 Oct 2026

**Lesson:** expected O(1) is conditional on keys, hashing, load, deletion policy, memory layout and the adversary.
**Milestone:** team formation + preliminary topic (collect registrations at the break).

Slides: `index.html` (33 slides).

| File (`Student_Resources/`) | Purpose | Build / run |
|---|---|---|
| `flawed_hash_map.hpp` | Open-addressing map with three seeded defects (lab) | header-only |
| `hash_map_test.cpp` | Smoke test (passes) + differential test vs `std::unordered_map` (fails) | `g++ -std=c++20 -O1 -g hash_map_test.cpp` |
| `hash_map_bench.cpp` | Five workloads vs `std::unordered_map`; exposes the performance defects (~10 s) | `g++ -std=c++20 -O2 hash_map_bench.cpp` |

Solution: `../../Instructor_Only/Lecture_02/fixed_hash_map.hpp` (release after class if you choose). To test it, add `-DFIXED -I../../../Instructor_Only/Lecture_02` to the build line.
