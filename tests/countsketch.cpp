#include "CountSketch.h"

#include <iostream>

int main() {
    CountSketch cs(20, 5, 42);  // 5 rows, 20 columns

    // Insert/update keys
    cs.update(42, 10);
    cs.update(42, 5);
    cs.update(7, 3);
    cs.update(123, -2);

    // Query frequencies
    std::cout << "Estimate for key 42: " << cs.estimate(42) << std::endl;
    std::cout << "Estimate for key 7: " << cs.estimate(7) << std::endl;
    std::cout << "Estimate for key 123: " << cs.estimate(123) << std::endl;
    std::cout << "Estimate for key 99 (never inserted): " << cs.estimate(99) << std::endl;

    std::cout << cs << std::endl;

    return 0;
}
