#include <iostream>

#include "FpEstimator.h"
int main() {
    F2Estimator sketch(0.1, 0.01, false, 42);

    // Insert/update keys
    sketch.update(42, 10);
    sketch.update(42, 5);
    sketch.update(7, 3);
    sketch.update(123, -2);

    // Nonzero Entries: 15, 3, -2

    // Estimate l2 norm
    std::cout << "Estimate for l2 norm: " << sketch.estimate_norm() << std::endl;
    std::cout << "Actual l2 norm: " << 15 * 15 + 3 * 3 + (-2) * (-2) << std::endl;

    return 0;
}
