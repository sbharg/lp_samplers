#include "KWiseHash.h"

#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

KWiseHash::KWiseHash(int k, uint64_t seed)
    : k_(k), a_(k) {
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<uint64_t> dist(0, MP61 - 1);

    for (size_t j = 0; j < k_; j++)
        a_[j] = dist(rng);
}

KWiseHash& KWiseHash::operator=(const KWiseHash& other) {
    if (this != &other) {
        k_ = other.k_;
        a_ = other.a_;
    }
    return *this;
}

uint64_t KWiseHash::hash(uint64_t x) const {
    uint64_t res = 0;
    // Horner’s method
    for (int j = k_ - 1; j >= 0; --j) {
        res = mul61(res, x);
        res += a_[j];
        if (res >= MP61) res -= MP61;
    }
    return res;
}

uint64_t KWiseHash::mod61(uint64_t hi, uint64_t lo) const {
    uint64_t lo61 = lo & MP61;
    uint64_t hi_part = (lo >> 61) + (hi << 3) + (hi >> 58);
    uint64_t sum = lo61 + hi_part;

    return sum >= MP61 ? sum - MP61 : sum;
}

uint64_t KWiseHash::mul61(uint64_t a, uint64_t b) const {
    __uint128_t prod = static_cast<__uint128_t>(a) * b;
    uint64_t lo = static_cast<uint64_t>(prod);
    uint64_t hi = static_cast<uint64_t>(prod >> 64);
    return mod61(hi, lo);
}