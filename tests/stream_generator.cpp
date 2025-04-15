#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "cxxopts.hpp"

int main(int argc, char* argv[]) {
    // --- Configuration Variables ---
    size_t n;                     // Length of the underlying frequency vector (to be set by opts)
    size_t num_updates;           // Total number of updates to generate (to be set by opts)
    std::string output_filename;  // Name of the file to save updates (to be set by opts)
    const int MIN_VALUE = -100;   // Minimum update value (inclusive)
    const int MAX_VALUE = 100;    // Maximum update value (inclusive)

    // --- Setup Command Line Options ---
    cxxopts::Options options(
        argv[0],  // Program name for help message
        "Generates (index, value) updates for a frequency vector in the\n"
        "general turnstile model (values [" +
            std::to_string(MIN_VALUE) + ", " + std::to_string(MAX_VALUE) +
            "]).\n"
            "Outputs updates to a specified file, one 'index value' pair per line.");

    try {
        // clang-format off
        options.add_options()(
            "n,length", "Length of the underlying frequency vector (positive integer)",
            cxxopts::value<size_t>(n))(
            "u,updates", "Total number of updates to generate (positive integer)",
            cxxopts::value<size_t>(num_updates))(
            "o,output", "Output filename to save the updates",
            cxxopts::value<std::string>(output_filename))(
            "h,help", "Print usage information");
        // clang-format on

        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return 0;
        }

        // --- Validate Required Arguments ---
        if (!result.count("length") || !result.count("updates") || !result.count("output")) {
            throw std::runtime_error(
                "Error: Missing required arguments: --length, --updates, --output are mandatory.");
        }

        if (n <= 0) {
            throw std::runtime_error(
                "Length of the frequency vector (n) must be a positive integer.");
        }
        if (num_updates <= 0) {
            throw std::runtime_error("Number of updates must be a positive integer.");
        }
        if (output_filename.empty()) {
            throw cxxopts::exceptions::option_requires_argument("output");
        }

    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << "Use --help for usage information." << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }

    // --- Arguments successfully parsed and validated ---
    std::cout << "--- Turnstile Stream Generator ---" << std::endl;
    std::cout << "Configuration:" << std::endl;
    std::cout << "  Vector Length (n): " << n << std::endl;
    std::cout << "  Number of Updates: " << num_updates << std::endl;
    std::cout << "  Update Value Range: [" << MIN_VALUE << ", " << MAX_VALUE << "]" << std::endl;
    std::cout << "  Output File: '" << output_filename << "'" << std::endl;

    // --- Setup Random Number Generation ---
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<long long> index_dist(0, n - 1);
    std::uniform_int_distribution<int> value_dist(MIN_VALUE, MAX_VALUE);

    // --- Open Output File ---
    std::ofstream outfile(output_filename);
    if (!outfile.is_open()) {
        std::cerr << "\nError: Could not open output file '" << output_filename << "' for writing."
                  << std::endl;
        return 1;
    }

    std::cout << "\nGenerating updates..." << std::endl;

    // --- Generate and Write Updates ---
    for (long long k = 0; k < num_updates; ++k) {
        long long index = index_dist(rng);
        int value = value_dist(rng);
        outfile << index << " " << value << "\n";
    }

    // --- Cleanup ---
    outfile.close();

    std::cout << "Successfully generated stream and saved to '" << output_filename << "'."
              << std::endl;

    return 0;  // Indicate successful execution
}