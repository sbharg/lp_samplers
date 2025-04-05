#include "CountSketch.h"

#include <algorithm>
#include <cmath>

CountSketch::CountSketch(size_t d, size_t w, uint64_t seed)
    : d_(d), w_(w), table_(d, std::vector<int64_t>(w, 0)), hash_a_(d), hash_b_(d), sign_a_(d), sign_b_(d) {
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<uint64_t> dist(1, PRIME - 1);

    for (size_t i = 0; i < d_; ++i) {
        hash_a_[i] = dist(rng);
        hash_b_[i] = dist(rng);
        sign_a_[i] = dist(rng);
        sign_b_[i] = dist(rng);
    }
}

size_t CountSketch::hash_idx(size_t i, int64_t key) const {
    uint64_t k = static_cast<uint64_t>(key);
    uint64_t res = (hash_a_[i] * k + hash_b_[i]) % PRIME;
    return res % w_;
}

int CountSketch::hash_sign(size_t i, int64_t key) const {
    uint64_t k = static_cast<uint64_t>(key);
    uint64_t res = (sign_a_[i] * k + sign_b_[i]) % PRIME;
    return (res & 1) ? -1 : 1;
}

void CountSketch::update(int64_t key, int64_t delta) {
    for (size_t i = 0; i < d_; ++i) {
        size_t idx = hash_idx(i, key);
        int sign = hash_sign(i, key);
        table_[i][idx] += sign * delta;
    }
}

int64_t CountSketch::estimate(int64_t key) const {
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
