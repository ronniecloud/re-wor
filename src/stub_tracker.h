// =============================================================================
// stub_tracker.h — Lock-free frequency tracker for PPC stubs & unmapped funcs
// =============================================================================
// Dumps a report every N seconds to re_wor_stub_freq.log.
// Hot stubs (high call rate during loading screen) = likely blockers.
// =============================================================================
#pragma once

#include <rex/ppc.h>
#include <atomic>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <fstream>
#include <chrono>
#include <cstdint>
#include <iomanip>

struct StubEntry {
    const char* name;
    std::atomic<uint64_t>* counter;
};

struct UnmappedEntry {
    uint64_t count;
    uint32_t first_caller;
};

class StubTracker {
public:
    static StubTracker& instance() {
        static StubTracker tracker;
        return tracker;
    }

    // Register a stub — called once per stub (static local in macro).
    // Returns pointer to atomic counter for fast-path increment.
    std::atomic<uint64_t>* register_stub(const char* name) {
        auto* counter = new std::atomic<uint64_t>(0);
        std::lock_guard<std::mutex> lock(stub_mutex_);
        stubs_.push_back({name, counter});
        return counter;
    }

    // Record an unmapped function call (thread-safe)
    void record_unmapped(uint32_t target, uint32_t caller) {
        std::lock_guard<std::mutex> lock(unmapped_mutex_);
        auto& entry = unmapped_[target];
        entry.count++;
        if (entry.count == 1) {
            entry.first_caller = caller;
        }
    }

    // Start the background reporter thread
    void start_reporter(int interval_seconds = 5) {
        reporter_ = std::thread([this, interval_seconds]() {
            std::ofstream log("re_wor_stub_freq.log", std::ios::out | std::ios::trunc);
            log << "=== STUB FREQUENCY TRACKER STARTED ===" << std::endl;
            log << "Reports every " << interval_seconds << " seconds." << std::endl;
            log << "Hot stubs (high call rate) = likely loading screen blockers." << std::endl;
            log << std::endl;

            auto start = std::chrono::steady_clock::now();

            while (!stop_.load(std::memory_order_relaxed)) {
                std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now() - start).count();

                log << "--- Report @ " << elapsed << "s ---" << std::endl;

                bool any_activity = false;

                // Snapshot stub counters (swap to 0)
                {
                    std::lock_guard<std::mutex> lock(stub_mutex_);
                    for (auto& entry : stubs_) {
                        uint64_t count = entry.counter->exchange(0, std::memory_order_relaxed);
                        if (count > 0) {
                            log << "  STUB " << entry.name << ": " << count
                                << " calls/" << interval_seconds << "s";
                            if (count > 100) log << "  *** HOT ***";
                            if (count > 10000) log << " !!! BLOCKER CANDIDATE !!!";
                            log << std::endl;
                            any_activity = true;
                        }
                    }
                }

                // Snapshot unmapped counters
                {
                    std::lock_guard<std::mutex> lock(unmapped_mutex_);
                    for (auto& [addr, entry] : unmapped_) {
                        if (entry.count > 0) {
                            log << "  UNMAPPED 0x" << std::hex << std::setfill('0')
                                << std::setw(8) << addr
                                << " (from 0x" << std::setw(8) << entry.first_caller
                                << "): " << std::dec << entry.count
                                << " calls/" << interval_seconds << "s";
                            if (entry.count > 100) log << "  *** HOT ***";
                            if (entry.count > 10000) log << " !!! BLOCKER CANDIDATE !!!";
                            log << std::endl;
                            entry.count = 0;
                            any_activity = true;
                        }
                    }
                }

                if (!any_activity) {
                    log << "  (no stub/unmapped activity this interval)" << std::endl;
                }
            }
        });
        reporter_.detach();
    }

    void stop() { stop_.store(true, std::memory_order_relaxed); }

private:
    StubTracker() = default;
    ~StubTracker() { stop(); }

    std::mutex stub_mutex_;
    std::vector<StubEntry> stubs_;

    std::mutex unmapped_mutex_;
    std::unordered_map<uint32_t, UnmappedEntry> unmapped_;

    std::atomic<bool> stop_{false};
    std::thread reporter_;
};

// ============================================================================
// Tracked stub macro — hot path = single atomic increment (relaxed ordering)
// ============================================================================
#define PPC_STUB_TRACKED(subroutine) \
    extern "C" PPC_FUNC(subroutine) { \
        (void)base; \
        static std::atomic<uint64_t>* s_ctr = \
            StubTracker::instance().register_stub(#subroutine); \
        s_ctr->fetch_add(1, std::memory_order_relaxed); \
        ctx.r3.u64 = 0; \
    }
