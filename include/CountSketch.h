#ifndef COUNT_SKETCH_H
#define COUNT_SKETCH_H

#include <cstdint>
#include <random>
#include <vector>

class CountSketch {
   public:
    CountSketch(size_t d, size_t w, uint64_t seed = 42);

    // update frequency of key by delta
    void update(int64_t key, int64_t delta);

    // estimate frequency of key
    int64_t estimate(int64_t key) const;

   private:
    size_t d_;  // number of hash/sign rows
    size_t w_;  // number of columns per row

    std::vector<std::vector<int64_t>> table_;  // sketch table

    std::vector<uint64_t> hash_a_, hash_b_;  // hash params
    std::vector<uint64_t> sign_a_, sign_b_;  // sign hash params

    static constexpr uint64_t PRIME = 2305843009213693951ULL;  // 2^61 - 1, large prime

    // 4-universal hash functions for indices and signs
    size_t hash_idx(size_t i, int64_t key) const;
    int hash_sign(size_t i, int64_t key) const;
};

#endif  // COUNT_SKETCH_H
