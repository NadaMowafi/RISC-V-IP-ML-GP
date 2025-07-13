#ifndef ROTATION_HPP_V
#define ROTATION_HPP_V

#include <stdexcept>
#include "VectorTraits.hpp"
#include "Image.hpp"
using namespace std;

enum class RotationDirection_V
{
    CW_90,
    CCW_90,
    ROTATE_180
};

class RotationError_V : public runtime_error
{
public:
    explicit RotationError_V(const string &message) : runtime_error(message) {}
};

template <typename T = uint8_t>
class ImageRotator_V
{
public:
    void rotate(Image<T> &image, RotationDirection_V direction);

private:
    void __riscv_rotate90CW(vector<vector<T>> &matrix);
    void __riscv_rotate90CCW(vector<vector<T>> &matrix);
    void __riscv_rotate180(vector<vector<T>> &matrix);
};

#endif // ROTATION_HPP
