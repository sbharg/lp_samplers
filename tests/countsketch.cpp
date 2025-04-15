#include "CountSketch.h"

#include <iostream>
#include <random>
#include <vector>

#include "cxxopts.hpp"

int main(int argc, char* argv[]) {
    cxxopts::Options options(argv[0], "Test program for CountSketch");
    options.add_options()("n,num",
                          "Size of frequency vector",
                          cxxopts::value<size_t>()->default_value("30"))("h,help", "print usage");

    size_t n;
    try {
        auto result = options.parse(argc, argv);
        if (result.count("help")) {
            std::cout << options.help() << "\n";
            std::exit(0);
        }

        n = result["num"].as<size_t>();
    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "Error parsing options: " << e.what() << std::endl;
        return 1;
    }

    int64_t seed = 88;
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int64_t> dist(-25, 25);

    // size_t n = 30;
    std::vector<int64_t> freq(n);
    for (size_t i = 0; i < n; ++i) {
        freq[i] = dist(rng);
    }

    size_t w = 25;
    size_t d = static_cast<size_t>(std::ceil(std::log2(n)));  // d = log2(n) + 1;
    CountSketch cs(w, d, true, 42);                           // d rows, w columns
    // Insert/update keys
    for (size_t i = 0; i < n; ++i) {
        cs.update(i, freq[i]);
    }

    // Query frequencies
    double error = 0;
    for (size_t i = 0; i < n; ++i) {
        error += std::abs(cs.estimate(i) - freq[i]);
    }
    std::cout << "Average error: " << error / n << std::endl;

    if (n <= 30) {
        // Print a table of the actual and esimated frequencies
        for (size_t i = 0; i < n; ++i) {
            std::cout << "Actual: " << freq[i] << " Estimated: " << cs.estimate(i) << std::endl;
        }
    } else {
        int x = 20;
    }

    return 0;
}
