#include <LpSampler.h>

#include <atomic>
#include <cmath>
#include <cstdint>
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

    size_t num_samplers = static_cast<size_t>(10 * (1.0 / eps) * -std::log(delta));
    size_t num_threads = std::min<size_t>(std::thread::hardware_concurrency(), num_samplers);
    if (num_threads == 0) num_threads = 4; // fallback
    size_t samplers_per_thread = (num_samplers + num_threads - 1) / num_threads;

    std::atomic<bool> found_sample(false);
    std::atomic<size_t> sampled_index(n); // n is invalid
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
    };

    for (size_t t = 0; t < num_threads; ++t) {
        threads.emplace_back(sampler_task, t);
    }

    while (!found_sample) {
        std::this_thread::yield();
    }
    for (auto& t : threads) {
        t.request_stop();
    }

    if (sampled_index < n) {
        std::cout << "Sampled index: " << sampled_index << std::endl;
    } else {
        std::cout << "Failed to sample" << std::endl;
    }

    return 0;
}