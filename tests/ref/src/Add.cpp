#include <iostream>
#include <vector>
#include <cmath>
#include "Add.hpp"



template <typename T>
vector<vector<T>> add(const vector<vector<T>>& image1, const vector<vector<T>>& image2, CLIPPING clip)
{
    if (image1.empty() || image1[0].empty() || image2.empty() || image2[0].empty()) {
        throw invalid_argument("One or both image are empty");
    }
    if (image1.size() != image1[0].size() || image2.size() != image2[0].size()) {
        throw invalid_argument("Images are not square");
    }
    int rows1 = image1.size();
    int cols1 = image1[0].size();    
    int rows2 = image2.size();
    int cols2 = image2[0].size();  

    int outputRows = max(rows1, rows2);
    int outputCols = max(cols1, cols2);
    vector<vector<T>> outputImg(outputRows, vector<T>(outputCols, 0));
    for (int i = 0; i < rows1; i++) {
        for (int j = 0; j < cols1; j++) 
        {
           outputImg[(outputRows-rows1)/2+i][(outputCols-cols1)/2+j] += image1[i][j];   
        }
    }
    
for (int i = 0; i < rows2; i++) {
        for (int j = 0; j < cols2; j++) {
            if (clip == CLIPPING::SATURATION)
            {
                outputImg[(outputRows-rows2)/2+i][(outputRows-rows2)/2+j] = static_cast<T>(min(static_cast<int>(numeric_limits<T>::max()),
                static_cast<int>(outputImg[(outputRows-rows2)/2+i][(outputRows-rows2)/2+j]+image2[i][j])));
            }
            
            else if (clip == CLIPPING::WRAP)
            {
            outputImg[(outputRows-rows2)/2+i][(outputRows-rows2)/2+j] = outputImg[(outputRows-rows2)/2+i][(outputRows-rows2)/2+j]+image2[i][j];   
            }
         }  
        }
    

    return outputImg;
}



template <typename T>
vector<vector<T>> addWeighted(const vector<vector<T>>& image1, double alpha, const vector<vector<T>>& image2, double beta, double gamma, CLIPPING clip)
{
    if (image1.empty() || image1[0].empty() || image2.empty() || image2[0].empty()) {
        throw invalid_argument("One or both image are empty");
    }
    if (image1.size() != image1[0].size() || image2.size() != image2[0].size()) {
        throw invalid_argument("Images are not square");
    }

    int rows1 = image1.size();
    int cols1 = image1[0].size();    
    int rows2 = image2.size();
    int cols2 = image2[0].size();

    int outputRows = max(rows1, rows2);
    int outputCols = max(cols1, cols2);

    vector<vector<double>> tempOut(outputRows, vector<double>(outputCols, 0));
    for (int i = 0; i < rows1; i++) {
        for (int j = 0; j < cols1; j++) {
            tempOut[(outputRows-rows1)/2+i][(outputCols-cols1)/2+j] += image1[i][j]*alpha+gamma;
        }
    }

    
    for (int i = 0; i < rows2; i++) {
        for (int j = 0; j < cols2; j++) {
        
            tempOut[(outputRows-rows2)/2+i][(outputRows-rows2)/2+j] += image2[i][j] * beta ;
        }
    }
    vector<vector<T>> outputImg(outputRows, vector<T>(outputCols, 0));
    for (int i = 0; i < outputRows; i++) {
        for (int j = 0; j < outputCols; j++) {
        if(clip==CLIPPING::SATURATION)
        {
            outputImg[i][j] = static_cast<T>(min(static_cast<double>(numeric_limits<T>::max()), round(tempOut[i][j])));
        }    
        else if (clip == CLIPPING::WRAP)
        {
         outputImg[i][j] = static_cast<T>(round(tempOut[i][j]));      
        }
        }
    }

    return outputImg;
}
template vector<vector<uint8_t>> add<uint8_t>(const vector<vector<uint8_t>> &, const vector<vector<uint8_t>> &, CLIPPING);
template vector<vector<uint16_t>> add<uint16_t>(const vector<vector<uint16_t>> &, const vector<vector<uint16_t>> &, CLIPPING);
template vector<vector<uint32_t>> add<uint32_t>(const vector<vector<uint32_t>> &, const vector<vector<uint32_t>> &, CLIPPING);
template vector<vector<uint64_t>> add<uint64_t>(const vector<vector<uint64_t>> &, const vector<vector<uint64_t>> &, CLIPPING);
template vector<vector<uint8_t>> addWeighted(const vector<vector<uint8_t>>&, double, const vector<vector<uint8_t>>&, double, double, CLIPPING);
template vector<vector<uint16_t>> addWeighted(const vector<vector<uint16_t>>&, double, const vector<vector<uint16_t>>&, double, double, CLIPPING);
template vector<vector<uint32_t>> addWeighted(const vector<vector<uint32_t>>&, double, const vector<vector<uint32_t>>&, double, double, CLIPPING);
template vector<vector<uint64_t>> addWeighted(const vector<vector<uint64_t>>&, double, const vector<vector<uint64_t>>&, double, double, CLIPPING);