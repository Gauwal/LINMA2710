#pragma once

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 300
#include <CL/opencl.hpp>
#include <vector>
#include <memory>
#include <string>

struct KernelCache {
    cl::Kernel kernel_fill;
    cl::Kernel kernel_add;
    cl::Kernel kernel_sub;
    cl::Kernel kernel_scalar_mul;
    cl::Kernel kernel_sub_mul;
    cl::Kernel kernel_transpose;
    cl::Kernel kernel_matrix_mul;
    bool initialized = false;

    void compileKernels(cl::Context context, const std::vector<cl::Device>& devices);
};

class MatrixCL {
public:
    static void initializeKernels(cl::Context context, const std::vector<cl::Device>& devices);

    MatrixCL(int rows, int cols, cl::Context context, cl::CommandQueue queue, const std::vector<float>* initial_data = nullptr);
    MatrixCL(const MatrixCL& other);
    MatrixCL& operator=(const MatrixCL& other);
    ~MatrixCL() = default;

    int numRows() const;
    int numCols() const;
    cl::Context getContext() const;
    cl::CommandQueue getQueue() const;
    const cl::Buffer& getBuffer() const;

    std::vector<float> copyToHost() const;

    MatrixCL operator+(const MatrixCL& other) const;
    MatrixCL operator-(const MatrixCL& other) const;
    MatrixCL operator*(const MatrixCL& other) const;
    MatrixCL operator*(float scalar) const;
    MatrixCL transpose() const;
    void fill(float value);
    void sub_mul(float scalar, const MatrixCL& other);

private:
    int rows_;
    int cols_;
    cl::Context context_;
    cl::CommandQueue queue_;
    cl::Buffer buffer_;

    static std::shared_ptr<KernelCache> kernels_;
    size_t buffer_size_bytes() const;
};
// Ensure cl::BuildError compiles on OpenCL 3.0
#ifndef CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_ENABLE_EXCEPTIONS
#endif
