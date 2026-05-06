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
    double time_ms, gflops, efficiency;
};

std::vector<Result> results;

void benchmark(int size, int num_threads) {
    Matrix A(size, size), B(size, size);
    A.fill(1.0);
    B.fill(2.0);
    
    // Warmup
    auto dummy = A * B;
    
    std::vector<double> times;
    for (int run = 0; run < 3; run++) {
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
    double time_ms = times[1];  // median
    double flops = 2.0 * size * size * size;
    double gflops = flops / (time_ms * 1e6);
    double efficiency = gflops / 3.5 * 100;
    
    results.push_back({size, num_threads, time_ms, gflops, efficiency});
    
    std::cout << "Size: " << std::setw(4) << size << " | Threads: " << std::setw(2) << num_threads 
              << " | " << std::setw(8) << std::fixed << std::setprecision(2) << time_ms << "ms | "
              << std::setw(6) << std::fixed << std::setprecision(2) << gflops << " GFLOPs | "
              << std::setw(6) << std::fixed << std::setprecision(1) << efficiency << "% eff\n";
}

int main() {
    std::cout << "\n╔═══════════════════════════════════════════════╗\n";
    std::cout << "║   QUICK OpenMP BENCHMARK (5 min runtime)   ║\n";
    std::cout << "╚═══════════════════════════════════════════════╝\n\n";
    
    std::vector<int> sizes = {100, 300, 500, 1000};
    std::vector<int> threads = {1, 2, 4, 8};
    
    std::cout << "Testing " << sizes.size() << " sizes × " << threads.size() << " thread counts = "
              << (sizes.size() * threads.size()) << " configurations\n\n";
    
    for (int size : sizes) {
        std::cout << "Matrix size " << size << "x" << size << ":\n";
        for (int t : threads) {
            benchmark(size, t);
        }
        std::cout << "\n";
    }
    
    // Save results
    std::ofstream file("quick_openmp_results.csv");
    file << "matrix_size,num_threads,time_ms,gflops,efficiency\n";
    for (auto& r : results) {
        file << r.size << "," << r.threads << "," << r.time_ms << "," << r.gflops << "," << r.efficiency << "\n";
    }
    file.close();
    
    std::cout << "Results saved to: quick_openmp_results.csv\n\n";
    return 0;
}
