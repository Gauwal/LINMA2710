#include "matrix.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <fstream>
#include <omp.h>
#include <algorithm>

struct BenchmarkResult {
    int matrix_size;
    int num_threads;
    double time_ms;
    double gflops;
    double bandwidth_gb_s;
};

std::vector<BenchmarkResult> results;

void benchmark_multiplication(int size, int num_threads, int num_runs = 3) {
    Matrix A(size, size);
    Matrix B(size, size);
    A.fill(1.0);
    B.fill(2.0);
    
    omp_set_num_threads(num_threads);
    
    std::vector<double> times;
    
    for (int run = 0; run < num_runs; run++) {
        auto start = std::chrono::high_resolution_clock::now();
        Matrix C = A * B;
        auto end = std::chrono::high_resolution_clock::now();
        
        double elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
        times.push_back(elapsed_ms);
    }
    
    // Use median to avoid outliers
    std::sort(times.begin(), times.end());
    double time_ms = times[times.size() / 2];
    
    double flops = 2.0 * size * size * size;
    double gflops = flops / (time_ms * 1e6);
    
    // Memory: 3 matrices * size^2 * 8 bytes = 24 * size^2 bytes
    double memory_bytes = 3.0 * size * size * 8.0;
    double bandwidth_gb_s = (memory_bytes / (1024*1024*1024)) / (time_ms / 1000.0);
    
    BenchmarkResult res;
    res.matrix_size = size;
    res.num_threads = num_threads;
    res.time_ms = time_ms;
    res.gflops = gflops;
    res.bandwidth_gb_s = bandwidth_gb_s;
    results.push_back(res);
}

int main() {
    std::cout << "=== COMPREHENSIVE OpenMP Benchmark ===" << std::endl;
    std::cout << "CPU: " << omp_get_max_threads() << " cores available" << std::endl << std::endl;
    
    std::vector<int> sizes = {100, 200, 300, 500, 750, 1000, 1500, 2000};
    int max_threads = omp_get_max_threads();
    std::vector<int> thread_counts;
    
    // Generate thread counts based on available cores
    thread_counts.push_back(1);
    for (int t = 2; t <= max_threads; t *= 2) {
        thread_counts.push_back(t);
    }
    if (thread_counts.back() != max_threads) {
        thread_counts.push_back(max_threads);
    }
    
    std::cout << "Warmup..." << std::endl;
    benchmark_multiplication(100, 1, 1);
    results.clear();
    
    std::cout << "\n=== DETAILED MATRIX MULTIPLICATION BENCHMARK ===" << std::endl;
    std::cout << "Running " << sizes.size() << " sizes × " << thread_counts.size() 
              << " thread configs × 3 iterations each" << std::endl << std::endl;
    
    std::cout << std::left << std::setw(12) << "Size" 
              << std::left << std::setw(12) << "Threads" 
              << std::left << std::setw(15) << "Time (ms)" 
              << std::left << std::setw(15) << "GFLOPs"
              << std::left << std::setw(15) << "GB/s" << std::endl;
    std::cout << std::string(69, '-') << std::endl;
    
    for (int size : sizes) {
        for (int threads : thread_counts) {
            benchmark_multiplication(size, threads, 3);
            
            BenchmarkResult& res = results.back();
            std::cout << std::left << std::setw(12) << size
                      << std::left << std::setw(12) << threads
                      << std::left << std::setw(15) << std::fixed << std::setprecision(3) << res.time_ms
                      << std::left << std::setw(15) << std::setprecision(2) << res.gflops
                      << std::left << std::setw(15) << std::setprecision(2) << res.bandwidth_gb_s << std::endl;
        }
        std::cout << std::string(69, '-') << std::endl;
    }
    
    // Compute and display speedup
    std::cout << "\n=== SPEEDUP ANALYSIS (vs 1 thread) ===" << std::endl;
    std::cout << std::left << std::setw(12) << "Size" 
              << std::left << std::setw(12) << "Threads" 
              << std::left << std::setw(15) << "Speedup"
              << std::left << std::setw(15) << "Efficiency %" << std::endl;
    std::cout << std::string(54, '-') << std::endl;
    
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
                for (const auto& r : results) {
                    if (r.matrix_size == size && r.num_threads == threads) {
                        double speedup = baseline / r.time_ms;
                        double efficiency = (speedup / threads) * 100.0;
                        
                        std::cout << std::left << std::setw(12) << size
                                  << std::left << std::setw(12) << threads
                                  << std::left << std::setw(15) << std::fixed << std::setprecision(2) << speedup << "x"
                                  << std::left << std::setw(15) << std::setprecision(1) << efficiency << "%" << std::endl;
                        break;
                    }
                }
            }
            std::cout << std::string(54, '-') << std::endl;
        }
    }
    
    // Strong scaling analysis
    std::cout << "\n=== STRONG SCALING ANALYSIS ===" << std::endl;
    std::cout << "Speedup for fixed 1000×1000 matrix, increasing threads:" << std::endl;
    for (int threads : thread_counts) {
        for (const auto& r : results) {
            if (r.matrix_size == 1000 && r.num_threads == threads) {
                std::cout << "  " << threads << " threads: " << std::fixed << std::setprecision(2) 
                         << r.gflops << " GFLOPs" << std::endl;
                break;
            }
        }
    }
    
    // Weak scaling analysis (if sizes roughly double)
    std::cout << "\n=== WEAK SCALING ANALYSIS ===" << std::endl;
    std::cout << "Performance vs problem size (single thread):" << std::endl;
    for (int size : sizes) {
        for (const auto& r : results) {
            if (r.matrix_size == size && r.num_threads == 1) {
                std::cout << "  " << size << "×" << size << ": " << std::fixed << std::setprecision(2) 
                         << r.gflops << " GFLOPs (limited by memory)" << std::endl;
                break;
            }
        }
    }
    
    // Save results to CSV
    std::ofstream csv("bench_openmp_results_comprehensive.csv");
    csv << "matrix_size,num_threads,time_ms,gflops,bandwidth_gb_s\n";
    for (const auto& r : results) {
        csv << r.matrix_size << "," << r.num_threads << "," 
            << r.time_ms << "," << r.gflops << "," << r.bandwidth_gb_s << "\n";
    }
    csv.close();
    
    std::cout << "\n✓ Results saved to bench_openmp_results_comprehensive.csv" << std::endl;
    std::cout << "Total measurements: " << results.size() << std::endl;
    
    return 0;
}
