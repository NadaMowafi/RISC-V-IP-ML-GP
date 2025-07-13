#ifndef ADD_HPP
#define ADD_HPP
#include <vector>
#include <cstdint>
using namespace std;   

enum class CLIPPING
{
    SATURATION,
    WRAP
};
template <typename T = uint8_t>
vector<vector<T>> add(const vector<vector<T>>& image1, const vector<vector<T>>& image2 , CLIPPING clip);
template <typename T = uint8_t>
vector<vector<T>> addWeighted( const vector<vector<T>>& image1,double alpha, const vector<vector<T>>& image2, double beta, double gamma, CLIPPING clip);

#endif // ADD_HPP