#include "matrix_opencl.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <fstream>
#include <cmath>

// Naive kernel - simple matrix multiplication
const std::string KERNEL_NAIVE = R"(
__kernel void matrix_mul_naive(
    __global const float* A,
    __global const float* B,
    __global float* C,
    int A_rows, int A_cols, int B_cols)
{
    int row = get_global_id(0);
    int col = get_global_id(1);
    
    if (row < A_rows && col < B_cols) {
        float sum = 0.0f;
        for (int k = 0; k < A_cols; k++) {
            sum += A[row * A_cols + k] * B[k * B_cols + col];
        }
        C[row * B_cols + col] = sum;
    }
}
)";

// Optimized kernel - with local memory and work group optimization
const std::string KERNEL_OPTIMIZED = R"(
#define TILE_SIZE 16

__kernel void matrix_mul_optimized(
    __global const float* A,
    __global const float* B,
    __global float* C,
    int A_rows, int A_cols, int B_cols,
    __local float* local_A,
    __local float* local_B)
{
    int global_row = get_global_id(0);
    int global_col = get_global_id(1);
    int local_row = get_local_id(0);
    int local_col = get_local_id(1);
    int group_row = get_group_id(0);
    int group_col = get_group_id(1);
    
    float acc = 0.0f;
    
    for (int tile = 0; tile < (A_cols + TILE_SIZE - 1) / TILE_SIZE; tile++) {
        // Load tile into local memory
        int a_col = tile * TILE_SIZE + local_col;
        int b_row = tile * TILE_SIZE + local_row;
        
        if (global_row < A_rows && a_col < A_cols) {
            local_A[local_row * TILE_SIZE + local_col] = A[global_row * A_cols + a_col];
        } else {
            local_A[local_row * TILE_SIZE + local_col] = 0.0f;
        }
        
        if (b_row < A_cols && global_col < B_cols) {
            local_B[local_row * TILE_SIZE + local_col] = B[b_row * B_cols + global_col];
        } else {
            local_B[local_row * TILE_SIZE + local_col] = 0.0f;
        }
        
        barrier(CLK_LOCAL_MEM_FENCE);
        
        // Compute partial result
        for (int k = 0; k < TILE_SIZE; k++) {
            acc += local_A[local_row * TILE_SIZE + k] * local_B[k * TILE_SIZE + local_col];
        }
        
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    
    if (global_row < A_rows && global_col < B_cols) {
        C[global_row * B_cols + global_col] = acc;
    }
}
)";

struct OCL_BenchResult {
    int matrix_size;
    std::string kernel_type;
    double time_ms;
    double gflops;
};

std::vector<OCL_BenchResult> results;

double benchmark_kernel(
    int size,
    const std::string& kernel_source,
    cl::Context context,
    cl::CommandQueue queue,
    const std::vector<cl::Device>& devices,
    const std::string& kernel_name)
{
    try {
        // Compile kernel
        cl::Program program(context, kernel_source);
        try {
            program.build(devices);
        } catch (const cl::BuildError& err) {
            std::cerr << "Build error for " << kernel_name << ": " << err.what() << std::endl;
            return -1;
        }
        
        std::string kernel_entry = (kernel_source.find("matrix_mul_optimized") != std::string::npos) 
            ? "matrix_mul_optimized" : "matrix_mul_naive";
        cl::Kernel kernel(program, kernel_entry.c_str());
        
        // Create test matrices
        std::vector<float> host_A(size * size, 1.0f);
        std::vector<float> host_B(size * size, 2.0f);
        
        cl::Buffer buf_A(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                        size * size * sizeof(float), host_A.data());
        cl::Buffer buf_B(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                        size * size * sizeof(float), host_B.data());
        cl::Buffer buf_C(context, CL_MEM_WRITE_ONLY, size * size * sizeof(float));
        
        // Set kernel arguments
        kernel.setArg(0, buf_A);
        kernel.setArg(1, buf_B);
        kernel.setArg(2, buf_C);
        kernel.setArg(3, size);
        kernel.setArg(4, size);
        kernel.setArg(5, size);
        
        if (kernel_source.find("matrix_mul_optimized") != std::string::npos) {
            // Local memory for optimized kernel
            const int TILE_SIZE = 16;
            kernel.setArg(6, cl::Local(TILE_SIZE * TILE_SIZE * sizeof(float)));
            kernel.setArg(7, cl::Local(TILE_SIZE * TILE_SIZE * sizeof(float)));
        }
        
        // Warmup
        queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(size, size), 
                                   cl::NDRange(16, 16));
        queue.finish();
        
        // Benchmark
        const int NUM_RUNS = 5;
        double total_time = 0;
        
        for (int run = 0; run < NUM_RUNS; run++) {
            auto start = std::chrono::high_resolution_clock::now();
            queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(size, size),
                                      cl::NDRange(16, 16));
            queue.finish();
            auto end = std::chrono::high_resolution_clock::now();
            
            total_time += std::chrono::duration<double, std::milli>(end - start).count();
        }
        
        double avg_time = total_time / NUM_RUNS;
        return avg_time;
        
    } catch (const cl::Error& err) {
        std::cerr << "OpenCL Error: " << err.what() << " (" << err.err() << ")" << std::endl;
        return -1;
    }
}

int main() {
    try {
        std::cout << "=== Part 4: OpenCL Matrix Multiplication Benchmark ===" << std::endl << std::endl;
        
        // Get OpenCL platform and device
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        
        if (platforms.empty()) {
            std::cerr << "No OpenCL platforms found!" << std::endl;
            return 1;
        }
        
        cl::Platform platform = platforms[0];
        std::cout << "Platform: " << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
        
        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
        
        if (devices.empty()) {
            std::cout << "No GPU found, trying CPU..." << std::endl;
            platform.getDevices(CL_DEVICE_TYPE_CPU, &devices);
        }
        
        if (devices.empty()) {
            std::cerr << "No OpenCL devices found!" << std::endl;
            return 1;
        }
        
        cl::Device device = devices[0];
        std::cout << "Device: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
        std::cout << "Device Type: " << (device.getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_GPU ? "GPU" : "CPU") << std::endl;
        std::cout << "Compute Units: " << device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << std::endl;
        std::cout << "Max Work Group Size: " << device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << std::endl;
        std::cout << "Global Memory: " << (device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() / (1024*1024)) << " MB" << std::endl;
        std::cout << std::endl;
        
        cl::Context context(device);
        cl::CommandQueue queue(context, device, CL_QUEUE_PROFILING_ENABLE);
        
        std::vector<int> sizes = {128, 256, 512, 1024};
        
        std::cout << "=== Kernel Performance Comparison ===" << std::endl;
        std::cout << std::left << std::setw(15) << "Size" 
                  << std::left << std::setw(20) << "Naive (ms)" 
                  << std::left << std::setw(20) << "Optimized (ms)"
                  << std::left << std::setw(15) << "Speedup"
                  << std::left << std::setw(15) << "Naive GFLOPs"
                  << std::left << std::setw(15) << "Optimized GFLOPs" << std::endl;
        std::cout << std::string(100, '-') << std::endl;
        
        for (int size : sizes) {
            double time_naive = benchmark_kernel(size, KERNEL_NAIVE, context, queue, devices, "naive");
            double time_optimized = benchmark_kernel(size, KERNEL_OPTIMIZED, context, queue, devices, "optimized");
            
            if (time_naive > 0 && time_optimized > 0) {
                double flops = 2.0 * size * size * size;
                double gflops_naive = flops / (time_naive * 1e6);
                double gflops_optimized = flops / (time_optimized * 1e6);
                double speedup = time_naive / time_optimized;
                
                OCL_BenchResult res_naive;
                res_naive.matrix_size = size;
                res_naive.kernel_type = "naive";
                res_naive.time_ms = time_naive;
                res_naive.gflops = gflops_naive;
                results.push_back(res_naive);
                
                OCL_BenchResult res_opt;
                res_opt.matrix_size = size;
                res_opt.kernel_type = "optimized";
                res_opt.time_ms = time_optimized;
                res_opt.gflops = gflops_optimized;
                results.push_back(res_opt);
                
                std::cout << std::left << std::setw(15) << size
                          << std::left << std::setw(20) << std::fixed << std::setprecision(3) << time_naive
                          << std::left << std::setw(20) << std::setprecision(3) << time_optimized
                          << std::left << std::setw(15) << std::setprecision(2) << speedup << "x"
                          << std::left << std::setw(15) << std::setprecision(2) << gflops_naive
                          << std::left << std::setw(15) << std::setprecision(2) << gflops_optimized << std::endl;
            }
        }
        
        // Save results to CSV
        std::ofstream csv("bench_opencl_results.csv");
        csv << "matrix_size,kernel_type,time_ms,gflops\n";
        for (const auto& r : results) {
            csv << r.matrix_size << "," << r.kernel_type << "," 
                << r.time_ms << "," << r.gflops << "\n";
        }
        csv.close();
        std::cout << "\nResults saved to bench_opencl_results.csv" << std::endl;
        
    } catch (const cl::Error& err) {
        std::cerr << "OpenCL Error: " << err.what() << " (" << err.err() << ")" << std::endl;
        return 1;
    }
    
    return 0;
}
