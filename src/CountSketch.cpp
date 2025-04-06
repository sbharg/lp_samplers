#include "CountSketch.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <random>

#include "MurmurHash3.h"

CountSketch::CountSketch(size_t w, size_t d, bool murmur, uint64_t seed)
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
 * A multiply-shift hash function that is 2-wise independent.
 * Returns the bucket that a key is hashed into for the i-th row.
 *
 * \param i The index of the row
 * \param key The key to hash.
 * \return The index of the column in the row that the key is hashed to.
 */
size_t CountSketch::hash_idx(const size_t i, const uint64_t key) const {
    auto [a, b] = index_params[i];
    uint64_t res = (a * key + b) % PRIME_;
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
    uint64_t res = (a * key + b) % PRIME_;
    return (res & 1) ? -1 : 1;
}

/**
 * A hash function using MurmurHash3 that is not 2-wise independent, but may be faster in practice.
 * Returns the bucket that a key is hashed into for the i-th row.
 *
 * \param i The index of the row
 * \param key The key to hash.
 * \return The index of the column in the row that the key is hashed to.
 */
size_t CountSketch::hash_idx_murmur(const size_t i, const uint64_t key) const {
    uint64_t res = murmur_hash3_64(key, seed_ + i);
    return res % w_;
}

/**
 * A hash function using MurmurHash3 that is not 2-wise independent, but may be faster in practice.
 * Returns the sign of the key for the i-th row.
 *
 * \param i The index of the row
 * \param key The key to hash.
 * \return Either 1 or -1.
 */
int CountSketch::hash_sign_murmur(const size_t i, const uint64_t key) const {
    uint64_t res = murmur_hash3_64(key, seed_ + 2 * i);
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
    std::function<size_t(size_t, uint64_t)> idx_hash;
    std::function<int(size_t, uint64_t)> sign_hash;

    if (!use_murmur_) {
        idx_hash = [this](size_t i, uint64_t key) { return hash_idx(i, key); };
        sign_hash = [this](size_t i, uint64_t key) { return hash_sign(i, key); };
    } else {
        idx_hash = [this](size_t i, uint64_t key) { return hash_idx_murmur(i, key); };
        sign_hash = [this](size_t i, uint64_t key) { return hash_sign_murmur(i, key); };
    }

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
    std::function<size_t(size_t, uint64_t)> idx_hash;
    std::function<int(size_t, uint64_t)> sign_hash;

    if (!use_murmur_) {
        idx_hash = [this](size_t i, uint64_t key) { return hash_idx(i, key); };
        sign_hash = [this](size_t i, uint64_t key) { return hash_sign(i, key); };
    } else {
        idx_hash = [this](size_t i, uint64_t key) { return hash_idx_murmur(i, key); };
        sign_hash = [this](size_t i, uint64_t key) { return hash_sign_murmur(i, key); };
    }

    std::vector<int64_t> estimates;
    estimates.reserve(d_);

    for (size_t i = 0; i < d_; ++i) {
        size_t idx = idx_hash(i, key);
        int sign = sign_hash(i, key);
        estimates.push_back(sign * table_[i][idx]);
    }

    // Return median estimate
    std::nth_element(estimates.begin(), estimates.begin() + estimates.size() / 2, estimates.end());
    return estimates[estimates.size() / 2];
}
