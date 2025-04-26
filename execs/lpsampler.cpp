#include <LpSampler.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <stop_token>
#include <thread>
#include <vector>

int main() {
    std::vector<int64_t> freqs = {119, 60, 7, 76, 63, 68, -37, 31, 29, -1};
    size_t n = freqs.size();

    double eps = 0.0625;
    double delta = 0.1;

    std::random_device rd;
    std::mt19937_64 rng(rd());
    uint64_t seed = rng();

    size_t num_samplers = static_cast<size_t>(4 * (1.0 / eps) * -std::log(delta));
    size_t num_threads =
        std::min<size_t>(std::thread::hardware_concurrency(), num_samplers);
    if (num_threads == 0) num_threads = 4;  // fallback
    size_t samplers_per_thread = (num_samplers + num_threads - 1) / num_threads;

    // std::cout << "Number of samplers: " << num_samplers << std::endl;
    // std::cout << "Number of threads: " << num_threads << std::endl;
    // std::cout << "Samplers per thread: " << samplers_per_thread << std::endl;

    std::atomic<bool> found_sample(false);
    std::atomic<size_t> sampled_index(n);  // n is invalid
    std::atomic<size_t> threads_finished(0);
    std::vector<std::jthread> threads;

    auto sampler_task = [&](std::stop_token stoken, size_t thread_idx) {
        size_t start = thread_idx * samplers_per_thread;
        size_t end = std::min(start + samplers_per_thread, num_samplers);
        for (size_t s = start; s < end && !stoken.stop_requested(); ++s) {
            LpSampler sampler(1, eps, delta, n, seed + s);
            for (size_t i = 0; i < n; ++i) {
                sampler.update(i, freqs[i]);
            }
            auto sample = sampler.sample();
            if (sample && !found_sample.exchange(true)) {
                sampled_index = *sample;
                break;
            }
        }
        threads_finished.fetch_add(1, std::memory_order_relaxed);
    };

    for (size_t t = 0; t < num_threads; ++t) {
        threads.emplace_back(sampler_task, t);
    }

    // Wait for either a sample or all threads to finish
    while (!found_sample &&
           threads_finished.load(std::memory_order_relaxed) < num_threads) {
        std::this_thread::yield();
    }

    for (auto& t : threads) {
        t.request_stop();
    }
    threads.clear();

    std::ofstream log_file("../logs/lpsampler.txt", std::ios::app);
    if (log_file.is_open()) {
        std::string result = (sampled_index < n) ? std::to_string(sampled_index) : "FAIL";
        log_file << result << "\n";
    }

    return 0;
}