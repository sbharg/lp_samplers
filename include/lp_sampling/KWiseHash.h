#ifndef K_WISE_HASH_H_
#define K_WISE_HASH_H_

#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

typedef __uint128_t uint128_t;  // 128-bit unsigned integer type

class KWiseHash {
  private:
    uint64_t k_;
    std::vector<std::vector<uint64_t>> a_;
    const uint64_t MP61 = (1ULL << 61) - 1;  // Large Mersenne prime

  public:
    KWiseHash(int k, uint64_t seed = std::random_device{}(), size_t i = 1)
        : k_(k), a_(i, std::vector<uint64_t>(k)) {
        std::mt19937_64 rng(seed);
        std::uniform_int_distribution<uint64_t> dist(0, MP61 - 1);

        for (size_t j = 0; j < i; j++)
            for (size_t l = 0; l < k_; l++)
                a_[j][l] = dist(rng);
    }

    ~KWiseHash() = default;

    // Default copy constructor (memberwise copy)
    KWiseHash(const KWiseHash& other) = default;

    // Copy‑assignment operator
    KWiseHash& operator=(const KWiseHash& other) {
        if (this != &other) {
            k_ = other.k_;
            a_ = other.a_;
        }
        return *this;
    }

    uint64_t hash(uint64_t x, size_t i = 0) const {
        uint64_t res = 0;
        // Horner’s method
        for (int j = k_ - 1; j >= 0; --j) {
            res = mul61(res, x);
            res += a_[i][j];
            if (res >= MP61) res -= MP61;
        }
        return res;
    }

  private:
    // Branchless reduction of a 128-bit value (hi:lo) modulo MP61
    uint64_t mod61(uint64_t hi, uint64_t lo) const {
        uint64_t lo61 = lo & MP61;
        uint64_t hi_part = (lo >> 61) + (hi << 3) + (hi >> 58);
        uint64_t sum = lo61 + hi_part;

        return sum >= MP61 ? sum - MP61 : sum;
    }

    // Fast multiplication mod MP61
    uint64_t mul61(uint64_t a, uint64_t b) const {
        uint128_t prod = static_cast<uint128_t>(a) * b;
        uint64_t lo = static_cast<uint64_t>(prod);
        uint64_t hi = static_cast<uint64_t>(prod >> 64);
        return mod61(hi, lo);
    }
};

#endif  // K_WISE_HASH_H_