#include "image_reader.hpp"
#include "image_writer.hpp"
#include "BilateralFilter.hpp"
#include <vector>
#include <cstdint>
#include <iostream>
/*
****************************************************************************************

- plz note that  A common practice is to set the kernel radius 
  to be AROUND 2-3 TIMEs the value of the spatial sigma so fine tune these parameters
  to get the best balanced results as increasing the kernel size would result in higher 
  computational power.

*Keep in mind*
- For higher resolution images or when noise patterns are more spread out, 
  you might increase the kernel size to capture a broader context.
- For images with subtle noise and fine details, a smaller kernel might be more appropriate.
*Keep in mind*
Spatial Sigma (𝜎𝑠): 2–5 for moderate smoothing.
while Intensity Sigma (𝜎𝑟): 10–30 for moderate smoothing.
****************************************************************************************
*/

int main() {
    ImageReader reader;
    ImageWriter writer;
    Image image;
    BilateralFilter BilateralFilter;
    int kernelSize = 7;
    double sigmaSpatial = 3.0;
    double sigmaIntensity = 15.0;
    // -------------------Apply Bilateral Filter-----------------------//
    ImageStatus status = reader.readImage("barb.512.pgm", image);
    if (status != ImageStatus::SUCCESS) {
        std::cerr << "Failed to read image: " << static_cast<int>(status) << std::endl;
        return 1;
    }
    std::cout << "Image read successfully. Size: "
              << image.metadata.width << "x"
              << image.metadata.height << std::endl;
    //Sometimes applying the kernel only once is not enough to get the best results
    //so we apply the kernel twice to get the best results
    for(int count=0; count == 2 ; count++){
      std::vector<std::vector<uint8_t>> filteredMatrix = BilateralFilter::apply(
        image.pixelMatrix,
        kernelSize,
        sigmaSpatial,
        sigmaIntensity);
    image.pixelMatrix = filteredMatrix;
    count++;
    }
    
    
    status = writer.writeImage("barb.512blur_bilateral.pgm", image);
    if (status != ImageStatus::SUCCESS) {
        std::cerr << "Failed to write Bilateral filtered image: " << static_cast<int>(status) << std::endl;
        return 1;
    }
    std::cout << "Bilateral Filtered image written successfully." << std::endl;

    return 0;
}