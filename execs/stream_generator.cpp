#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "cxxopts.hpp"
#include "zipfian_int_distribution.h"

int main(int argc, char* argv[]) {
    // --- Configuration Variables ---
    size_t n;                     // Length of the vector (required)
    size_t num_updates;           // Number of updates
    std::string output_filename;  // Output filename (required)
    std::string mode;             // Generation mode ("stream" or "zipfian")
    double zipf_s;                // Zipfian exponent (only for zipfian mode)

    const int STREAM_MIN_VALUE = -100;  // Min update value for stream mode
    const int STREAM_MAX_VALUE = 100;   // Max update value for stream mode

    // --- Setup Command Line Options ---
    // clang-format off
    cxxopts::Options options(
        argv[0],
        "Generates updates for a frequency vector.\n"
        "Modes:\n"
        "  stream: Generates (index, value) pairs for the turnstile model.\n"
        "          Values are in [" + std::to_string(STREAM_MIN_VALUE) + ", " + std::to_string(STREAM_MAX_VALUE) + "].\n"
        "  zipfian: Generates n items according to Zipf's law (exponent s),\n"
        "           outputs the final frequency vector.");
    // clang-format on

    try {
        // clang-format off
        options.add_options()
            ("n,length", "Length of the underlying frequency vector (positive integer)",
            cxxopts::value<size_t>(n))
            ("u,updates","Total number of stream updates to generate (required for 'stream' mode)",
            cxxopts::value<size_t>(num_updates))
            ("o,output", "Output filename to save the data",
            cxxopts::value<std::string>(output_filename))
            ("m,mode", "Generation mode: 'stream' (default) or 'zipfian'",
            cxxopts::value<std::string>(mode)->default_value("stream"))
            ("s,zipf-s", "Zipfian distribution exponent (s > 0, used only for 'zipfian' mode)",
             cxxopts::value<double>(zipf_s)->default_value("1"))
            ("h,help","Print usage information");
        // clang-format on

        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return 0;
        }

        if (!result.count("length")) {
            throw cxxopts::exceptions::exception("Error: Missing required argument: --length (-n)");
        } else if (!result.count("updates")) {
            throw cxxopts::exceptions::exception("Error: Missing required argument: --updates (-u)");
        } else if (!result.count("output")) {
            throw cxxopts::exceptions::exception("Error: Missing required argument: --output (-o)");
        }

        n = result["length"].as<size_t>();
        num_updates = result["updates"].as<size_t>();
        output_filename = result["output"].as<std::string>();

        mode = result["mode"].as<std::string>();
        zipf_s = result["zipf-s"].as<double>();

        // --- Validate Required Arguments and Values ---
        if (n <= 0) {
            throw cxxopts::exceptions::exception("Error: Vector length 'n' (--length) must be a positive integer.");
        } else if (result["updates"].as<size_t>() <= 0) {
            throw cxxopts::exceptions::exception("Error: Number of updates 'u' (--updates) must be a positive integer.");
        } else if (output_filename.empty()) {
            throw cxxopts::exceptions::option_requires_argument("--output");
        } else if (mode != "stream" && mode != "zipfian") {
            throw cxxopts::exceptions::exception("Error: Invalid mode '" + mode + "'. Choose 'stream' or 'zipfian'.");
        } else if (mode == "zipfian" && zipf_s <= 0.0) {
            throw cxxopts::exceptions::exception("Error: Zipfian exponent 's' (--zipf-s) must be positive for 'zipfian' mode.");
        }
    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "Error parsing options: " << e.what() << std::endl;
        std::cerr << "Use --help for usage information." << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected standard error occurred: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "--- Data Generator ---" << std::endl;
    std::cout << "Configuration:" << std::endl;
    std::cout << "  Mode: " << mode << std::endl;
    std::cout << "  Vector Length (n): " << n << std::endl;
    if (mode == "stream") {
        std::cout << "  Number of Stream Updates: " << num_updates << std::endl;
        std::cout << "  Stream Value Range: [" << STREAM_MIN_VALUE << ", " << STREAM_MAX_VALUE << "]" << std::endl;
    } else {  // zipfian mode
        std::cout << "  Total Frequency: " << num_updates << std::endl;
        std::cout << "  Zipfian Exponent (s): " << zipf_s << std::endl;
    }
    std::cout << "  Output File: '" << output_filename << "'" << std::endl;

    std::ofstream outfile(output_filename);
    if (!outfile.is_open()) {
        std::cerr << "\nError: Could not open output file '" << output_filename << "' for writing." << std::endl;
        return 1;
    }

    std::cout << "\nGenerating data..." << std::endl;

    std::random_device rd;
    std::mt19937 rng(rd());

    if (mode == "stream") {
        std::uniform_int_distribution<size_t> index_dist(0, n - 1);
        std::uniform_int_distribution<int> value_dist(STREAM_MIN_VALUE, STREAM_MAX_VALUE);

        outfile << "# " << n << " " << num_updates << "\n";
        for (size_t k = 0; k < num_updates; ++k) {
            size_t index = index_dist(rng);
            int value = value_dist(rng);
            outfile << index << " " << value << "\n";
            if (!outfile) {  // Check for write errors
                std::cerr << "\nError: Failed to write to output file '" << output_filename << "'. Disk full?" << std::endl;
                outfile.close();
                return 1;
            }
        }
        std::cout << "Successfully generated " << num_updates << " stream updates." << std::endl;

    } else {  // mode == "zipfian"
        zipfian_int_distribution<u_int64_t> zipf_dist(n, zipf_s);
        std::vector<u_int64_t> frequencies(n, 0);

        for (size_t k = 0; k < num_updates; ++k) {
            u_int64_t generated_value = zipf_dist(rng);  // Generates value in [1, n]
            frequencies[generated_value - 1]++;
        }

        outfile << "# " << n << " " << num_updates << " " << zipf_s << "\n";
        for (size_t i = 0; i < n; ++i) {
            outfile << i << " " << frequencies[i] << "\n";
            if (!outfile) {  // Check for write errors
                std::cerr << "\nError: Failed to write frequency data to output file '" << output_filename << "'. Disk full?" << std::endl;
                outfile.close();
                return 1;
            }
        }
        std::cout << "Successfully generated Zipfian vector for " << n << " items with total frequency" << num_updates << "." << std::endl;
    }

    outfile.close();
    std::cout << "Output saved to '" << output_filename << "'." << std::endl;

    return 0;
}