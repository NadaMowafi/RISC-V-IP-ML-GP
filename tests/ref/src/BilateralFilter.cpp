#include "BilateralFilter.hpp"


double BilateralFilter::gaussian(double x, double sigma) {
    double xSquared = x * x;
    double sigmaSquared = sigma * sigma;
    return std::exp(-(xSquared) / (2 * sigmaSquared)) / (2 * M_PI * sigmaSquared);
}

std::vector<std::vector<uint8_t>> BilateralFilter::apply(
    const std::vector<std::vector<uint8_t>>& image,
    int kernelSize,
    double sigmaSpatial,
    double sigmaIntensity
) {
    int rows = image.size();
    int cols = image[0].size();
    std::vector<std::vector<uint8_t>> output(rows, std::vector<uint8_t>(cols, 0));

    int halfKernel = kernelSize / 2;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            double sumWeights = 0.0;
            double filteredValue = 0.0;

            for (int ki = -halfKernel; ki <= halfKernel; ++ki) {
                for (int kj = -halfKernel; kj <= halfKernel; ++kj) {
                    int ni = i + ki;
                    int nj = j + kj;

                    if (ni >= 0 && ni < rows && nj >= 0 && nj < cols) {
                        double spatialWeight = gaussian(std::sqrt(ki * ki + kj * kj), sigmaSpatial);
                        double intensityWeight = gaussian(image[ni][nj] - image[i][j], sigmaIntensity);
                        double weight = spatialWeight * intensityWeight;

                        filteredValue += weight * image[ni][nj];
                        sumWeights += weight;
                    }
                }
            }

            output[i][j] = filteredValue / sumWeights;
        }
    }

    return output;
}