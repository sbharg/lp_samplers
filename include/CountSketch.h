#ifndef COUNT_SKETCH_H
#define COUNT_SKETCH_H

#include <cstdint>
#include <iostream>
#include <vector>

class CountSketch {
   public:
    /**
     * Constructs a CountSketch data structure with width w and depth d.
     *
     * \param w The size of each row in the sketch. Assumes w < 2^61 -1.
     * \param d The number of hash/sign rows in the sketch. Defaults to 5.
     * \param murmur Whether to use MurmurHash3 for hashing. Defaults to false.
     * \param seed The seed for the random number generator. Defaults to 42.
     */
    CountSketch(size_t w, size_t d = 5, bool murmur = false, uint64_t seed = 42);

    // Modifies the CountSketch to handle stream updates of the form (key, delta).
    void update(const uint64_t key, const int64_t delta);
    // Computes an estimate of the frequency of a given key.
    int64_t estimate(const uint64_t key) const;

    friend std::ostream& operator<<(std::ostream& os, const CountSketch& cs);

   private:
    const uint64_t seed_;
    const size_t w_;  // size of row
    const size_t d_;  // number of hash/sign rows
    const bool use_murmur_;

    std::vector<std::vector<int64_t>> table_;  // Sketch matrix of size d x w

    const uint64_t PRIME_ = 2305843009213693951ULL;           // 2^61 - 1, large prime
    std::vector<std::pair<uint64_t, uint64_t>> index_params;  // Parameters for index hash functions
    std::vector<std::pair<uint64_t, uint64_t>> sign_params;   // Parameters for sign hash functions

    // 2-wise independent hash functions using the multiply-shift method
    size_t hash_idx(const size_t i, const uint64_t key) const;
    int hash_sign(const size_t i, const uint64_t key) const;

    // Hash functions using MurmurHash3. Not 2-wise independent, but may be faster in practice
    size_t hash_idx_murmur(const size_t i, const uint64_t key) const;
    int hash_sign_murmur(const size_t i, const uint64_t key) const;
};

#endif  // COUNT_SKETCH_H
