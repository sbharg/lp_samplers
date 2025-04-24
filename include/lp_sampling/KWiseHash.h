#ifndef K_WISE_HASH_H_
#define K_WISE_HASH_H_

#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

class KWiseHash {
  private:
    uint64_t k_;
    std::vector<uint64_t> a_;
    const uint64_t MP61 = (1ULL << 61) - 1;  // Large Mersenne prime

  public:
    KWiseHash(uint64_t k, uint64_t seed = std::random_device{}());

    ~KWiseHash() = default;

    // Default copy constructor (memberwise copy)
    KWiseHash(const KWiseHash& other) = default;

    // Copy‑assignment operator
    KWiseHash& operator=(const KWiseHash& other);

    uint64_t hash(uint64_t x) const;

    uint64_t get_mp() const { return MP61; }

  private:
    // Branchless reduction of a 128-bit value (hi:lo) modulo MP61
    uint64_t mod61(uint64_t hi, uint64_t lo) const;

    // Fast multiplication mod MP61
    uint64_t mul61(uint64_t a, uint64_t b) const;
};

#endif  // K_WISE_HASH_H_