#include <cmath>
#include <iostream>

#include "FpEstimator.h"

int main() {
    std::random_device rd;
    uint64_t seed = rd();
    std::cout << "Random seed: " << seed << std::endl;

    F2Estimator sketch_f2(0.1, 0.01, seed, false);
    F1Estimator sketch_f1(0.125, 0.01, seed);
    std::cout << "Constructed Sketches" << std::endl;
    std::cout << "F1 Sketch size: " << sketch_f1.get_w() << "\n\n";

    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int64_t> dist(-25, 25);

    size_t n = 30;
    std::vector<int64_t> freq(n);
    for (size_t i = 0; i < n; ++i) {
        freq[i] = dist(rng);
    }

    double l1_norm = 0;
    double l2_norm = 0;
    for (size_t i = 0; i < n; ++i) {
        l1_norm += static_cast<double>(std::abs(freq[i]));
        l2_norm += static_cast<double>(freq[i] * freq[i]);
    }
    l2_norm = sqrt(l2_norm);

    // Insert/update keys
    for (size_t i = 0; i < n; ++i) {
        sketch_f2.update(i, freq[i]);
        sketch_f1.update(i, freq[i]);
    }

    // Estimate l2 norm
    std::cout << "Estimate for l2 norm: " << sketch_f2.estimate_norm() << std::endl;
    std::cout << "Actual l2 norm: " << l2_norm << std::endl;

    std::cout << "Estimate for l1 norm: " << sketch_f1.estimate_norm() << std::endl;
    std::cout << "Actual l1 norm: " << l1_norm << std::endl;

    return 0;
}
