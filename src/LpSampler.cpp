#include "LpSampler.h"

#include <cmath>
#include <cstdint>
#include <optional>
#include <queue>

#include "CountSketch.h"
#include "FpEstimator.h"
#include "KWiseHash.h"

LpSampler::LpSampler(uint16_t p, double eps, double delta, uint64_t n, uint64_t seed)
    : p_(p),
      eps_(eps),
      delta_(delta),
      n_(n),
      seed_(seed),
      scalars_(static_cast<uint64_t>(2 * std::ceil(1 - std::log2(eps))), seed) {
    if (p > 2 || p == 0) {
        throw std::invalid_argument("Only implemented for p = 1 or p = 2");
    }
    if (eps <= 0 || eps >= 1) {
        throw std::invalid_argument("eps must be in (0, 1)");
    }
    if (delta <= 0 || delta >= 1) {
        throw std::invalid_argument("delta must be in (0, 1)");
    }

    if (p == 1) {
        m_ = static_cast<uint64_t>(8 * std::ceil(-std::log(eps_)));
        fp_ = std::make_unique<F1Estimator>(norm_eps_, delta / 2, seed);
    } else {
        m_ = static_cast<uint64_t>(8 * 1 / eps * std::log(n_));
        fp_ = std::make_unique<F2Estimator>(norm_eps_, delta / 2, seed);
    }

    size_t depth = 4 * static_cast<size_t>(std::ceil(std::log(n_)));
    depth = (depth & 1) ? depth : depth + 1;  // make sure depth is odd
    cs_ = std::make_unique<CountSketch>(6 * m_, depth, seed, false);

    f2_err_ = std::make_unique<F2Estimator>(norm_eps_, delta_ / 2, seed_);
}

void LpSampler::update(const uint64_t i, const double delta) {
    double u_i = scalars_.hash(i) / static_cast<double>(scalars_.get_mp());  // Uni(0, 1)
    double z_i = delta / std::pow(u_i, 1 / p_);

    cs_->update(i, z_i);
    fp_->update(i, delta);
    f2_err_->update(i, z_i);
}

std::optional<uint64_t> LpSampler::sample() const {
    if (sampled_) {
        throw std::runtime_error("Already sampled");
    }
    sampled_ = true;

    double r = 1.5 * fp_->estimate_norm();

    auto cmp = [](const std::pair<uint64_t, double>& a,
                  const std::pair<uint64_t, double>& b) {
        return std::fabs(a.second) >= std::fabs(b.second);
    };
    std::priority_queue<std::pair<uint64_t, double>,
                        std::vector<std::pair<uint64_t, double>>,
                        decltype(cmp)>
        pq;

    std::pair<uint64_t, double> max_pair = {0, 0};
    for (size_t i = 0; i < n_; ++i) {
        double z_star_i = cs_->estimate(i);

        if (std::fabs(z_star_i) > std::fabs(max_pair.second)) {
            max_pair = {i, z_star_i};
        }

        if (pq.size() < m_) {
            pq.push({i, z_star_i});
        } else if (std::fabs(pq.top().second) < std::fabs(z_star_i)) {
            pq.pop();
            pq.push({i, z_star_i});
        }
    }

    F2Estimator m_sparse(norm_eps_, delta_ / 2, seed_);
    while (!pq.empty()) {
        auto pair = pq.top();
        pq.pop();
        m_sparse.update(pair.first, pair.second);
    }

    f2_err_->subtract(m_sparse);
    double s = 1.5 * f2_err_->estimate_norm();

    if (s > std::pow(eps_, 1 - 1 / p_) * r * std::sqrt(m_) ||
        std::fabs(max_pair.second) < r / std::pow(eps_, 1 / p_)) {
        return std::nullopt;
    }
    return max_pair.first;
}