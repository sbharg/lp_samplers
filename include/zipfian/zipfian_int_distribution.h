/*
This implementation is a direct port of Apache Common's RejectionInversionZipfSampler, written in Java.
It is based on the method described by Wolfgang Hörmann and Gerhard Derflinger in
[*Rejection-inversion to generate variates from Monotone discrete distributions*](https://dl.acm.org/citation.cfm?id=235029)
from *ACM Transactions on Modeling and Computer Simulation (TOMACS) 6.3 (1996)*.
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
        : n_(n), s_(s), uniformDist(0.0, 1.0) {
        if (n_ <= 0 || n_ > std::numeric_limits<IntType>::max()) {
            throw std::invalid_argument("Number of elements is out of range.");
        }
        if (s_ <= 0.0) {
            throw std::invalid_argument("Exponent is not strictly positive.");
        }

        // Precompute constants.
        h_integralX1 = h_integral(1.5) - 1.0;
        h_integral_n = h_integral(static_cast<double>(n_) + 0.5);
        s = 2.0 - h_integral_inverse(h_integral(2.5) - h(2.0));
    }

    /**
     * @brief Generating functions.
     */
    template <typename UniformRandomNumberGenerator>
    size_t operator()(UniformRandomNumberGenerator& urng) {
        return this->operator()(urng, n_, s_);
    }

    template <typename UniformRandomNumberGenerator>
    size_t operator()(UniformRandomNumberGenerator& urng, size_t n, double exponent) {
        while (true) {
            // Draw a uniform double in [0, 1) and map it to the interval (h_integralX1, h_integral_n].
            double u_ = std::generate_canonical<double, std::numeric_limits<double>::digits, UniformRandomNumberGenerator>(urng);
            double u = h_integral_n + u_ * (h_integralX1 - h_integral_n);
            double x = h_integral_inverse(u);
            int k = static_cast<int>(x + 0.5);

            // Clamp k to the valid range.
            if (k < 1) {
                k = 1;
            } else if (k > n_) {
                k = n_;
            }

            // Accept the candidate k if it meets the rejection criteria.
            if ((k - x <= s) || (u >= h_integral(k + 0.5) - h(k))) {
                return k;
            }
        }
    }

    /**
     * Generates a sample from a bounded Zipfian distribution using rejection inversion.
     *
     * @return An integer in the interval [1, n_] sampled according to the Zipf law.
     */
    int operator()(std::mt19937& rng) {
        while (true) {
            // Draw a uniform double in [0, 1) and map it to the interval (h_integralX1, h_integral_n].
            double u = h_integral_n + uniformDist(rng) * (h_integralX1 - h_integral_n);
            double x = h_integral_inverse(u);
            int k = static_cast<int>(x + 0.5);

            // Clamp k to the valid range.
            if (k < 1) {
                k = 1;
            } else if (k > n_) {
                k = n_;
            }

            // Accept the candidate k if it meets the rejection criteria.
            if ((k - x <= s) || (u >= h_integral(k + 0.5) - h(k))) {
                return k;
            }
        }
    }

  private:
    int n_;               // The total number of elements.
    double s_;            // The Zipf exponent.
    double h_integralX1;  // h_integral(1.5) - 1
    double h_integral_n;  // h_integral(n_ + 0.5)
    double s;             // 2 - _i(h_integral(2.5) - h(2))
    std::uniform_real_distribution<double> uniformDist;

    /**
     * Computes H(x) = helper2((1 - exponent) * log(x)) * log(x).
     * For exponent != 1, this is equivalent to (x^(1-exponent) - 1) / (1 - exponent),
     * while for exponent == 1 it yields log(x).
     */
    double h_integral(double x) const {
        double logX = std::log(x);
        return helper2((1.0 - s_) * logX) * logX;
    }

    /**
     * Computes h(x) = 1 / x^exponent.
     */
    double h(double x) const {
        // Equivalent to exp(-exponent * log(x)).
        return std::pow(x, -s_);
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
     * Helper function: calculates log(1 + x) / x.
     * Uses a Taylor series expansion when x is close to 0.
     *
     * @param x Value for computation.
     * @return The result of log1p(x)/x.
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