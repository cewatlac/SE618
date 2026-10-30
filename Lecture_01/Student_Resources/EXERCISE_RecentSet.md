# Lecture 1 exercise: design a "seen recently?" component

Teams of 2–3, 25 minutes, one page.

**Scenario:** a payments platform must detect repeated event IDs. Peak 50,000 events/s; upstream retries arrive within 10 minutes; pod memory limit 2 GiB; p99 budget for the check 200 µs.

Produce:
1. **Expiration semantics:** what does "recently" mean, precisely?
2. **API:** signatures, plus behavior on edge cases.
3. **Representation** and at least 3 **invariants**.
4. **Memory** estimate at peak, and what happens at 3× peak.
5. **Complexity** of each operation, including the cost of expiry.
6. **Five tests** that would catch real bugs.

Decisions you must make and justify: count-based or time-based window? Does a duplicate extend the window? Who provides "now"? When is expiry performed? What happens when memory is full? Exact, or approximate with a fallback?

`recent_set.hpp` holds the count-bounded `RecentSet` from the lecture and a baseline to use as a test oracle. `recent_set_test.cpp` shows contract and differential tests you can adapt.
