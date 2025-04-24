#ifndef FP_ESTIMATOR_H_
#define FP_ESTIMATOR_H_

#include <cstdint>
#include <iostream>
#include <vector>

#include "KWiseHash.h"

class FpEstimator {
  public:
    virtual ~FpEstimator() = default;
    virtual void update(const uint64_t key, const double delta) = 0;
    virtual double estimate_norm() const = 0;
};

class F2Estimator : public FpEstimator {
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
    F2Estimator(double eps = 0.1,
                double delta = 0.01,
                uint64_t seed = 42,
                bool murmur = false);

    // Modifies the CountSketch to handle stream updates of the form (key, delta).
    void update(const uint64_t key, const double delta) override;
    // Computes an estimate of the frequency of a given key.
    double estimate_norm() const override;

    void subtract(const F2Estimator& other);

    friend std::ostream& operator<<(std::ostream& os, const F2Estimator& sketch);

  private:
    const size_t w_;  // size of row
    const double eps_;
    const double delta_;
    const uint64_t seed_;
    const bool use_murmur_;

    std::vector<double> table_;  // Sketch vector of size w_

    KWiseHash index_hash_;
    KWiseHash sign_hash_;

    size_t idx_hash(const uint64_t key) const;
    int sign_hash(const uint64_t key) const;
};

class cauchy_distribution {
  public:
    cauchy_distribution(uint64_t k, uint64_t seed);

    ~cauchy_distribution() = default;
    cauchy_distribution(const cauchy_distribution& other) = default;
    cauchy_distribution& operator=(const cauchy_distribution& other);

    double operator()(size_t i) const;

  private:
    uint64_t k_;      // k-wise indepedence parameter
    KWiseHash hash_;  // k-wise hash function for thetas
};

class F1Estimator : public FpEstimator {
  public:
    F1Estimator(double eps = 0.1, double delta = 0.01, uint64_t seed = 42);

    ~F1Estimator() = default;
    F1Estimator(const F1Estimator& other) = default;
    F1Estimator& operator=(const F1Estimator& other) = default;

    // Modifies the table to handle stream updates of the form (key, delta).
    void update(const uint64_t key, const double delta) override;
    // Computes an estimate of the frequency of a given key.
    double estimate_norm() const override;

    size_t get_w() const { return w_; }
    size_t get_eps() const { return eps_; }
    size_t get_delta() const { return delta_; }

  private:
    const size_t w_;  // size of row
    const double eps_;
    const double delta_;
    const uint64_t seed_;

    std::vector<cauchy_distribution> dists_;
    std::vector<double> table_;  // Sketch vector of size
};

#endif  // FP_ESTIMATOR_H_