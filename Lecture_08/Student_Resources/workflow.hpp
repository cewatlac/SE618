// SE618 Lecture 8 — starter workflow engine. The happy path works. Failure handling does not.
// Your tasks (see EXERCISE.md; workflow_test.cpp injects the failures):
//   TODO 1 complete(): stale and duplicate completion messages
//   TODO 2 apply(Failed): retries with backoff, retry limit, failure propagation (SKIPPED)
//   TODO 3 recover(): tasks that were RUNNING when the engine crashed
#pragma once
#include <algorithm>
#include <cstdint>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace se618 {

enum class State { Pending, Ready, Running, Succeeded, Failed, Retrying, Skipped, Cancelled };
inline bool terminal(State s) { return s == State::Succeeded || s == State::Failed || s == State::Skipped || s == State::Cancelled; }

struct TaskSpec {
    std::string id;
    std::vector<std::string> deps;
    int max_attempts = 3;       // total attempts allowed, including the first
    bool idempotent = true;     // safe to re-run if the outcome of an attempt is unknown?
};

// Durable, append-only record. The engine appends an event BEFORE acting on it; recovery
// replays the journal. (Here a vector stands in for an fsync'd log file or database table.)
struct Event {
    enum class Type { Started, Succeeded, Failed, Cancelled } type;
    std::string task;
    int attempt = 0;
    bool transient = false;
    std::int64_t time = 0;
    std::string note;
};
using Journal = std::vector<Event>;

class Workflow {
public:
    Workflow(std::vector<TaskSpec> specs, Journal& journal, std::int64_t backoff_base = 1000)
        : journal_(&journal), backoff_(backoff_base) {
        for (auto& s : specs) {
            if (tasks_.count(s.id)) throw std::invalid_argument("duplicate task " + s.id);
            tasks_[s.id] = Task{s, State::Pending, 0, 0};
        }
        for (auto& [id, t] : tasks_)
            for (auto& d : t.spec.deps) if (!tasks_.count(d)) throw std::invalid_argument(id + " depends on unknown " + d);
        promote();
    }

    std::vector<std::string> ready() const {
        std::vector<std::string> r;
        for (auto& [id, t] : tasks_) if (t.state == State::Ready) r.push_back(id);
        return r;                                   // std::map => sorted, deterministic
    }

    // Ready -> Running. The Started event is journaled BEFORE the task is dispatched.
    int start(const std::string& id, std::int64_t now) {
        Task& t = at(id);
        if (t.state != State::Ready) throw std::logic_error("start: " + id + " is not READY");
        journal_->push_back({Event::Type::Started, id, t.attempts + 1, false, now, ""});
        apply(journal_->back());
        return t.attempts;
    }

    // Running(attempt) -> Succeeded. Stale or duplicate completions are ignored and counted.
    bool complete(const std::string& id, int attempt, std::int64_t now) {
        // TODO 1: a completion is only valid for the CURRENT attempt of a RUNNING task.
        // Anything else (duplicate delivery, late message from an earlier attempt) must be
        // ignored, counted in ignored_, and must not change state. Return false in that case.
        (void)at(id);
        journal_->push_back({Event::Type::Succeeded, id, attempt, false, now, ""});
        apply(journal_->back());
        return true;
    }

    // Running(attempt) -> Retrying (transient, attempts left) or Failed (+ Skipped dependents).
    bool fail(const std::string& id, int attempt, bool transient, std::int64_t now, std::string note = "") {
        Task& t = at(id);
        if (t.state != State::Running || attempt != t.attempts) { ++ignored_; return false; }
        journal_->push_back({Event::Type::Failed, id, attempt, transient, now, std::move(note)});
        apply(journal_->back());
        return true;
    }

    // Retrying -> Ready once the exponential backoff has elapsed. Time is injected (testable).
    void tick(std::int64_t now) {
        for (auto& [id, t] : tasks_) if (t.state == State::Retrying && now >= t.retry_at) t.state = State::Ready;
    }

    void cancel(std::int64_t now) {
        journal_->push_back({Event::Type::Cancelled, "", 0, false, now, ""});
        apply(journal_->back());
    }

    // Rebuild state from the journal after a crash. A task that was RUNNING has an unknown
    // outcome: re-run it if idempotent (at-least-once), otherwise fail it for a human to reconcile.
    static Workflow recover(std::vector<TaskSpec> specs, Journal& journal, std::int64_t now) {
        Journal replay = journal;
        Workflow w(std::move(specs), journal);
        for (auto& e : replay) w.apply(e);
        // TODO 3: after replay, a task still RUNNING had an unknown outcome when the engine
        // died. Decide what happens to it (hint: TaskSpec::idempotent, max_attempts, fail_now).
        (void)now;
        return w;
    }

    State state(const std::string& id) const { return tasks_.at(id).state; }
    int attempts(const std::string& id) const { return tasks_.at(id).attempts; }
    std::size_t ignored_messages() const noexcept { return ignored_; }
    bool finished() const {
        return std::all_of(tasks_.begin(), tasks_.end(), [](auto& kv) { return terminal(kv.second.state); });
    }

    bool check_invariants() const {
        for (auto& [id, t] : tasks_) {
            bool deps_ok = std::all_of(t.spec.deps.begin(), t.spec.deps.end(), [&](auto& d) { return tasks_.at(d).state == State::Succeeded; });
            bool upstream_failed = std::any_of(t.spec.deps.begin(), t.spec.deps.end(), [&](auto& d) {
                auto s = tasks_.at(d).state; return s == State::Failed || s == State::Skipped || s == State::Cancelled; });
            switch (t.state) {
                case State::Ready: case State::Running: case State::Succeeded: case State::Retrying:
                    if (!deps_ok) return false;                          // never run before deps succeed
                    break;
                case State::Skipped:
                    if (!upstream_failed) return false;
                    break;
                case State::Pending:
                    if (deps_ok) return false;                           // promotion is not forgotten
                    break;
                default: break;
            }
            if (t.attempts < 0 || t.attempts > t.spec.max_attempts) return false;
            if (t.state == State::Running && t.attempts == 0) return false;
        }
        return true;
    }

private:
    struct Task { TaskSpec spec; State state; int attempts; std::int64_t retry_at; };

    Task& at(const std::string& id) {
        auto it = tasks_.find(id);
        if (it == tasks_.end()) throw std::out_of_range("unknown task " + id);
        return it->second;
    }

    void fail_now(const std::string& id, Task& t, std::int64_t now, std::string note) {
        t.state = State::Running;                                // make the transition explicit and journaled
        journal_->push_back({Event::Type::Failed, id, t.attempts, false, now, std::move(note)});
        apply(journal_->back());
    }

    // Single transition function used by both the live path and journal replay.
    void apply(const Event& e) {
        if (e.type == Event::Type::Cancelled) {
            for (auto& [id, t] : tasks_) if (!terminal(t.state)) t.state = State::Cancelled;
            return;
        }
        Task& t = at(e.task);
        switch (e.type) {
            case Event::Type::Started: t.state = State::Running; t.attempts = e.attempt; break;
            case Event::Type::Succeeded: t.state = State::Succeeded; promote(); break;
            case Event::Type::Failed:
                // TODO 2: transient failure with attempts left -> RETRYING with
                // retry_at = e.time + backoff_ * 2^(attempts-1); otherwise FAILED, and every
                // transitive dependent that is not terminal becomes SKIPPED (skip_dependents).
                t.state = State::Failed;
                break;
            default: break;
        }
    }

    void promote() {
        for (auto& [id, t] : tasks_) {
            if (t.state != State::Pending) continue;
            bool ok = std::all_of(t.spec.deps.begin(), t.spec.deps.end(), [&](auto& d) { return tasks_.at(d).state == State::Succeeded; });
            if (ok) t.state = State::Ready;
        }
    }

    void skip_dependents(const std::string& failed) {
        bool changed = true;
        std::set<std::string> dead{failed};
        while (changed) {                                        // transitive closure over dependents
            changed = false;
            for (auto& [id, t] : tasks_) {
                if (terminal(t.state) || dead.count(id)) continue;
                for (auto& d : t.spec.deps) if (dead.count(d)) { t.state = State::Skipped; dead.insert(id); changed = true; break; }
            }
        }
    }

    std::map<std::string, Task> tasks_;
    Journal* journal_;
    std::int64_t backoff_;
    std::size_t ignored_ = 0;
};

}  // namespace se618
