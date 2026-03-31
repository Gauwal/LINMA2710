#include "matrix.hpp"
#include <stdexcept>
#ifdef _OPENMP
#include <omp.h>
#endif

Matrix::Matrix(int rows, int cols)
    : rows(rows), cols(cols)
{
    data.resize(rows * cols, 0.0);
}

Matrix::Matrix(const Matrix &other)
    : rows(other.rows), cols(other.cols), data(other.data)
{
}

void Matrix::fill(double value)
{
    #pragma omp parallel for
    for (size_t i = 0; i < data.size(); ++i)
    {
        data[i] = value;
    }
}

Matrix Matrix::operator+(const Matrix &other) const
{
    Matrix result(rows, cols);
    #pragma omp parallel for
    for (size_t i = 0; i < data.size(); ++i)
    {
        result.data[i] = data[i] + other.data[i];
    }
    return result;
}

Matrix Matrix::operator-(const Matrix &other) const
{
    Matrix result(rows, cols);
    #pragma omp parallel for
    for (size_t i = 0; i < data.size(); ++i)
    {
        result.data[i] = data[i] - other.data[i];
    }
    return result;
}

Matrix Matrix::operator*(const Matrix &other) const
{
    Matrix result(rows, other.cols); 
    Matrix otherT = other.transpose(); // Transpose for cache and SIMD performance

    // Only parallelize for relatively large matrices to avoid thread overhead
    #pragma omp parallel for if (rows * other.cols > 10000)
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < other.cols; ++j)
        {
            double sum = 0.0;
            // Now memory is read sequentially resulting in SIMD auto-vectorization
            for (int k = 0; k < cols; ++k)
            {
                sum += data[i * cols + k] * otherT.data[j * otherT.cols + k];
            }
            result.data[i * result.cols + j] = sum;
        }
    }
    return result;
}

Matrix Matrix::operator*(double scalar) const
{
    Matrix result(rows, cols);
    #pragma omp parallel for
    for (size_t i = 0; i < data.size(); ++i)
    {
        result.data[i] = data[i] * scalar;
    }
    return result;
}

Matrix Matrix::transpose() const
{
    Matrix result(cols, rows);
    #pragma omp parallel for
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            result.data[j * result.cols + i] = data[i * cols + j];
        }
    }
    return result;
}

Matrix Matrix::apply(const std::function<double(double)> &func) const
{
    Matrix result(rows, cols);
    #pragma omp parallel for
    for (size_t i = 0; i < data.size(); ++i)
    {
        result.data[i] = func(data[i]);
    }
    return result;
}

void Matrix::sub_mul(double scalar, const Matrix &other)
{
    #pragma omp parallel for
    for (size_t i = 0; i < data.size(); ++i)
    {
        data[i] -= scalar * other.data[i];
    }
}
