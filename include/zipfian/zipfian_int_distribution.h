/*
This implementation is a direct port of [Apache Common's RejectionInversionZipfSampler][1].
Based on the method described in [Rejection-inversion to generate variates from Monotone discrete distributions][2]
by Wolfgang Hörmann and Gerhard Derflinger.

[1]: https://github.com/apache/commons-rng/blob/6a1b0c16090912e8fc5de2c1fb5bd8490ac14699/commons-rng-sampling/src/main/java/org/apache/commons/rng/sampling/distribution/RejectionInversionZipfSampler.java
[2]: https://dl.acm.org/citation.cfm?id=235029
*/

#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>

template <typename IntType = uint64_t>
class zipfian_int_distribution {
    static_assert(std::is_integral<IntType>::value, "Template argument not an integral type.");

  public:
    /**
     * Constructor.
     *
     * @param n Number of elements; must be positive.
     * @param s Exponent parameter; must be positive.
     * @throws std::invalid_argument if n_ <= 0 or exponent <= 0.
     */
    zipfian_int_distribution(size_t n, double s)
        : n_(n), s_(s), uniform_(0.0, 1.0) {
        if (n_ <= 0) {
            throw std::invalid_argument("Number of elements is out of range.");
        }
        if (s_ <= 0.0) {
            throw std::invalid_argument("Exponent is not strictly positive.");
        }

        // Precompute constants.
        h_integral_x_ = h_integral(1.5) - 1.0;
        h_integral_n_ = h_integral(static_cast<double>(n_) + 0.5);
        inv_const_ = 2.0 - h_integral_inverse(h_integral(2.5) - h(2.0));
    }

    /**
     * @brief Generating functions.
     */
    template <typename UniformRandomNumberGenerator>
    size_t operator()(UniformRandomNumberGenerator& urng) {
        return this->operator()(urng, n_);
    }

    template <typename UniformRandomNumberGenerator>
    size_t operator()(UniformRandomNumberGenerator& urng, size_t n) {
        while (true) {
            // Draw a uniform double in [0, 1) and map it to the interval (h_integral_n_, h_integral_x_].
            double u_ = uniform_(urng);
            double u = h_integral_n_ + u_ * (h_integral_x_ - h_integral_n_);
            double x = h_integral_inverse(u);
            int k = static_cast<int>(x + 0.5);

            // Clamp k to the valid range.
            if (k < 1) {
                k = 1;
            } else if (k > n_) {
                k = n_;
            }

            // Accept the candidate k if it meets the rejection criteria.
            if ((k - x <= inv_const_) || (u >= h_integral(k + 0.5) - h(k))) {
                return k;
            }
        }
    }

  private:
    int n_;                // The total number of elements.
    double s_;             // The Zipf exponent.
    double h_integral_x_;  // h_integral(1.5) - 1
    double h_integral_n_;  // h_integral(n_ + 0.5)
    double inv_const_;     // 2 - h_integral_inverse(h_integral(2.5) - h(2))
    std::uniform_real_distribution<double> uniform_;

    /**
     * Computes H(x) = helper2((1 - exponent) * log(x)) * log(x).
     * For exponent != 1, this is equivalent to (x^(1-exponent) - 1) / (1 - exponent),
     * while for exponent == 1 it yields log(x).
     */
    double h_integral(double x) const {
        double log_x = std::log(x);
        return helper2((1.0 - s_) * log_x) * log_x;
    }

    /**
     * Computes h(x) = 1 / x^exponent.
     */
    double h(double x) const {
        // Equivalent to exp(-exponent * log(x)).
        return std::exp(-s_ * std::log(x));
    }

    /**
     * Computes the inverse function of H, i.e., finds y such that H(y) = x.
     *
     * @param x The value to invert.
     * @return The result y = H⁻¹(x).
     */
    double h_integral_inverse(double x) const {
        double t = x * (1.0 - s_);
        if (t < -1.0) {
            // Limit t to -1 to avoid issues due to numerical errors.
            t = -1.0;
        }
        return std::exp(helper1(t) * x);
    }

    /**
     * Helper function: calculates ln(1 + x) / x.
     * Uses a Taylor series expansion when x is close to 0.
     *
     * @param x Value for computation.
     * @return The result of ln(1 + x)/x.
     */
    double helper1(double x) const {
        if (std::fabs(x) > 1e-8) {
            return std::log1p(x) / x;
        } else {
            return 1.0 - x * (0.5 - x * (0.33333333333333333 - 0.25 * x));
        }
    }

    /**
     * Helper function: calculates (exp(x) - 1) / x.
     * Uses a Taylor series expansion when x is close to 0.
     *
     * @param x Value for computation.
     * @return The result of (exp(x)-1)/x.
     */
    double helper2(double x) const {
        if (std::fabs(x) > 1e-8) {
            return std::expm1(x) / x;
        } else {
            return 1.0 + x * 0.5 * (1.0 + x * 0.33333333333333333 * (1.0 + 0.25 * x));
        }
    }
};