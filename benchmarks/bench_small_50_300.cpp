#include "matrix.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <fstream>
#include <omp.h>
#include <algorithm>

struct Result {
    int size, threads;
    double time_ms, gflops;
};

std::vector<Result> results;

void benchmark(int size, int num_threads) {
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
    
    results.push_back({size, num_threads, time_ms, gflops});
    
    std::cout << "  Size " << std::setw(4) << size << " | Threads " << std::setw(2) << num_threads 
              << " | " << std::setw(8) << std::fixed << std::setprecision(2) << time_ms 
              << "ms | " << std::setw(7) << std::fixed << std::setprecision(2) << gflops << " GFLOPs\n";
}

int main() {
    std::cout << "\n╔════════════════════════════════════════════════╗\n";
    std::cout << "║   BENCHMARK JOB A: Sizes 50-300              ║\n";
    std::cout << "║   (5 matrix sizes × 4 threads × 5 runs)      ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";
    
    std::vector<int> sizes = {50, 100, 150, 200, 300};
    std::vector<int> threads = {1, 2, 4, 8};
    
    std::cout << "Total: " << sizes.size() << " sizes × " << threads.size() << " threads = "
              << (sizes.size() * threads.size()) << " tests\n\n";
    
    for (int size : sizes) {
        std::cout << "Size " << size << "×" << size << ":\n";
        for (int t : threads) {
            benchmark(size, t);
        }
    }
    
    std::ofstream file("results_sizes_50_300.csv");
    file << "matrix_size,num_threads,time_ms,gflops\n";
    for (auto& r : results) {
        file << r.size << "," << r.threads << "," << r.time_ms << "," << r.gflops << "\n";
    }
    file.close();
    
    std::cout << "\n✓ Results saved: results_sizes_50_300.csv\n\n";
    return 0;
}
