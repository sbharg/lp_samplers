#include "FpEstimator.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>

#include "KWiseHash.h"
#include "MurmurHash3.h"

F2Estimator::F2Estimator(double eps, double delta, uint64_t seed, bool murmur)
    : w_(6 / (eps * eps * delta)),
      eps_(eps),
      delta_(delta),
      seed_(seed),
      use_murmur_(murmur),
      table_(w_),
      index_hash_(2, seed_),
      sign_hash_(4, seed_ + 20) {}

std::ostream& operator<<(std::ostream& os, const F2Estimator& sketch) {
    if (sketch.table_.size() <= 25) {
        for (const auto& val : sketch.table_) {
            os << val << " ";
        }
        os << std::endl;
    }
    return os;
}

/**
 * A hash function that returns the bucket that a key is hashed into.
 * If use_murmur_ is true, uses MurmurHash3. Otherwise, uses a simple multiply-shift hash
 * function. MurmurHash3 is not 2-wise independent, but may be faster in practice.
 * Multiply-shift is 2-wise independent, but may be slower in practice.
 *
 * \param key The key to hash.
 * \return The index of the column in the row that the key is hashed to.
 */
size_t F2Estimator::idx_hash(const uint64_t key) const {
    uint64_t res = 0;
    if (use_murmur_) {
        res = murmur_hash3_64(key, seed_);
    } else {
        res = index_hash_.hash(key);
    }
    return res % w_;
}

/**
 * A hash function that returns the sign of the key.
 * If use_murmur_ is true, uses MurmurHash3. Otherwise, uses a simple multiply-shift hash
 * function. MurmurHash3 is not 2-wise independent, but may be faster in practice.
 * Multiply-shift is 2-wise independent, but may be slower in practice.
 *
 * \param key The key to hash.
 * \return Either 1 or -1.
 */
int F2Estimator::sign_hash(const uint64_t key) const {
    uint64_t res = 0;
    if (use_murmur_) {
        res = murmur_hash3_64(key, seed_ + 20);
    } else {
        res = sign_hash_.hash(key);
    }
    return (res & 1) ? -1 : 1;
}

/**
 * Modifies the CountSketch to handle stream updates of the form (key, delta).
 * Updates table[h(key)] += sign(key) * delta.
 *
 * \param key The key whose frequency is being updated.
 * \param delta The change in frequency of the key.
 */
void F2Estimator::update(const uint64_t key, const int64_t delta) {
    size_t idx = idx_hash(key);
    int sign = sign_hash(key);
    table_[idx] += sign * delta;
}

/**
 * Computes an estimate of the square of the l2 norm of the frequency vector.
 * For each i \in [d], the estimate of freq(key) is sign_i(key) * table[i][h_i(key)].
 * To boost accuracy, the median of these estimates is returned.
 *
 * \return The l2 norm estimate.
 */
double F2Estimator::estimate_norm() const {
    double estimate = 0;

    for (const auto& val : table_) {
        estimate += val * val;
    }

    return sqrt(estimate);
}

cauchy_distribution::cauchy_distribution(uint64_t k, uint64_t seed)
    : k_(k), hash_(k, seed) {}

cauchy_distribution& cauchy_distribution::operator=(const cauchy_distribution& other) {
    if (this != &other) {
        k_ = other.k_;
        hash_ = other.hash_;
    }
    return *this;
}

double cauchy_distribution::operator()(size_t i) const {
    std::mt19937_64 rng(i);
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

    double theta = hash_.hash(dist(rng)) / static_cast<double>(hash_.get_mp());
    theta = (theta - 0.5) * M_PI;  // Uni(-π/2, π/2)
    return std::tan(theta);
}

F1Estimator::F1Estimator(double eps, double delta, uint64_t seed)
    : w_(static_cast<size_t>(std::ceil(3 / (eps * eps) * -std::log(delta))) & 1
             ? static_cast<size_t>(std::ceil(3 / (eps * eps) * -std::log(delta)))
             : static_cast<size_t>(std::ceil(3 / (eps * eps) * -std::log(delta))) + 1),
      eps_(eps),
      delta_(delta),
      seed_(seed),
      table_(w_) {
    dists_.reserve(w_);
    KWiseHash seed_hash(2, seed_);
    std::random_device rd;
    std::mt19937_64 rng(rd());
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

    for (size_t i = 0; i < w_; ++i) {
        dists_.emplace_back(
            static_cast<uint64_t>(std::ceil((1 / eps) * std::pow(-std::log(eps), 3))),
            seed_hash.hash(dist(rng)));
    }
}

/**
 * Modifies the table to handle stream updates of the form (key, delta).
 * Updates table[i] += hash_i(key) * delta for each i \in [w_].
 *
 * \param key The key whose frequency is being updated.
 * \param delta The change in frequency of the key.
 */
void F1Estimator::update(const uint64_t key, const int64_t delta) {
    for (size_t i = 0; i < w_; ++i) {
        double cauchy_rv = dists_[i](key);
        table_[i] += delta * cauchy_rv;
    }
}

/**
 * Computes an estimate of the square of the l2 norm of the frequency vector.
 * For each i \in [d], the estimate of freq(key) is sign_i(key) * table[i][h_i(key)].
 * To boost accuracy, the median of these estimates is returned.
 *
 * \return The l1 norm estimate.
 */
double F1Estimator::estimate_norm() const {
    std::vector<int64_t> estimates;
    estimates.reserve(w_);

    std::transform(
        table_.begin(), table_.end(), std::back_inserter(estimates), [](const auto& val) {
            return std::abs(val);
        });

    // Return median estimate
    std::nth_element(
        estimates.begin(), estimates.begin() + estimates.size() / 2, estimates.end());
    return estimates[estimates.size() / 2];
}