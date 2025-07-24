#include "ImageReader.hpp"
#include "ImageWriter.hpp"
#include "Rotate.hpp"
#include "Flipping.hpp"
#include "BoxFilter.hpp"
#include "Gaussian.hpp"
#include "Gaussian_Vector.hpp"
#include "FFT.hpp"
#include <iostream>
#include <vector>
#include <cstdint>

using namespace std;

int main() {
    ImageReader reader;
    ImageWriter writer;
    ImageRotator rotator;
    ImageFlipper flipper;
    BoxFilter boxFilter;
    Image image;
    Image filteredImage;
    int kernelSize = 5;
    double sigma = 1.0;

//     // Read the image
//     ImageStatus status = reader.readImage("barb.512.pgm", image);
//     if (status != ImageStatus::SUCCESS) {
//         cerr << "Failed to read image: " << static_cast<int>(status) << endl;
//         return 1;
//     }
//     cout << "Image read successfully. Size: " << image.metadata.width << "x" << image.metadata.height << endl;

//     // ------------------- Apply BoxFilter FFT ----------------------- 
//     vector<vector<uint8_t>> filteredMatrixFFT = boxFilter.applyBoxFilterFFT(image.pixelMatrix, kernelSize);
//     image.pixelMatrix = filteredMatrixFFT;
//     status = writer.writeImage("barb.512blur_fft.pgm", image);
//     if (status != ImageStatus::SUCCESS) {
//         cerr << "Failed to write FFT filtered image: " << static_cast<int>(status) << endl;
//         return 1;
//     }
//     cout << "FFT Filtered image written successfully." << endl;

//     // ------------------- Apply BoxFilter Sliding -----------------------
//     status = reader.readImage("barb.512.pgm", image);
//     if (status != ImageStatus::SUCCESS) {
//         cerr << "Failed to read image: " << static_cast<int>(status) << endl;
//         return 1;
//     }
//     vector<vector<uint8_t>> filteredMatrixSliding = boxFilter.applyBoxFilterSlidingGrey(image.pixelMatrix, kernelSize);
//     image.pixelMatrix = filteredMatrixSliding;
//     status = writer.writeImage("barb.512blur_sliding.pgm", image);
//     if (status != ImageStatus::SUCCESS) {
//         cerr << "Failed to write Sliding Window filtered image: " << static_cast<int>(status) << endl;
//         return 1;
//     }
//     cout << "Sliding Window filtered image written successfully." << endl;

//     // ------------------- Apply Gaussian Filter -----------------------
//     status = reader.readImage("barb.512.pgm", image);
//     if (status != ImageStatus::SUCCESS) {
//         cerr << "Failed to read image: " << static_cast<int>(status) << endl;
//         return 1;
//     }
//     vector<vector<double>> gaussianKernel = generateGaussianKernel(kernelSize, sigma);
//     vector<vector<uint8_t>> filteredMatrixGaussian = applyGaussianFilter(image.pixelMatrix, gaussianKernel);
//     image.pixelMatrix = filteredMatrixGaussian;
//     status = writer.writeImage("barb.512blur_gaussian.pgm", image);
//     if (status != ImageStatus::SUCCESS) {
//         cerr << "Failed to write Gaussian filtered image: " << static_cast<int>(status) << endl;
//         return 1;
//     }
//     cout << "Gaussian filtered image written successfully." << endl;

//     // ------------------- Rotation & Flipping -----------------------
//     status = reader.readImage("barb.512.pgm", image);
//     if (status != ImageStatus::SUCCESS) {
//         cerr << "Failed to read image: " << static_cast<int>(status) << endl;
//         return 1;
//     }
//     cout << "Processing image rotation and flipping..." << endl;

//     // Rotate 90 degrees CCW
//     rotator.rotate(image, RotationDirection::CCW_90);
//     status = writer.writeImage("output_90CCW.pgm", image);
//     if (status != ImageStatus::SUCCESS) {
//         cerr << "Failed to write 90CCW image: " << static_cast<int>(status) << endl;
//         return 1;
//     }
//     cout << "90-degree CCW rotated image written successfully." << endl;

//     // Flip Horizontally
//     status = reader.readImage("barb.512.pgm", image);
//     if (status != ImageStatus::SUCCESS) {
//         cerr << "Failed to read image: " << static_cast<int>(status) << endl;
//         return 1;
//     }
//     flipper.flip(image, FlippingDirection::HORIZONTAL);
//     status = writer.writeImage("Flipped_Horizontal.pgm", image);
//     if (status != ImageStatus::SUCCESS) {
//         cerr << "Failed to write flipped image: " << static_cast<int>(status) << endl;
//         return 1;
//     }
//     cout << "Horizontal flipped image written successfully." << endl;

//     return 0;
// }
// ------------------- Vectorized Gaussian -----------------------
ImageStatus status = reader.readImage("barb.512.pgm", image);
if (status != ImageStatus::SUCCESS) {
    cerr << "Failed to read image: " << static_cast<int>(status) << endl;
    return 1;
}

cout << "Image read successfully. Size: "
     << image.metadata.width << "x"
     << image.metadata.height << endl;


auto output = __riscv_applyGaussianFilterSeparable<uint8_t>(image.pixelMatrix, kernelSize, sigma);

filteredImage.pixelMatrix = output;
filteredImage.pixelData = image.pixelData;
filteredImage.metadata = image.metadata;

status = writer.writeImage("Gaussian_Vector.pgm", filteredImage);
if (status != ImageStatus::SUCCESS) {
    cerr << "Failed to write image: " << static_cast<int>(status) << endl;
    return 1;
}
cout << "Gaussian filtered image written successfully." << endl;

return 0;
}
