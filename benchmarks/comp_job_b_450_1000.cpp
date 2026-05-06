#include "matrix.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <fstream>
#include <omp.h>
#include <algorithm>
#include <cmath>

struct Result {
    int size, threads;
    double time_ms, gflops, bandwidth, efficiency;
};

std::vector<Result> results;

void benchmark_openmp(int size, int num_threads) {
    Matrix A(size, size), B(size, size);
    A.fill(1.0);
    B.fill(2.0);
    
    auto dummy = A * B;  // warmup
    
    std::vector<double> times;
    for (int run = 0; run < 5; run++) {
        auto start = std::chrono::high_resolution_clock::now();
        
        #pragma omp parallel for num_threads(num_threads) collapse(2)
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                double sum = 0.0;
                for (int k = 0; k < size; k++) {
                    sum += A.get(i, k) * B.get(k, j);
                }
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        times.push_back(std::chrono::duration<double, std::milli>(end - start).count());
    }
    
    std::sort(times.begin(), times.end());
    double time_ms = times[2];  // median
    double flops = 2.0 * size * size * size;
    double gflops = flops / (time_ms * 1e6);
    double bandwidth = (3.0 * size * size * 8) / (time_ms * 1e9);
    double efficiency = (gflops / 3.5) * 100;
    
    results.push_back({size, num_threads, time_ms, gflops, bandwidth, efficiency});
    
    std::cout << "  Size " << std::setw(4) << size << " | Threads " << std::setw(2) << num_threads 
              << " | " << std::setw(8) << std::fixed << std::setprecision(2) << time_ms 
              << "ms | " << std::setw(7) << std::fixed << std::setprecision(2) << gflops << " GF\n";
}

int main() {
    std::cout << "\n╔═══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  COMPREHENSIVE BENCHMARK JOB B: Sizes 450-1000           ║\n";
    std::cout << "║  OpenMP: Full thread scaling + 5 runs per config         ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════╝\n\n";
    
    std::vector<int> sizes = {450, 500, 600, 700, 800, 900, 1000};
    int max_threads = omp_get_max_threads();
    std::vector<int> threads;
    
    // Adaptive thread counts based on system
    for (int t = 1; t <= max_threads && threads.size() < 8; t *= 2) {
        threads.push_back(t);
    }
    if (threads.back() != max_threads) {
        threads.push_back(max_threads);
    }
    
    std::cout << "System: " << max_threads << " available cores\n";
    std::cout << "Testing: " << sizes.size() << " sizes × " << threads.size() 
              << " thread counts × 5 runs = " << (sizes.size() * threads.size() * 5)
              << " total benchmark runs\n\n";
    
    for (int size : sizes) {
        std::cout << "Size " << size << "×" << size << ":\n";
        for (int t : threads) {
            benchmark_openmp(size, t);
        }
    }
    
    // Save results
    std::ofstream file("comprehensive_results_450_1000.csv");
    file << "matrix_size,num_threads,time_ms,gflops,bandwidth_gb_s,efficiency\n";
    for (auto& r : results) {
        file << r.size << "," << r.threads << "," << r.time_ms << "," 
             << r.gflops << "," << r.bandwidth << "," << r.efficiency << "\n";
    }
    file.close();
    
    std::cout << "\n✓ Results saved: comprehensive_results_450_1000.csv\n";
    std::cout << "✓ Completed " << results.size() << " OpenMP measurements\n\n";
    return 0;
}
