#ifndef BILATERALFILTER_HPP
#define BILATERALFILTER_HPP

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

class BilateralFilter {
public:
    static std::vector<std::vector<uint8_t>> apply(
        const std::vector<std::vector<uint8_t>>& image,
        int kernelSize,
        double sigmaSpatial,        // for  Euclidean distances.
        double sigmaIntensity       // for intensity differences.
    );

private:
    static double gaussian(double x, double sigma);
};

#endif