#ifndef LP_SAMPLER_H_
#define LP_SAMPLER_H_

#include <cstdint>
#include <memory>
#include <optional>
#include <random>

#include "CountSketch.h"
#include "FpEstimator.h"
#include "KWiseHash.h"

class LpSampler {
  public:
    LpSampler(uint16_t p, double eps, double delta, uint64_t n, uint64_t seed = 42);
    ~LpSampler() = default;

    void update(const uint64_t i, const double delta);
    std::optional<uint64_t> sample() const;

  private:
    uint16_t p_;
    double eps_;
    double delta_;
    uint64_t n_;  // number of possible keys
    uint64_t seed_;

    uint64_t m_;  // width of CountSketch

    KWiseHash scalars_;  // Hash function for sampling uni variables
    std::unique_ptr<CountSketch> cs_;
    std::unique_ptr<FpEstimator> fp_;      // Fp sketch for Lp norm of x
    std::unique_ptr<F2Estimator> f2_err_;  // F2 sketch for L2 norm of z - z_hat
};

#endif  // LP_SAMPLER_H_