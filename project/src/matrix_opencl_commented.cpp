#include "matrix_opencl.hpp"
#include <iostream>

// Initialize the static shared pointer to hold compiled kernels. 
// This avoids recompiling the OpenCL C code every time we create a new MatrixCL object.
std::shared_ptr<KernelCache> MatrixCL::kernels_ = nullptr;

// =====================================================================================
// OPENCL KERNEL C-STRINGS
// These are programs compiled at runtime by the GPU driver. They execute natively on the GPU.
// =====================================================================================

const std::string kernel_source_fill = R"(
    // __kernel: Indicates this function is an entry point from the CPU.
    // __global: Asserts that 'matrix' lives in the heavy, main device memory (VRAM).
    __kernel void fill(__global float* matrix, float value, int rows, int cols) {
        
        // get_global_id(dim) fetches the unique absolute ID of this executing thread.
        // dim=0 maps to rows, dim=1 maps to columns.
        int i = get_global_id(0);
        int j = get_global_id(1);
        
        // Safety bounds check. Sometimes thread grids are launched in multiples of e.g. 16 or 64.
        // If our matrix is 10x10, threads 10 through 15 will safely do nothing.
        if (i < rows && j < cols) {
            // Flatten 2D coordinate into 1D memory array
            matrix[i * cols + j] = value;
        }
    }
)";

const std::string kernel_source_add = R"(
    __kernel void add(__global const float* A,    // Read-only input A
                      __global const float* B,    // Read-only input B
                      __global float* C,          // Write-only output C
                      int rows, int cols) {
        
        int i = get_global_id(0);
        int j = get_global_id(1);
        
        if (i < rows && j < cols) {
            int idx = i * cols + j;
            // Point-wise addition perfectly parallelized on thousands of cores
            C[idx] = A[idx] + B[idx];
        }
    }
)";

const std::string kernel_source_sub_mul = R"(
    // Computes A = A - (scalar * B) in-place.
    __kernel void sub_mul(__global float* A,
                          __global const float* B,
                          float scalar,
                          int rows, int cols) {
        
        int i = get_global_id(0);
        int j = get_global_id(1);
        
        if (i < rows && j < cols) {
            int idx = i * cols + j;
            // Notice: Subtracting from the same memory it reads from (in-place)
            A[idx] -= scalar * B[idx];
        }
    }
)";

const std::string kernel_source_transpose = R"(
    __kernel void transpose(__global const float* A,
                            __global float* B,
                            int A_rows, int A_cols) {
        
        int r = get_global_id(0); // r maps to A's row
        int c = get_global_id(1); // c maps to A's col
        
        if (r < A_rows && c < A_cols) {
            // Read A's element at (r, c) and place it in B at (c, r)
            // A flattening: r * A_cols + c
            // B flattening: c * A_rows + r (because B has swapped dimensions)
            B[c * A_rows + r] = A[r * A_cols + c];
        }
    }
)";

const std::string kernel_source_matrix_mul = R"(
    // Naïve Matrix Multiplication: O(N^3) complexity
    // A faster variant would require loading blocks heavily into __local memory (Tiling).
    __kernel void matrix_mul(__global const float* A,
                             __global const float* B,
                             __global float* C,
                             int A_rows, int A_cols, int B_cols) {
                             
        // Determine which element of C this particular thread calculates
        int row = get_global_id(0);
        int col = get_global_id(1);
        
        if (row < A_rows && col < B_cols) {
            float sum = 0.0f;
            
            // The thread travels across the entire given 'row' of A
            // and downwards the given 'col' of B simultaneously.
            // Notice that thousands of threads are fetching from global VRAM blindly here.
            for (int k = 0; k < A_cols; k++) {
                sum += A[row * A_cols + k] * B[k * B_cols + col];
            }
            
            // Assign the dot-product result into output
            C[row * B_cols + col] = sum;
        }
    }
)";

// =====================================================================================
// KERNEL CACHE COMPILATION
// =====================================================================================

// Invoked heavily once during program startup.
void KernelCache::compileKernels(cl::Context context, const std::vector<cl::Device>& devices) {
    if (initialized) return;

    std::cout << "Compiling OpenCL kernels..." << std::endl;
    try {
        // cl::Program parses the raw string text and compiles it against the specific
        // driver installed for the GPU.
        cl::Program prog_fill = loadAndBuildProgram(context, devices, kernel_source_fill, "fill");
        kernel_fill = cl::Kernel(prog_fill, "fill");

        cl::Program prog_add = loadAndBuildProgram(context, devices, kernel_source_add, "add");
        kernel_add = cl::Kernel(prog_add, "add");

        cl::Program prog_sub_mul = loadAndBuildProgram(context, devices, kernel_source_sub_mul, "sub_mul");
        kernel_sub_mul = cl::Kernel(prog_sub_mul, "sub_mul");

        cl::Program prog_transpose = loadAndBuildProgram(context, devices, kernel_source_transpose, "transpose");
        kernel_transpose = cl::Kernel(prog_transpose, "transpose");

        cl::Program prog_matrix_mul = loadAndBuildProgram(context, devices, kernel_source_matrix_mul, "matrix_mul");
        kernel_matrix_mul = cl::Kernel(prog_matrix_mul, "matrix_mul");

        initialized = true;
        std::cout << "OpenCL kernels compiled successfully." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Failed to compile one or more OpenCL kernels. Aborting." << std::endl;
        throw;
    }
}

void MatrixCL::initializeKernels(cl::Context context, const std::vector<cl::Device>& devices) {
    try {
        if (!kernels_ || !kernels_->initialized) {
            std::cout << "Creating and compiling kernels..." << std::endl;
            kernels_ = std::make_shared<KernelCache>();
            kernels_->compileKernels(context, devices);
        }
    } catch (const cl::Error& err) {
        std::cerr << "OpenCL error in kernel initialization: "
                  << err.what() << " (" << err.err() << ")" << std::endl;
        throw;
    } catch (const std::exception& e) {
        std::cerr << "Exception in kernel initialization: " << e.what() << std::endl;
        throw;
    }
}


// =====================================================================================
// HOST C++ API (CPU SIDE)
// =====================================================================================

// Helper calculating byte allocation lengths.
size_t MatrixCL::buffer_size_bytes() const {
    return static_cast<size_t>(rows_) * cols_ * sizeof(float);
}

// Memory Allocation Constructor
MatrixCL::MatrixCL(int rows, int cols, cl::Context context, cl::CommandQueue queue, const std::vector<float>* initial_data)
    : rows_(rows), cols_(cols), context_(context), queue_(queue)
{
    if (rows_ * cols_ > 0) {
        // Reserve an empty block of raw VRAM on the GPU directly. (Equivalent of cudaMalloc).
        buffer_ = cl::Buffer(context_, CL_MEM_READ_WRITE, buffer_size_bytes());
        
        // If the CPU passed us real numbers, ship them down the PCIe pipeline immediately!
        if (initial_data && initial_data->size() == (size_t)(rows_ * cols_)) {
            // CL_TRUE specifies a "blocking write". The CPU will pause here until the GPU gets it.
            queue_.enqueueWriteBuffer(buffer_, CL_TRUE, 0, buffer_size_bytes(), initial_data->data());
        }
    }
}

// GPU-to-GPU Copy Constructor
MatrixCL::MatrixCL(const MatrixCL& other)
    : rows_(other.rows_), cols_(other.cols_),
      context_(other.context_), queue_(other.queue_)
{
    if (rows_ * cols_ > 0) {
        buffer_ = cl::Buffer(context_, CL_MEM_READ_WRITE, buffer_size_bytes());
        // Enqueue an instruction for the GPU to duplicate data internally (Super fast!).
        queue_.enqueueCopyBuffer(other.buffer_, buffer_, 0, 0, buffer_size_bytes());
        // Block until it finishes.
        queue_.finish();
    }
}

MatrixCL& MatrixCL::operator=(const MatrixCL& other)
{
    if (this == &other) return *this;

    rows_ = other.rows_;
    cols_ = other.cols_;
    context_ = other.context_;
    queue_ = other.queue_;

    if (rows_ * cols_ > 0) {
        buffer_ = cl::Buffer(context_, CL_MEM_READ_WRITE, buffer_size_bytes());
        queue_.enqueueCopyBuffer(other.buffer_, buffer_, 0, 0, buffer_size_bytes());
        queue_.finish();
    } else {
        buffer_ = cl::Buffer(); // Empty handle
    }

    return *this;
}

int MatrixCL::numRows() const { return rows_; }
int MatrixCL::numCols() const { return cols_; }
cl::Context MatrixCL::getContext() const { return context_; }
cl::CommandQueue MatrixCL::getQueue() const { return queue_; }
const cl::Buffer& MatrixCL::getBuffer() const { return buffer_; }

// Hardware memory retrieval
std::vector<float> MatrixCL::copyToHost() const
{
    // Pre-allocate system RAM.
    std::vector<float> host_data(static_cast<size_t>(rows_) * cols_);
    size_t size = buffer_size_bytes();
    if (size == 0) return host_data;

    // Execute a read stream transferring via PCIe bus back to host_data.data()
    // CL_TRUE makes sure the CPU pointer holds real data before continuing!
    queue_.enqueueReadBuffer(buffer_, CL_TRUE, 0, size, host_data.data());

    return host_data;
}

// -------------------------------------------------------------
// Asynchronous Execution Launchers
// -------------------------------------------------------------

void MatrixCL::fill(float value)
{
    if (rows_ * cols_ == 0) return;

    // We explicitly bind every parameter our C Kernel function expects.
    // 0->matrix, 1->value, 2->rows, 3->cols
    kernels_->kernel_fill.setArg(0, buffer_);
    kernels_->kernel_fill.setArg(1, value);
    kernels_->kernel_fill.setArg(2, rows_);
    kernels_->kernel_fill.setArg(3, cols_);

    // Determine the Global Workspace (Thread Grid sizes) -> X threads per Y threads.
    cl::NDRange global(rows_, cols_);
    
    // Command the scheduler to throw this work on the GPU
    queue_.enqueueNDRangeKernel(kernels_->kernel_fill, cl::NullRange, global, cl::NullRange);
    
    // Suspend C++ execution until the queue empties
    queue_.finish();
}

MatrixCL MatrixCL::operator+(const MatrixCL& other) const
{
    // Boot up another uninitialized array sitting blank in VRAM
    MatrixCL result(rows_, cols_, context_, queue_);
    if (rows_ * cols_ == 0) return result;

    kernels_->kernel_add.setArg(0, buffer_);         // Our 'this' buffer natively
    kernels_->kernel_add.setArg(1, other.buffer_);   // Their RHS buffer natively
    kernels_->kernel_add.setArg(2, result.buffer_);  // Tell kernel where to inject outputs!
    kernels_->kernel_add.setArg(3, rows_);
    kernels_->kernel_add.setArg(4, cols_);

    cl::NDRange global(rows_, cols_);
    queue_.enqueueNDRangeKernel(kernels_->kernel_add, cl::NullRange, global, cl::NullRange);
    queue_.finish();

    return result; // Wrapper returning ownership of output buffers
}

MatrixCL MatrixCL::operator-(const MatrixCL& other) const
{
    MatrixCL result(*this); // Deep copy "this" memory as a starting point.
    if (rows_ * cols_ == 0) return result;

    // We apply our `sub_mul` kernel as a clever shortcut.
    // Result = Result - 1.0f * Other
    kernels_->kernel_sub_mul.setArg(0, result.buffer_);
    kernels_->kernel_sub_mul.setArg(1, other.buffer_);
    float scalar = 1.0f;
    kernels_->kernel_sub_mul.setArg(2, scalar);
    kernels_->kernel_sub_mul.setArg(3, rows_);
    kernels_->kernel_sub_mul.setArg(4, cols_);

    cl::NDRange global(rows_, cols_);
    queue_.enqueueNDRangeKernel(kernels_->kernel_sub_mul, cl::NullRange, global, cl::NullRange);
    queue_.finish();

    return result;
}

MatrixCL MatrixCL::operator*(float scalar) const
{
    MatrixCL result(rows_, cols_, context_, queue_);
    if (rows_ * cols_ == 0) return result;
    
    // Another workaround reusing `sub_mul`:
    // 0 = 0 - (-scalar) * this -> Result = scalar * this
    result.fill(0.0f);
    kernels_->kernel_sub_mul.setArg(0, result.buffer_);
    kernels_->kernel_sub_mul.setArg(1, buffer_);
    kernels_->kernel_sub_mul.setArg(2, -scalar);
    kernels_->kernel_sub_mul.setArg(3, rows_);
    kernels_->kernel_sub_mul.setArg(4, cols_);

    cl::NDRange global(rows_, cols_);
    queue_.enqueueNDRangeKernel(kernels_->kernel_sub_mul, cl::NullRange, global, cl::NullRange);
    queue_.finish();

    return result;
}

MatrixCL MatrixCL::operator*(const MatrixCL& other) const
{
    int C_rows = this->rows_;
    int C_cols = other.cols_;
    // Prepare the differently sized product matrix internally
    MatrixCL result(C_rows, C_cols, context_, queue_);
    if (C_rows * C_cols == 0) return result;

    // Load args: A, B, C, A_rows, A_cols, B_cols
    kernels_->kernel_matrix_mul.setArg(0, buffer_);
    kernels_->kernel_matrix_mul.setArg(1, other.buffer_);
    kernels_->kernel_matrix_mul.setArg(2, result.buffer_);
    kernels_->kernel_matrix_mul.setArg(3, C_rows);
    kernels_->kernel_matrix_mul.setArg(4, this->cols_); 
    kernels_->kernel_matrix_mul.setArg(5, C_cols);

    // Threads are shaped identically to the resulting output dimensions!
    // Eg. 1 Million threads launched purely for output calculation.
    cl::NDRange global(C_rows, C_cols);
    queue_.enqueueNDRangeKernel(kernels_->kernel_matrix_mul, cl::NullRange, global, cl::NullRange);
    queue_.finish();

    return result;
}

MatrixCL MatrixCL::transpose() const
{
    MatrixCL result(cols_, rows_, context_, queue_);
    if (rows_ * cols_ == 0) return result;

    kernels_->kernel_transpose.setArg(0, buffer_);
    kernels_->kernel_transpose.setArg(1, result.buffer_);
    kernels_->kernel_transpose.setArg(2, rows_);
    kernels_->kernel_transpose.setArg(3, cols_);

    cl::NDRange global(rows_, cols_);
    queue_.enqueueNDRangeKernel(kernels_->kernel_transpose, cl::NullRange, global, cl::NullRange);
    queue_.finish();

    return result;
}

void MatrixCL::sub_mul(float scalar, const MatrixCL& other)
{
    // Destructive / In-place mutation wrapper heavily useful for optimization.
    if (rows_ * cols_ == 0) return;

    kernels_->kernel_sub_mul.setArg(0, buffer_);
    kernels_->kernel_sub_mul.setArg(1, other.buffer_);
    kernels_->kernel_sub_mul.setArg(2, scalar);
    kernels_->kernel_sub_mul.setArg(3, rows_);
    kernels_->kernel_sub_mul.setArg(4, cols_);

    cl::NDRange global(rows_, cols_);
    queue_.enqueueNDRangeKernel(kernels_->kernel_sub_mul, cl::NullRange, global, cl::NullRange);
    queue_.finish();
}
