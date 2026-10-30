# Lecture 7 exercise: from "cycle" to a diagnosable resolver

Pairs, about 30 minutes.

## Part A — decide the behaviour (10 min)
For this input, write down exactly what `resolve()` should return (`ok`, `order`, `diagnostics`):

```
web   -> api
api   -> db, cache, db      (db listed twice)
db    -> (none)
jobs  -> queue              (queue is not declared)
auth  -> auth               (depends on itself)
a -> b,  b -> c,  c -> a    (cycle)
util  -> (none)
```

Questions: should the call throw? Should it return a partial order? Which problems are errors and which are warnings? What exactly should the operator see for the cycle?

## Part B — implement (about 17 minutes in class, then continue after class)
In class, aim for a subset: (1) a deterministic order; (2) the missing-dependency diagnostic; (3) one more diagnostic if time allows. Making the whole test suite pass is continuation work.

`resolver.hpp` states the contract and still contains the original naive implementation, which:
- produces an order that depends on hash-map iteration and input order;
- reports only "cycle detected";
- silently invents missing packages and merges duplicate declarations.

Rewrite `resolve()` so `resolver_test.cpp` passes: `g++ -std=c++20 -O1 -g resolver_test.cpp && ./a.out`.
