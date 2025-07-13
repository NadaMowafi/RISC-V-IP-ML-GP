#ifndef FLIPPINGV_HPP
#define FLIPPINGV_HPP
#include <stdexcept>
#include <riscv_vector.h>
#include "Image.hpp"
#include "VectorTraits.hpp"
#include <vector>
using namespace std;

enum class FlippingDirection_V
{
    VERTICAL,
    HORIZONTAL
};

class FlipError_V : public runtime_error
{
public:
    explicit FlipError_V(const string &message) : runtime_error(message) {}
};

template <typename T = uint8_t>
class ImageFlipper_V
{

public:
    void flip(Image<T> &image, const FlippingDirection_V direction);

private:
    void flipVertical(vector<vector<T>> &matrix);
    void flipHorizontal(vector<vector<T>> &matrix);
};

#endif // FLIPPINGV_HPP
