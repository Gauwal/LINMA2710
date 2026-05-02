#include "matrix_opencl.hpp"
#include <iostream>

std::shared_ptr<KernelCache> MatrixCL::kernels_ = nullptr;

cl::Program loadAndBuildProgram(cl::Context context,
                                const std::vector<cl::Device>& devices,
                                const std::string& sourceCode,
                                const std::string& kernel_name_for_error)
{
    cl::Program program(context, sourceCode);
    try {
        program.build(devices);
    } catch (const cl::BuildError& err) {
        std::cerr << "OpenCL Build Error for kernel source '" << kernel_name_for_error << "':\n"
                  << err.what() << "(" << err.err() << ")" << std::endl;
        for (const auto& pair : err.getBuildLog()) {
            std::cerr << "Device " << pair.first.getInfo<CL_DEVICE_NAME>() << ":" << std::endl;
            std::cerr << pair.second << std::endl;
        }
        throw;
    } catch (const cl::Error& err) {
        std::cerr << "OpenCL Error during program build for '" << kernel_name_for_error << "': "
                  << err.what() << " (" << err.err() << ")" << std::endl;
        throw;
    }
    return program;
}

const std::string source_fill = R"(__kernel void kernel_func(__global float* matrix, float value, int rows, int cols) { int i = get_global_id(0); int j = get_global_id(1); if (i < rows && j < cols) matrix[i * cols + j] = value; })";
const std::string source_add = R"(__kernel void kernel_func(__global const float* A, __global const float* B, __global float* C, int rows, int cols) { int i = get_global_id(0); int j = get_global_id(1); if (i < rows && j < cols) { int idx = i * cols + j; C[idx] = A[idx] + B[idx]; } })";
const std::string source_sub = R"(__kernel void kernel_func(__global const float* A, __global const float* B, __global float* C, int rows, int cols) { int i = get_global_id(0); int j = get_global_id(1); if (i < rows && j < cols) { int idx = i * cols + j; C[idx] = A[idx] - B[idx]; } })";
const std::string source_scalar_mul = R"(__kernel void kernel_func(__global const float* A, float scalar, __global float* C, int rows, int cols) { int i = get_global_id(0); int j = get_global_id(1); if (i < rows && j < cols) { int idx = i * cols + j; C[idx] = A[idx] * scalar; } })";
const std::string source_sub_mul = R"(__kernel void kernel_func(__global float* A, __global const float* B, float scalar, int rows, int cols) { int i = get_global_id(0); int j = get_global_id(1); if (i < rows && j < cols) { int idx = i * cols + j; A[idx] -= scalar * B[idx]; } })";
const std::string source_transpose = R"(__kernel void kernel_func(__global const float* A, __global float* B, int rows, int cols) { int r = get_global_id(0); int c = get_global_id(1); if (r < rows && c < cols) { B[c * rows + r] = A[r * cols + c]; } })";
const std::string source_matrix_mul = R"(__kernel void kernel_func(__global const float* A, __global const float* B, __global float* C, int A_rows, int A_cols, int B_cols) { int row = get_global_id(0); int col = get_global_id(1); if (row < A_rows && col < B_cols) { float sum = 0.0f; for(int k = 0; k < A_cols; k++) { sum += A[row * A_cols + k] * B[k * B_cols + col]; } C[row * B_cols + col] = sum; } })";

void KernelCache::compileKernels(cl::Context context, const std::vector<cl::Device>& devices) {
    if (initialized) return;
    kernel_fill = cl::Kernel(loadAndBuildProgram(context, devices, source_fill, "fill"), "kernel_func");
    kernel_add = cl::Kernel(loadAndBuildProgram(context, devices, source_add, "add"), "kernel_func");
    kernel_sub = cl::Kernel(loadAndBuildProgram(context, devices, source_sub, "sub"), "kernel_func");
    kernel_scalar_mul = cl::Kernel(loadAndBuildProgram(context, devices, source_scalar_mul, "scalar_mul"), "kernel_func");
    kernel_sub_mul = cl::Kernel(loadAndBuildProgram(context, devices, source_sub_mul, "sub_mul"), "kernel_func");
    kernel_transpose = cl::Kernel(loadAndBuildProgram(context, devices, source_transpose, "transpose"), "kernel_func");
    kernel_matrix_mul = cl::Kernel(loadAndBuildProgram(context, devices, source_matrix_mul, "matrix_mul"), "kernel_func");
    initialized = true;
}

void MatrixCL::initializeKernels(cl::Context context, const std::vector<cl::Device>& devices) {
    if (!kernels_ || !kernels_->initialized) {
        kernels_ = std::make_shared<KernelCache>();
        kernels_->compileKernels(context, devices);
    }
}

size_t MatrixCL::buffer_size_bytes() const { return static_cast<size_t>(rows_) * cols_ * sizeof(float); }

MatrixCL::MatrixCL(int rows, int cols, cl::Context context, cl::CommandQueue queue, const std::vector<float>* initial_data)
    : rows_(rows), cols_(cols), context_(context), queue_(queue) {
    if (rows_ * cols_ > 0) {
        buffer_ = cl::Buffer(context_, CL_MEM_READ_WRITE, buffer_size_bytes());
        if (initial_data && initial_data->size() == (size_t)(rows_ * cols_)) {
            queue_.enqueueWriteBuffer(buffer_, CL_TRUE, 0, buffer_size_bytes(), initial_data->data());
        }
    }
}

MatrixCL::MatrixCL(const MatrixCL& other) : rows_(other.rows_), cols_(other.cols_), context_(other.context_), queue_(other.queue_) {
    if (rows_ * cols_ > 0) {
        buffer_ = cl::Buffer(context_, CL_MEM_READ_WRITE, buffer_size_bytes());
        queue_.enqueueCopyBuffer(other.buffer_, buffer_, 0, 0, buffer_size_bytes());
        queue_.finish();
    }
}

MatrixCL& MatrixCL::operator=(const MatrixCL& other) {
    if (this == &other) return *this;
    rows_ = other.rows_; cols_ = other.cols_; context_ = other.context_; queue_ = other.queue_;
    if (rows_ * cols_ > 0) {
        buffer_ = cl::Buffer(context_, CL_MEM_READ_WRITE, buffer_size_bytes());
        queue_.enqueueCopyBuffer(other.buffer_, buffer_, 0, 0, buffer_size_bytes()); 
        queue_.finish();
    } else { 
        buffer_ = cl::Buffer(); 
    }
    return *this;
}

int MatrixCL::numRows() const { return rows_; }
int MatrixCL::numCols() const { return cols_; }
cl::Context MatrixCL::getContext() const { return context_; }
cl::CommandQueue MatrixCL::getQueue() const { return queue_; }
const cl::Buffer& MatrixCL::getBuffer() const { return buffer_; }

std::vector<float> MatrixCL::copyToHost() const {
    std::vector<float> host_data(static_cast<size_t>(rows_) * cols_);
    if (buffer_size_bytes() > 0) 
        queue_.enqueueReadBuffer(buffer_, CL_TRUE, 0, buffer_size_bytes(), host_data.data());
    return host_data;
}

void MatrixCL::fill(float value) {
    if (rows_ * cols_ > 0) {
        kernels_->kernel_fill.setArg(0, buffer_); 
        kernels_->kernel_fill.setArg(1, value); 
        kernels_->kernel_fill.setArg(2, rows_); 
        kernels_->kernel_fill.setArg(3, cols_);
        queue_.enqueueNDRangeKernel(kernels_->kernel_fill, cl::NullRange, cl::NDRange(rows_, cols_), cl::NullRange); 
        queue_.finish();
    }
}

MatrixCL MatrixCL::operator+(const MatrixCL& other) const {
    MatrixCL result(rows_, cols_, context_, queue_);
    if (rows_ * cols_ > 0) {
        kernels_->kernel_add.setArg(0, buffer_); 
        kernels_->kernel_add.setArg(1, other.buffer_); 
        kernels_->kernel_add.setArg(2, result.buffer_); 
        kernels_->kernel_add.setArg(3, rows_); 
        kernels_->kernel_add.setArg(4, cols_);
        queue_.enqueueNDRangeKernel(kernels_->kernel_add, cl::NullRange, cl::NDRange(rows_, cols_), cl::NullRange); 
        queue_.finish();
    } 
    return result;
}

MatrixCL MatrixCL::operator-(const MatrixCL& other) const {
    MatrixCL result(rows_, cols_, context_, queue_);
    if (rows_ * cols_ > 0) {
        kernels_->kernel_sub.setArg(0, buffer_); 
        kernels_->kernel_sub.setArg(1, other.buffer_); 
        kernels_->kernel_sub.setArg(2, result.buffer_); 
        kernels_->kernel_sub.setArg(3, rows_); 
        kernels_->kernel_sub.setArg(4, cols_);
        queue_.enqueueNDRangeKernel(kernels_->kernel_sub, cl::NullRange, cl::NDRange(rows_, cols_), cl::NullRange); 
        queue_.finish();
    } 
    return result;
}

MatrixCL MatrixCL::operator*(float scalar) const {
    MatrixCL result(rows_, cols_, context_, queue_);
    if (rows_ * cols_ > 0) {
        kernels_->kernel_scalar_mul.setArg(0, buffer_); 
        kernels_->kernel_scalar_mul.setArg(1, scalar); 
        kernels_->kernel_scalar_mul.setArg(2, result.buffer_); 
        kernels_->kernel_scalar_mul.setArg(3, rows_); 
        kernels_->kernel_scalar_mul.setArg(4, cols_);
        queue_.enqueueNDRangeKernel(kernels_->kernel_scalar_mul, cl::NullRange, cl::NDRange(rows_, cols_), cl::NullRange); 
        queue_.finish();
    } 
    return result;
}

MatrixCL MatrixCL::operator*(const MatrixCL& other) const {
    MatrixCL result(rows_, other.cols_, context_, queue_);
    if (rows_ * other.cols_ > 0) {
        kernels_->kernel_matrix_mul.setArg(0, buffer_); 
        kernels_->kernel_matrix_mul.setArg(1, other.buffer_); 
        kernels_->kernel_matrix_mul.setArg(2, result.buffer_); 
        kernels_->kernel_matrix_mul.setArg(3, rows_); 
        kernels_->kernel_matrix_mul.setArg(4, cols_); 
        kernels_->kernel_matrix_mul.setArg(5, other.cols_);
        queue_.enqueueNDRangeKernel(kernels_->kernel_matrix_mul, cl::NullRange, cl::NDRange(rows_, other.cols_), cl::NullRange); 
        queue_.finish();
    } 
    return result;
}

MatrixCL MatrixCL::transpose() const {
    MatrixCL result(cols_, rows_, context_, queue_);
    if (rows_ * cols_ > 0) {
        kernels_->kernel_transpose.setArg(0, buffer_); 
        kernels_->kernel_transpose.setArg(1, result.buffer_); 
        kernels_->kernel_transpose.setArg(2, rows_); 
        kernels_->kernel_transpose.setArg(3, cols_);
        queue_.enqueueNDRangeKernel(kernels_->kernel_transpose, cl::NullRange, cl::NDRange(rows_, cols_), cl::NullRange); 
        queue_.finish();
    } 
    return result;
}

void MatrixCL::sub_mul(float scalar, const MatrixCL& other) {
    if (rows_ * cols_ > 0) {
        kernels_->kernel_sub_mul.setArg(0, buffer_); 
        kernels_->kernel_sub_mul.setArg(1, other.buffer_); 
        kernels_->kernel_sub_mul.setArg(2, scalar); 
        kernels_->kernel_sub_mul.setArg(3, rows_); 
        kernels_->kernel_sub_mul.setArg(4, cols_);
        queue_.enqueueNDRangeKernel(kernels_->kernel_sub_mul, cl::NullRange, cl::NDRange(rows_, cols_), cl::NullRange); 
        queue_.finish();
    }
}
