#include "CountSketch.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>

/**
 * Constructs a CountSketch data structure with width w and depth d.
 *
 * \param w The size of each row in the sketch. Assumes w < 2^61 -1.
 * \param d The number of hash/sign rows in the sketch.
 */
CountSketch::CountSketch(size_t w, size_t d, uint64_t seed)
    : w_(w), d_(d), table_(d, std::vector<int64_t>(w, 0)), index_params(d), sign_params(d) {
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<uint64_t> dist(1, PRIME - 1);

    for (size_t i = 0; i < d_; ++i) {
        // Generate random parameters for hash functions
        index_params[i] = std::make_pair(dist(rng), dist(rng));
        sign_params[i] = std::make_pair(dist(rng), dist(rng));
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

/**
 * A multiply-shift hash function that is 2-wise independent.
 * Returns the bucket that a key is hashed into for the i-th row.
 *
 * \param i The index of the row
 * \param key The key to hash.
 * \return The index of the column in the row that the key is hashed to.
 */
size_t CountSketch::hash_idx(const size_t i, const uint64_t key) const {
    auto [a, b] = index_params[i];
    uint64_t res = (a * key + b) % PRIME;
    return res % w_;
}

/**
 * A multiply-shift hash function that is 2-wise independent.
 * Returns the sign of the key for the i-th row.
 *
 * \param i The index of the row
 * \param key The key to hash.
 * \return Either 1 or -1.
 */
int CountSketch::hash_sign(const size_t i, const uint64_t key) const {
    auto [a, b] = sign_params[i];
    uint64_t res = (a * key + b) % PRIME;
    return (res & 1) ? -1 : 1;
}

/**
 * Modifies the CountSketch to handle stream updates of the form (key, delta).
 * For each i \in [d], updates table[j][h_i(key)] += sign_i(key) * delta.
 *
 * \param key The key whose frequency is being updated.
 * \param delta The change in frequency of the key.
 */
void CountSketch::update(const uint64_t key, const int64_t delta) {
    for (size_t i = 0; i < d_; ++i) {
        size_t idx = hash_idx(i, key);
        int sign = hash_sign(i, key);
        table_[i][idx] += sign * delta;
    }
}

/**
 * Computes an estimate of the frequency of a given key.
 * For each i \in [d], the estimate of freq(key) is sign_i(key) * table[i][h_i(key)].
 * To boost accuracy, the median of these estimates is returned.
 *
 * \param key The key whose frequency is being estimated.
 * \return The median estimate of the frequency of the key.
 */
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
