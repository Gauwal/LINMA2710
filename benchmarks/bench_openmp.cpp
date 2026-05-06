#include "matrix.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <fstream>
#include <omp.h>

struct BenchmarkResult {
    int matrix_size;
    int num_threads;
    double time_ms;
    double gflops;
};

std::vector<BenchmarkResult> results;

double benchmark_multiplication(int size, int num_threads) {
    Matrix A(size, size);
    Matrix B(size, size);
    A.fill(1.0);
    B.fill(2.0);
    
    omp_set_num_threads(num_threads);
    
    auto start = std::chrono::high_resolution_clock::now();
    Matrix C = A * B;
    auto end = std::chrono::high_resolution_clock::now();
    
    double elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
    
    return elapsed_ms;
}

int main() {
    std::cout << "=== Part 1 & 2: Sequential + OpenMP Benchmark ===" << std::endl;
    std::cout << "CPU: " << omp_get_max_threads() << " cores available" << std::endl << std::endl;
    
    std::vector<int> sizes = {100, 200, 500, 1000, 2000};
    std::vector<int> thread_counts = {1, 2, 4, 8, 16};
    
    // Warmup
    std::cout << "Warmup..." << std::endl;
    benchmark_multiplication(100, 1);
    
    std::cout << "\n=== Matrix Multiplication Benchmark ===" << std::endl;
    std::cout << std::left << std::setw(15) << "Size" 
              << std::left << std::setw(15) << "Threads" 
              << std::left << std::setw(20) << "Time (ms)" 
              << std::left << std::setw(15) << "GFLOPs" << std::endl;
    std::cout << std::string(65, '-') << std::endl;
    
    for (int size : sizes) {
        for (int threads : thread_counts) {
            if (threads > omp_get_max_threads()) continue;
            
            double time_ms = benchmark_multiplication(size, threads);
            double flops = 2.0 * size * size * size;
            double gflops = flops / (time_ms * 1e6);
            
            BenchmarkResult res;
            res.matrix_size = size;
            res.num_threads = threads;
            res.time_ms = time_ms;
            res.gflops = gflops;
            results.push_back(res);
            
            std::cout << std::left << std::setw(15) << size
                      << std::left << std::setw(15) << threads
                      << std::left << std::setw(20) << std::fixed << std::setprecision(3) << time_ms
                      << std::left << std::setw(15) << std::setprecision(2) << gflops << std::endl;
        }
        std::cout << std::string(65, '-') << std::endl;
    }
    
    // Compute speedup
    std::cout << "\n=== Speedup Analysis (vs 1 thread) ===" << std::endl;
    std::cout << std::left << std::setw(15) << "Size" 
              << std::left << std::setw(15) << "Threads" 
              << std::left << std::setw(20) << "Speedup" << std::endl;
    std::cout << std::string(50, '-') << std::endl;
    
    for (int size : sizes) {
        double baseline = 0;
        for (const auto& r : results) {
            if (r.matrix_size == size && r.num_threads == 1) {
                baseline = r.time_ms;
                break;
            }
        }
        
        if (baseline > 0) {
            for (int threads : thread_counts) {
                if (threads > omp_get_max_threads()) continue;
                
                for (const auto& r : results) {
                    if (r.matrix_size == size && r.num_threads == threads) {
                        double speedup = baseline / r.time_ms;
                        std::cout << std::left << std::setw(15) << size
                                  << std::left << std::setw(15) << threads
                                  << std::left << std::setw(20) << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
                        break;
                    }
                }
            }
            std::cout << std::string(50, '-') << std::endl;
        }
    }
    
    // Save results to CSV
    std::ofstream csv("bench_openmp_results.csv");
    csv << "matrix_size,num_threads,time_ms,gflops\n";
    for (const auto& r : results) {
        csv << r.matrix_size << "," << r.num_threads << "," 
            << r.time_ms << "," << r.gflops << "\n";
    }
    csv.close();
    std::cout << "\nResults saved to bench_openmp_results.csv" << std::endl;
    
    return 0;
}
