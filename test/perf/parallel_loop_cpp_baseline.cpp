#include <atomic>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <vector>

struct ParallelLoopBaselineResult {
    std::uint64_t value;
    bool stopped;
};

ParallelLoopBaselineResult parallel_loop_cpp_baseline(std::uint64_t limit,
                                                      std::uint64_t target,
                                                      std::size_t workers,
                                                      std::size_t batch) {
    std::atomic<bool> stop{false};
    std::atomic<std::uint64_t> first{0};
    std::vector<std::thread> threads;
    threads.reserve(workers);

    for (std::size_t worker = 0; worker < workers; ++worker) {
        threads.emplace_back([&, worker] {
            std::uint64_t cursor = static_cast<std::uint64_t>(worker * batch);
            while (!stop.load(std::memory_order_acquire) && cursor < limit) {
                const auto end = cursor + static_cast<std::uint64_t>(batch);
                for (auto value = cursor; value < end && value < limit; ++value) {
                    if (value == target) {
                        first.store(value, std::memory_order_release);
                        stop.store(true, std::memory_order_release);
                        return;
                    }
                }
                cursor += static_cast<std::uint64_t>(workers * batch);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return ParallelLoopBaselineResult{
        first.load(std::memory_order_acquire),
        stop.load(std::memory_order_acquire),
    };
}

