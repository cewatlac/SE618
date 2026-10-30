// SE618 Lecture 12 — where does the time go? GUESS FIRST (EXERCISE.md), then measure.
// A log-analytics job: parse 300,000 access-log lines, count requests and bytes per endpoint,
// then report the 10 busiest endpoints.
//
// Step 1 (benchmark): g++ -std=c++20 -O2 profile_me.cpp -o prof && ./prof
// Step 2 (profile)  : one of
//     g++ -std=c++20 -O2 -DPHASE_TIMERS profile_me.cpp -o prof && ./prof     (manual phase timers)
//     g++ -std=c++20 -O2 -pg profile_me.cpp -o prof && ./prof && gprof prof gmon.out | head -30
//     Linux: perf record -g ./prof && perf report      Windows: Visual Studio / VTune / Very Sleepy
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef PHASE_TIMERS
struct PhaseTimer {
    const char* name; std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
    explicit PhaseTimer(const char* n) : name(n) {}
    ~PhaseTimer() { std::printf("  [phase] %-22s %8.1f ms\n", name, std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count()); }
};
#define PHASE(n) PhaseTimer phase_timer_(n)
#else
#define PHASE(n)
#endif

static std::vector<std::string> generate_log(std::size_t n) {
    PHASE("generate input");
    std::mt19937 g(12);
    const char* verbs[] = {"GET", "POST", "PUT", "DELETE"};
    std::vector<std::string> lines;
    lines.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        std::ostringstream os;
        os << "10.0." << g() % 256 << '.' << g() % 256 << " - - [25/Oct/2026:10:" << g() % 60 << ':' << g() % 60
           << " +0200] \"" << verbs[g() % 4] << " /api/v1/resource" << (g() % 500) << "/item HTTP/1.1\" "
           << (g() % 10 ? 200 : 500) << ' ' << g() % 50000;
        lines.push_back(os.str());
    }
    return lines;
}

struct Stat { std::uint64_t requests = 0, bytes = 0, errors = 0; };

int main() {
    auto lines = generate_log(300000);
    auto t0 = std::chrono::steady_clock::now();
    std::unordered_map<std::string, Stat> stats;
    {
        PHASE("parse lines (regex)");
        static const std::regex re(R"re("(\w+) (\S+) HTTP/[\d.]+" (\d{3}) (\d+))re");
        std::smatch m;
        for (auto& line : lines) {
            if (!std::regex_search(line, m, re)) continue;
            Stat& s = stats[m[2].str()];
            ++s.requests;
            s.bytes += std::stoull(m[4].str());
            if (m[3].str()[0] == '5') ++s.errors;
        }
    }
    std::vector<std::pair<std::string, Stat>> top(stats.begin(), stats.end());
    {
        PHASE("sort endpoints");
        std::sort(top.begin(), top.end(), [](auto& a, auto& b) { return a.second.requests > b.second.requests; });
    }
    double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    std::printf("%zu lines, %zu endpoints, analysis took %.1f ms (input generation excluded)\n", lines.size(), stats.size(), ms);
    for (std::size_t i = 0; i < 3 && i < top.size(); ++i)
        std::printf("  %s  requests=%llu errors=%llu\n", top[i].first.c_str(), (unsigned long long)top[i].second.requests, (unsigned long long)top[i].second.errors);
}
