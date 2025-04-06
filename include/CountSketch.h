#ifndef COUNT_SKETCH_H
#define COUNT_SKETCH_H

#include <cstdint>
#include <iostream>
#include <vector>

class CountSketch {
   public:
    CountSketch(size_t w, size_t d = 5, uint64_t seed = 42);

    // Modifies the CountSketch to handle stream updates of the form (key, delta).
    void update(const uint64_t key, const int64_t delta);
    // Computes an estimate of the frequency of a given key.
    int64_t estimate(const uint64_t key) const;

    friend std::ostream& operator<<(std::ostream& os, const CountSketch& cs);

   private:
    const size_t w_;  // size of row
    const size_t d_;  // number of hash/sign rows

    std::vector<std::vector<int64_t>> table_;  // Sketch matrix of size d x w

    const uint64_t PRIME = 2305843009213693951ULL;            // 2^61 - 1, large prime
    std::vector<std::pair<uint64_t, uint64_t>> index_params;  // Parameters for index hash functions
    std::vector<std::pair<uint64_t, uint64_t>> sign_params;   // Parameters for sign hash functions

    // 2-wise independent hash functions using the multiply-shift method
    size_t hash_idx(const size_t i, const uint64_t key) const;
    int hash_sign(const size_t i, const uint64_t key) const;
};

#endif  // COUNT_SKETCH_H
