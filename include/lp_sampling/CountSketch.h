#ifndef COUNT_SKETCH_H_
#define COUNT_SKETCH_H_

#include <cstdint>
#include <iostream>
#include <vector>

#include "KWiseHash.h"

class CountSketch {
  public:
    /**
     * Constructs a CountSketch data structure with width w and depth d.
     *
     * \param w The size of each row in the sketch. Assumes w < 2^61 -1.
     * \param d The number of hash/sign rows in the sketch. Defaults to 5.
     * \param seed The seed for the random number generator. Defaults to 42.
     * \param murmur Whether to use MurmurHash3 for hashing. Defaults to false.
     */
    CountSketch(size_t w, size_t d = 5, uint64_t seed = 42, bool murmur = false);

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

    std::vector<std::vector<int64_t>> table_;  // Sketch matrix of size d_ x w_

    std::vector<KWiseHash> index_hashes;
    std::vector<KWiseHash> sign_hashes;

    size_t idx_hash(const size_t i, const uint64_t key) const;
    int sign_hash(const size_t i, const uint64_t key) const;
};

#endif  // COUNT_SKETCH_H_
