#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <functional>

class Matrix
{
private:
    int rows, cols;
    std::vector<double> data;

public:
    // --- Constructors & Assignment ---
    Matrix(int rows, int cols);
    Matrix(const Matrix &other);
    Matrix &operator=(const Matrix &other)
    {
        if (this != &other)
        {
            rows = other.rows;
            cols = other.cols;
            data = other.data;
        }
        return *this;
    }

    // --- Common API (shared with DistributedMatrix and MatrixCL) ---

    inline int numRows() const { return rows; }
    inline int numCols() const { return cols; }

    void fill(double value);

    Matrix operator+(const Matrix &other) const;
    Matrix operator-(const Matrix &other) const;
    Matrix operator*(const Matrix &other) const; // Matrix multiplication
    Matrix operator*(double scalar) const;       // Scalar multiplication

    Matrix transpose() const;

    // this = this - scalar * other
    void sub_mul(double scalar, const Matrix &other);

    // --- Matrix-specific operations ---

    inline double get(int i, int j) const {
        return data[i * cols + j];
    }
    inline void set(int i, int j, double value) {
        data[i * cols + j] = value;
    }

    // Get raw pointer to data (for performance-critical code)
    inline double* getData() { return data.data(); }
    inline const double* getData() const { return data.data(); }

    // Apply a function element-wise
    Matrix apply(const std::function<double(double)> &func) const;
};

#endif // MATRIX_H
