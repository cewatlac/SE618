# SE618 Practice Cases

These cases practise the *type* of reasoning the final exam expects. They are not exam questions. For each case, write a short answer (about half a page) using the pattern **assumptions → choice → why → alternative → trade-off → evidence needed → limitation**. Model reasoning is discussed in class.

---

**P1 — The workload changed.** A product-page cache used LRU for two years with a good hit rate. After a redesign, most traffic comes from a stable "top 1,000 products" list, plus a nightly export job that reads every product once. The hit rate dropped from 72% to 51%.
(a) Explain the likely mechanism. (b) Propose a policy change and justify it. (c) What would you measure before deploying it, and what result would make you keep LRU?

**P2 — Fast queries, slow writes.** A search index answers queries in 3 ms, but since launch, document updates have become ~50× slower than writing the raw document to the database. Updates now arrive at 200 per second, and editors complain that changes appear late.
(a) Where does the update cost come from? (b) Give two design options and their consistency consequences. (c) What contract should the system state about when an update becomes searchable?

**P3 — Capacity was wrong.** A Bloom filter in front of a database was planned for 10 million keys at a 1% false-positive rate. It now holds 30 million keys.
(a) Estimate what happened to the false-positive rate, and explain why the change is not linear. (b) What does the database experience? (c) What should have been monitored, and what are your options now?

**P4 — More threads, less throughput.** A service using one global lock around a hash map gets faster from 1 to 4 threads and slower at 16.
(a) Give three possible causes and the measurement that distinguishes them. (b) Would sharding the map fix it? Under which workload would it not? (c) What would you try if 40% of requests hit one key?

**P5 — "5× faster".** A pull request claims "the new parser is 5× faster" based on one run on the developer's laptop, comparing against numbers recorded last month.
(a) What can be concluded? (b) List the threats to validity. (c) Design the experiment you would accept as evidence.

**P6 — Thousands of snapshots.** A document service stores a full snapshot after every edit. Users make many tiny edits; some documents have 20,000 versions. Storage costs are growing quickly, but users rarely open versions older than a week.
(a) Compare two alternative representations for history. (b) What happens to read cost for old versions in each? (c) What lifecycle policy (retention, compaction) would you propose, and what must it never delete?

**P7 — Success, crash, retry.** A workflow task sends an invoice via an external API. The API call succeeded, but the engine crashed before recording success. After recovery, the task ran again.
(a) What went wrong, and which states and events were involved? (b) Propose two ways to prevent a duplicate invoice, and their costs. (c) Which tests would have caught this before production?

**P8 — No longer immutable.** A dashboard answers range-sum queries over one year of per-minute data using prefix sums, rebuilt nightly. The product now requires late corrections to be visible within one minute, at about 50 corrections per second, mostly to the last hour of data.
(a) Why is the current design now problematic, or is it? (b) Propose a design and justify it using the update pattern. (c) What would you benchmark to confirm your choice?
