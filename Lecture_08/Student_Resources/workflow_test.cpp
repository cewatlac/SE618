// SE618 Lecture 8 — failure-injection tests. Each scenario states the expected behaviour.
// g++ -std=c++20 -O1 -g workflow_test.cpp && ./a.out
// Instructor: add -DSOLUTION -I../../../Instructor_Only/Lecture_08 to test the solution.
#ifdef SOLUTION
#include "workflow_solution.hpp"
#else
#include "workflow.hpp"
#endif
#include <cstdio>
#include <exception>
#include <set>

using namespace se618;
static int fails = 0;
#define CHECK(c) do { if (!(c)) { ++fails; std::printf("FAIL line %d: %s\n", __LINE__, #c); } } while (0)

// download -> validate -> transform -> {train, profile} ; train -> evaluate -> publish
static std::vector<TaskSpec> pipeline() {
    return {{"download", {}}, {"validate", {"download"}}, {"transform", {"validate"}},
            {"train", {"transform"}}, {"profile", {"transform"}}, {"evaluate", {"train"}},
            {"publish", {"evaluate"}, 3, /*idempotent=*/false}};
}
static void run_until(Workflow& w, const std::string& stop, std::int64_t& now) {   // happy-path executor
    for (bool progress = true; progress;) {
        progress = false;
        for (auto& id : w.ready()) {
            if (id == stop) continue;
            int a = w.start(id, now); w.complete(id, a, ++now); progress = true;
            CHECK(w.check_invariants());
        }
    }
}

// A payment provider that honours idempotency keys (as real payment APIs do).
struct FakeGateway {
    std::set<std::string> seen; int charges = 0;
    void charge(const std::string& key) { if (seen.insert(key).second) ++charges; }
};

static void scenarios() {
    std::int64_t now = 0;
    try {   // S1 happy path: runs to completion, dependencies respected
        Journal j; Workflow w(pipeline(), j);
        CHECK((w.ready() == std::vector<std::string>{"download"}));
        run_until(w, "", now);
        CHECK(w.finished() && w.state("publish") == State::Succeeded);
    } catch (const std::exception& e) { ++fails; std::printf("FAIL: exception: %s\n", e.what()); }
    try {   // S2 transient failure: RETRYING, backoff respected, second attempt succeeds
        Journal j; Workflow w(pipeline(), j, 1000);
        int a = w.start("download", 0);
        w.fail("download", a, /*transient=*/true, 10);
        CHECK(w.state("download") == State::Retrying);
        w.tick(500);  CHECK(w.state("download") == State::Retrying);    // backoff not elapsed
        w.tick(1010); CHECK(w.state("download") == State::Ready);
        CHECK(w.start("download", 1010) == 2);
        CHECK(w.complete("download", 2, 1020) && w.state("validate") == State::Ready);
    } catch (const std::exception& e) { ++fails; std::printf("FAIL: exception: %s\n", e.what()); }
    try {   // S3 retry limit + propagation: dependents SKIPPED, independent branch continues
        Journal j; Workflow w(pipeline(), j, 1);
        run_until(w, "train", now);                                     // everything up to train done
        CHECK(w.state("profile") == State::Succeeded);
        for (int k = 0; k < 3; ++k) {                                   // max_attempts = 3
            int a = w.start("train", now); w.fail("train", a, true, now); w.tick(now += 100);
        }
        CHECK(w.state("train") == State::Failed);
        CHECK(w.state("evaluate") == State::Skipped && w.state("publish") == State::Skipped);
        CHECK(w.finished() && w.check_invariants());
    } catch (const std::exception& e) { ++fails; std::printf("FAIL: exception: %s\n", e.what()); }
    try {   // S4 permanent failure is not retried
        Journal j; Workflow w(pipeline(), j);
        int a = w.start("download", 0); w.fail("download", a, /*transient=*/false, 1);
        CHECK(w.state("download") == State::Failed && w.state("validate") == State::Skipped);
    } catch (const std::exception& e) { ++fails; std::printf("FAIL: exception: %s\n", e.what()); }
    try {   // S5 duplicate and stale completion messages are ignored
        Journal j; Workflow w(pipeline(), j, 1);
        int a1 = w.start("download", 0); w.fail("download", a1, true, 1); w.tick(10);
        int a2 = w.start("download", 10);
        CHECK(!w.complete("download", a1, 11));                         // late message from attempt 1
        CHECK(w.state("download") == State::Running);
        CHECK(w.complete("download", a2, 12));
        CHECK(!w.complete("download", a2, 13));                         // duplicate delivery
        CHECK(w.ignored_messages() == 2 && w.check_invariants());
    } catch (const std::exception& e) { ++fails; std::printf("FAIL: exception: %s\n", e.what()); }
    try {   // S6 engine crash while an idempotent task runs: replay, re-run as attempt 2
        Journal j;
        { Workflow w(pipeline(), j); int a = w.start("download", 0); (void)a; }   // crash: object gone
        Workflow w = Workflow::recover(pipeline(), j, 100);
        CHECK(w.state("download") == State::Ready);
        CHECK(w.start("download", 100) == 2 && w.check_invariants());
    } catch (const std::exception& e) { ++fails; std::printf("FAIL: exception: %s\n", e.what()); }
    try {   // S7 crash during a NON-idempotent task: do not blindly re-run; fail for reconciliation
        Journal j;
        { Workflow w(pipeline(), j); run_until(w, "publish", now); w.start("publish", now); }
        Workflow w = Workflow::recover(pipeline(), j, now);
        CHECK(w.state("publish") == State::Failed && w.state("evaluate") == State::Succeeded);
        CHECK(!j.empty() && !j.back().note.empty());                     // reason is recorded
    } catch (const std::exception& e) { ++fails; std::printf("FAIL: exception: %s\n", e.what()); }
    try {   // S8 the side effect succeeded, the engine crashed before journaling Succeeded.
        // The retry is safe only because the key identifies the TASK, not the attempt.
        FakeGateway pay; Journal j;
        { Workflow w({{"charge", {}}}, j); w.start("charge", 0); pay.charge("wf42/charge"); }   // crash here
        Workflow w = Workflow::recover({{"charge", {}}}, j, 5);
        int a = w.start("charge", 5); pay.charge("wf42/charge"); w.complete("charge", a, 6);
        CHECK(pay.charges == 1 && w.finished());
    } catch (const std::exception& e) { ++fails; std::printf("FAIL: exception: %s\n", e.what()); }
    try {   // S9 replaying a finished journal reproduces the same state
        Journal j; Workflow w(pipeline(), j); run_until(w, "", now);
        Workflow r = Workflow::recover(pipeline(), j, now);
        CHECK(r.finished() && r.state("publish") == State::Succeeded && r.attempts("train") == 1);
    } catch (const std::exception& e) { ++fails; std::printf("FAIL: exception: %s\n", e.what()); }
}

int main() {
    try { scenarios(); }
    catch (const std::exception& e) { ++fails; std::printf("FAIL: uncaught exception: %s\n", e.what()); }
    std::printf(fails ? "%d failure(s)\n" : "all tests passed\n", fails);
    return fails != 0;
}
