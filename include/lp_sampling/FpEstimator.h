#ifndef FP_ESTIMATOR_H_
#define FP_ESTIMATOR_H_

#include <cstdint>
#include <iostream>
#include <vector>

class F2Estimator {
  public:
    /**
     * Constructs a CountSketch data structure with a single row of width 6 * (eps^2 *
     * delta)^{-1}.
     *
     * \param eps The desired error rate. Defaults to 0.1.
     * \param delta The desired failure probability. Defaults to 0.01.
     * \param seed The seed for the random number generator. Defaults to 42.
     * \param murmur Whether to use MurmurHash3 for hashing. Defaults to false.
     */
    F2Estimator(double eps = 0.1, double delta = 0.01, uint64_t seed = 42, bool murmur = false);

    // Modifies the CountSketch to handle stream updates of the form (key, delta).
    void update(const uint64_t key, const int64_t delta);
    // Computes an estimate of the frequency of a given key.
    double estimate_norm() const;

    friend std::ostream& operator<<(std::ostream& os, const F2Estimator& sketch);

  private:
    const size_t w_;  // size of row
    const double eps_;
    const double delta_;
    const uint64_t seed_;
    const bool use_murmur_;

    std::vector<int64_t> table_;  // Sketch matrix of size d_ x w_

    struct sign_hash_params {
        uint64_t a_1;
        uint64_t a_2;
        uint64_t a_3;
        uint64_t a_4;
    };

    const uint64_t PRIME_ = (1ULL << 61) - 1;    // 2^61 - 1, large Mersenne prime
    std::pair<uint64_t, uint64_t> index_params;  // Parameters for MS index hash functions
    sign_hash_params sign_params;                // Parameters for MS sign hash functions
    uint64_t mod_prime(uint64_t x) const {
        uint64_t res = (x >> 61) + (x & PRIME_);
        return (res >= PRIME_) ? res - PRIME_ : res;
    }

    size_t idx_hash(const uint64_t key) const;
    int sign_hash(const uint64_t key) const;
};

#endif  // FP_ESTIMATOR_H_