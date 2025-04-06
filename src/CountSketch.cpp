#include "CountSketch.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>

CountSketch::CountSketch(size_t w, size_t d, uint64_t seed)
    : w_(w), d_(d), table_(d, std::vector<int64_t>(w, 0)), index_params(d), sign_params(d) {
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<uint64_t> dist(1, PRIME - 1);

    for (size_t i = 0; i < d_; ++i) {
        uint64_t index_a = dist(rng);
        uint64_t index_b = dist(rng);
        uint64_t sign_a = dist(rng);
        uint64_t sign_b = dist(rng);
        index_params[i] = std::make_pair(index_a, index_b);
        sign_params[i] = std::make_pair(sign_a, sign_b);
    }
}

std::ostream& operator<<(std::ostream& os, const CountSketch& cs) {
    for (const auto& row : cs.table_) {
        for (const auto& val : row) {
            os << val << " ";
        }
        os << std::endl;
    }
    return os;
}

size_t CountSketch::hash_idx(const size_t i, const uint64_t key) const {
    auto [a, b] = index_params[i];
    uint64_t res = (a * key + b) % PRIME;
    return res % w_;
}

int CountSketch::hash_sign(const size_t i, const uint64_t key) const {
    auto [a, b] = sign_params[i];
    uint64_t res = (a * key + b) % PRIME;
    return (res & 1) ? -1 : 1;
}

void CountSketch::update(const uint64_t key, const int64_t delta) {
    for (size_t i = 0; i < d_; ++i) {
        size_t idx = hash_idx(i, key);
        int sign = hash_sign(i, key);
        table_[i][idx] += sign * delta;
    }
}

int64_t CountSketch::estimate(const uint64_t key) const {
    std::vector<int64_t> estimates;
    estimates.reserve(d_);

    for (size_t i = 0; i < d_; ++i) {
        size_t idx = hash_idx(i, key);
        int sign = hash_sign(i, key);
        estimates.push_back(sign * table_[i][idx]);
    }

    // Return median estimate
    std::nth_element(estimates.begin(), estimates.begin() + estimates.size() / 2, estimates.end());
    return estimates[estimates.size() / 2];
}
