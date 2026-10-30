# Three benchmark reports to critique

For each report, answer in your group: **(1) What can be concluded? (2) What cannot? (3) How would you redesign the experiment?** List every problem you find, and rank the top three by how much they could change the conclusion.

---

## Report 1 — "Our trie is 40× faster than a sorted array"
> We implemented autocomplete with a trie and with a sorted array + binary search. We built both from a list of 500 English words and ran the query "pro" once for each implementation, timing with `std::chrono`. The trie answered in 0.4 µs and the sorted array in 16 µs. The trie is 40× faster, so we chose the trie for our 5-million-product catalogue. The trie was compiled in Release mode; we used the sorted-array code from our earlier Debug build because it was already compiled. Memory was not measured because modern servers have plenty of RAM.

---

## Report 2 — "Sharding improves throughput by 3.1×"
> We compared a global-lock cache and a 16-shard cache. Each was run once for 10 seconds with 8 threads on a developer laptop while the IDE and a video call were open. Keys were uniformly random over 10 million keys. Mean throughput: global 2.1 M ops/s, sharded 6.5 M ops/s (+210%). We therefore expect a 3.1× improvement in production, where 60% of traffic goes to the 100 most popular products.

---

## Report 3 — "LFU reduces latency by 18%"
> We replaced LRU with LFU in our cache simulator and replayed a synthetic Zipf trace (s = 0.9) of 1 million requests. Average latency fell from 11.2 ms to 9.2 ms (18%). We ran the LFU experiment 10 times and report the best run; the LRU run was done once last week on another machine. Hit rate was not recorded because latency is what users feel. We conclude that LFU is the better eviction policy.
