#include "CountSketch.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>

#include "MurmurHash3.h"

CountSketch::CountSketch(size_t w, size_t d, uint64_t seed, bool murmur)
    : w_(w), d_(d), seed_(seed), use_murmur_(murmur), table_(d, std::vector<int64_t>(w, 0)) {
    if (!use_murmur_) {
        index_params.reserve(d_);
        sign_params.reserve(d_);

        std::mt19937_64 rng(seed);
        std::uniform_int_distribution<uint64_t> dist(1, 2147483647);

        // Generate random parameters for hash functions
        for (size_t i = 0; i < d_; ++i) {
            index_params.emplace_back(dist(rng), dist(rng));
            sign_params.emplace_back(dist(rng), dist(rng));
        }
    }
}

std::ostream& operator<<(std::ostream& os, const CountSketch& cs) {
    if (cs.table_[0].size() <= 25) {
        for (const auto& row : cs.table_) {
            for (const auto& val : row) {
                os << val << " ";
            }
            os << std::endl;
        }
    }
    return os;
}

/**
 * A hash function that returns the bucket that a key is hashed into for the i-th row.
 * If use_murmur_ is true, uses MurmurHash3. Otherwise, uses a simple multiply-shift hash function.
 * MurmurHash3 is not 2-wise independent, but may be faster in practice.
 * Multiply-shift is 2-wise independent, but may be slower in practice.
 *
 * \param i The index of the row
 * \param key The key to hash.
 * \return The index of the column in the row that the key is hashed to.
 */
size_t CountSketch::idx_hash(const size_t i, const uint64_t key) const {
    uint64_t res = 0;
    if (use_murmur_) {
        res = murmur_hash3_64(key, seed_ + i);
    } else {
        auto [a, b] = index_params[i];
        res = mod_prime(a * key + b);
    }
    return res % w_;
}

/**
 * A hash function that returns the sign of the key for the i-th row.
 * If use_murmur_ is true, uses MurmurHash3. Otherwise, uses a simple multiply-shift hash function.
 * MurmurHash3 is not 2-wise independent, but may be faster in practice.
 * Multiply-shift is 2-wise independent, but may be slower in practice.
 *
 * \param i The index of the row
 * \param key The key to hash.
 * \return Either 1 or -1.
 */
int CountSketch::sign_hash(const size_t i, const uint64_t key) const {
    uint64_t res = 0;
    if (use_murmur_) {
        res = murmur_hash3_64(key, seed_ + 2 * i);
    } else {
        auto [a, b] = sign_params[i];
        res = mod_prime(a * key + b);
    }
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
        size_t idx = idx_hash(i, key);
        int sign = sign_hash(i, key);
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
    std::vector<int64_t> estimates(d_);

    for (size_t i = 0; i < d_; ++i) {
        size_t idx = idx_hash(i, key);
        int sign = sign_hash(i, key);
        estimates[i] = sign * table_[i][idx];
    }

    // Return median estimate
    std::nth_element(estimates.begin(), estimates.begin() + estimates.size() / 2, estimates.end());
    return estimates[estimates.size() / 2];
}
