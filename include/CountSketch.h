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
     * \param d The number of hash/sign rows in the sketch.
     */
    CountSketch(size_t w, size_t d = 5, uint64_t seed = 42);

    /**
     * Modifies the CountSketch to handle stream updates of the form (key, delta).
     * For each i \in [d], updates table[j][h_i(key)] += sign_i(key) * delta.
     *
     * \param key The key whose frequency is being updated.
     * \param delta The change in frequency of the key.
     */
    void update(const uint64_t key, const int64_t delta);

    /**
     * Computes an estimate of the frequency of a given key.
     * For each i \in [d], the estimate of freq(key) is sign_i(key) * table[i][h_i(key)].
     * To boost accuracy, the median of these estimates is returned.
     *
     * \param key The key whose frequency is being estimated.
     * \return The median estimate of the frequency of the key.
     */
    int64_t estimate(const uint64_t key) const;

    friend std::ostream& operator<<(std::ostream& os, const CountSketch& cs);

   private:
    const size_t w_;  // size of row
    const size_t d_;  // number of hash/sign rows

    std::vector<std::vector<int64_t>> table_;  // Sketch matrix of size d x w

    const uint64_t PRIME = 2305843009213693951ULL;            // 2^61 - 1, large prime
    std::vector<std::pair<uint64_t, uint64_t>> index_params;  // Parameters for index hash functions
    std::vector<std::pair<uint64_t, uint64_t>> sign_params;   // Parameters for sign hash functions

    /**
     * A multiply-shift hash function that is 2-wise independent.
     * Returns the bucket that a key is hashed into for the i-th row.
     *
     * \param i The index of the row
     * \param key The key to hash.
     * \return The index of the column in the row that the key is hashed to.
     */
    size_t hash_idx(const size_t i, const uint64_t key) const;
    /**
     * A multiply-shift hash function that is 2-wise independent.
     * Returns the sign of the key for the i-th row.
     *
     * \param i The index of the row
     * \param key The key to hash.
     * \return Either 1 or -1.
     */
    int hash_sign(const size_t i, const uint64_t key) const;
};

#endif  // COUNT_SKETCH_H
