#include "CountSketch.h"

#include <iostream>
#include <random>
#include <vector>

int main(int argc, char* argv[]) {
    int64_t seed = 88;
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int64_t> dist(-25, 25);

    size_t n = 30;
    std::vector<int64_t> freq(n);
    for (size_t i = 0; i < n; ++i) {
        freq[i] = dist(rng);
    }

    size_t w = 25;                                            // width of the sketch
    size_t d = static_cast<size_t>(std::ceil(std::log2(n)));  // d = log2(n) + 1;
    CountSketch cs(w, d, 42, false);                          // d rows, w columns

    std::cout << "Constructed cs" << std::endl;
    // Insert/update keys
    for (size_t i = 0; i < n; ++i) {
        cs.update(i, freq[i]);
    }

    std::cout << cs << std::endl;

    // Query frequencies
    double error = 0;
    for (size_t i = 0; i < n; ++i) {
        error += std::abs(cs.estimate(i) - freq[i]);
    }
    std::cout << "Average error: " << error / n << std::endl;

    if (n <= 30) {
        // Print a table of the actual and esimated frequencies
        for (size_t i = 0; i < n; ++i) {
            std::cout << "Actual: " << freq[i] << " Estimated: " << cs.estimate(i)
                      << std::endl;
        }
    } else {
        int x = 20;
    }

    return 0;
}
