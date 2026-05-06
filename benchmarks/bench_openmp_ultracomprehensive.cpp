#include "matrix.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <fstream>
#include <omp.h>
#include <algorithm>
#include <cmath>

struct BenchmarkResult {
    int matrix_size;
    int num_threads;
    double time_ms;
    double gflops;
    double bandwidth_gb_s;
    double efficiency;
    int run_num;
};

std::vector<BenchmarkResult> results;

double measure_thread_overhead(int size, int num_threads) {
    /**
     * Measure the overhead of thread creation and synchronization
     */
    Matrix A(size, size);
    Matrix B(size, size);
    A.fill(1.0);
    B.fill(2.0);
    
    double* B_data = B.getData();
    
    // Warmup with threads to ensure they're created
    #pragma omp parallel for num_threads(num_threads)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            B_data[i * size + j] *= 1.0001;
        }
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    double* A_data = A.getData();
    #pragma omp parallel for num_threads(num_threads) collapse(2)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double sum = 0.0;
            for (int k = 0; k < size; k++) {
                sum += A_data[i * size + k] * B_data[k * size + j];
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

void benchmark_multiplication(int size, int num_threads, int num_runs = 5) {
    Matrix A(size, size);
    Matrix B(size, size);
    A.fill(1.0);
    B.fill(2.0);
    
    omp_set_num_threads(num_threads);
    
    std::vector<double> times;
    
    // Warmup run
    auto start = std::chrono::high_resolution_clock::now();
    Matrix C = A * B;
    auto end = std::chrono::high_resolution_clock::now();
    
    for (int run = 0; run < num_runs; run++) {
        start = std::chrono::high_resolution_clock::now();
        C = A * B;
        end = std::chrono::high_resolution_clock::now();
        
        double elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
        times.push_back(elapsed_ms);
    }
    
    // Use median
    std::sort(times.begin(), times.end());
    double time_ms = times[times.size() / 2];
    
    double flops = 2.0 * size * size * size;
    double gflops = flops / (time_ms * 1e6);
    
    // Memory: 3 matrices * size^2 * 8 bytes
    double memory_bytes = 3.0 * size * size * 8.0;
    double bandwidth_gb_s = (memory_bytes / (1024*1024*1024)) / (time_ms / 1000.0);
    
    // For now, estimate efficiency (will be refined)
    double efficiency = gflops / 2.1;  // Approximate peak as 2.1 GFLOPs
    
    BenchmarkResult res;
    res.matrix_size = size;
    res.num_threads = num_threads;
    res.time_ms = time_ms;
    res.gflops = gflops;
    res.bandwidth_gb_s = bandwidth_gb_s;
    res.efficiency = efficiency;
    res.run_num = num_runs;
    results.push_back(res);
}

int main() {
    std::cout << "\n╔════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║     COMPREHENSIVE OPENMP BENCHMARK - EXTENSIVE DATA COLLECTION     ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════╝\n\n";
    
    int max_threads = omp_get_max_threads();
    std::cout << "System: " << max_threads << " cores available\n";
    std::cout << "Running COMPREHENSIVE benchmark with multiple sizes and thread counts...\n\n";
    
    // More comprehensive matrix sizes (smaller to larger)
    std::vector<int> sizes = {
        50, 100, 150, 200, 250, 300, 400, 500, 600, 700, 800, 
        900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1800, 2000
    };
    
    // Thread configurations
    std::vector<int> thread_configs;
    thread_configs.push_back(1);  // Baseline
    for (int t = 2; t <= max_threads; t *= 2) {
        thread_configs.push_back(t);
    }
    if (thread_configs.back() != max_threads && max_threads > 1) {
        thread_configs.push_back(max_threads);
    }
    
    std::cout << "Matrix sizes: " << sizes.size() << " (from 50 to 2000)\n";
    std::cout << "Thread configs: " << thread_configs.size() << " (";
    for (size_t i = 0; i < thread_configs.size(); i++) {
        std::cout << thread_configs[i];
        if (i < thread_configs.size() - 1) std::cout << ", ";
    }
    std::cout << ")\n";
    std::cout << "Total configurations: " << (sizes.size() * thread_configs.size()) << "\n";
    std::cout << "Total runs per config: 5 (median selected)\n";
    std::cout << "Expected measurements: " << (sizes.size() * thread_configs.size() * 5) << "\n\n";
    
    // Warmup
    std::cout << "Warmup run...\n";
    benchmark_multiplication(100, 1, 1);
    results.clear();
    
    // Main benchmarking
    std::cout << "\n╔════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║              STARTING COMPREHENSIVE BENCHMARKING                   ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════╝\n\n";
    
    int total_configs = sizes.size() * thread_configs.size();
    int current_config = 0;
    
    std::cout << std::left << std::setw(8) << "Size"
              << std::left << std::setw(10) << "Threads"
              << std::left << std::setw(12) << "Time (ms)"
              << std::left << std::setw(12) << "GFLOPs"
              << std::left << std::setw(12) << "GB/s"
              << std::left << std::setw(8) << "Progress" << std::endl;
    std::cout << std::string(62, '─') << std::endl;
    
    for (int size : sizes) {
        for (int threads : thread_configs) {
            current_config++;
            
            benchmark_multiplication(size, threads, 5);
            
            BenchmarkResult& res = results.back();
            
            // Calculate progress
            float progress = (float)current_config / total_configs * 100.0;
            
            std::cout << std::left << std::setw(8) << size
                      << std::left << std::setw(10) << threads
                      << std::left << std::setw(12) << std::fixed << std::setprecision(3) << res.time_ms
                      << std::left << std::setw(12) << std::setprecision(2) << res.gflops
                      << std::left << std::setw(12) << std::setprecision(2) << res.bandwidth_gb_s
                      << std::left << std::setw(8) << std::setprecision(1) << progress << "%\r";
            std::cout.flush();
        }
    }
    
    std::cout << std::string(62, ' ') << "\r";  // Clear line
    std::cout << "✓ Benchmarking complete!\n\n";
    
    // ========================================================================
    // ANALYSIS & REPORTING
    // ========================================================================
    
    std::cout << "╔════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                        ANALYSIS & FINDINGS                        ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════╝\n\n";
    
    // === KEY METRICS ===
    std::cout << "📊 KEY PERFORMANCE METRICS:\n";
    std::cout << std::string(64, '─') << std::endl;
    
    // Find performance at different sizes
    std::vector<int> key_sizes = {100, 500, 1000, 1500, 2000};
    for (int size : key_sizes) {
        auto size_results = std::find_if(results.begin(), results.end(),
            [size](const BenchmarkResult& r) { return r.matrix_size == size && r.num_threads == 1; });
        
        if (size_results != results.end()) {
            std::cout << "\n" << size << "×" << size << " matrix (1 thread):\n";
            std::cout << "  Time: " << std::fixed << std::setprecision(3) << size_results->time_ms << " ms\n";
            std::cout << "  GFLOPs: " << std::setprecision(2) << size_results->gflops << " GFLOP/s\n";
            std::cout << "  Bandwidth: " << size_results->bandwidth_gb_s << " GB/s\n";
        }
    }
    
    // === SPEEDUP ANALYSIS ===
    std::cout << "\n\n📈 SPEEDUP ANALYSIS (1000×1000 matrix):\n";
    std::cout << std::string(64, '─') << std::endl;
    std::cout << std::left << std::setw(12) << "Threads"
              << std::left << std::setw(15) << "Time (ms)"
              << std::left << std::setw(15) << "Speedup"
              << std::left << std::setw(15) << "Efficiency"
              << std::left << std::setw(12) << "GFLOPs" << std::endl;
    std::cout << std::string(64, '─') << std::endl;
    
    auto size_1000 = [](const BenchmarkResult& r) { return r.matrix_size == 1000; };
    auto size_1000_results = std::vector<BenchmarkResult>();
    for (const auto& r : results) {
        if (size_1000(r)) {
            size_1000_results.push_back(r);
        }
    }
    
    std::sort(size_1000_results.begin(), size_1000_results.end(),
        [](const BenchmarkResult& a, const BenchmarkResult& b) {
            return a.num_threads < b.num_threads;
        });
    
    if (size_1000_results.size() > 0) {
        double baseline = size_1000_results[0].time_ms;
        
        for (const auto& r : size_1000_results) {
            double speedup = baseline / r.time_ms;
            double efficiency = (speedup / r.num_threads) * 100.0;
            
            std::cout << std::left << std::setw(12) << r.num_threads
                      << std::left << std::setw(15) << std::fixed << std::setprecision(3) << r.time_ms
                      << std::left << std::setw(15) << std::setprecision(2) << speedup << "×"
                      << std::left << std::setw(15) << std::setprecision(1) << efficiency << "%"
                      << std::left << std::setw(12) << std::setprecision(2) << r.gflops << std::endl;
        }
    }
    
    // === SCALING BEHAVIOR ===
    std::cout << "\n\n🔍 SCALING BEHAVIOR:\n";
    std::cout << std::string(64, '─') << std::endl;
    
    std::cout << "\nStrong Scaling (fixed 1000×1000, increasing threads):\n";
    if (size_1000_results.size() > 0) {
        double baseline_time = size_1000_results[0].time_ms;
        for (const auto& r : size_1000_results) {
            double speedup = baseline_time / r.time_ms;
            std::cout << "  " << std::setw(2) << r.num_threads << " thread(s): "
                     << std::fixed << std::setprecision(2) << speedup << "× speedup\n";
        }
    }
    
    std::cout << "\nWeak Scaling (constant time per thread, varying size):\n";
    std::cout << "  (Performance should remain roughly constant)\n";
    for (int t : {1, 2, 4}) {
        bool found = false;
        for (const auto& size : {100, 1000, 2000}) {
            auto it = std::find_if(results.begin(), results.end(),
                [size, t](const BenchmarkResult& r) {
                    return r.matrix_size == size && r.num_threads == t;
                });
            
            if (it != results.end()) {
                if (!found) {
                    std::cout << "  " << t << " thread(s): ";
                    found = true;
                }
                std::cout << size << "×" << size << "=" << std::fixed << std::setprecision(2)
                         << it->gflops << "GFLOP/s, ";
            }
        }
        if (found) std::cout << "\n";
    }
    
    // === MEMORY BANDWIDTH ===
    std::cout << "\n\n💾 MEMORY BANDWIDTH ANALYSIS:\n";
    std::cout << std::string(64, '─') << std::endl;
    
    std::cout << "Bandwidth utilization (1 thread, varying matrix size):\n";
    for (int size : {100, 500, 1000, 1500, 2000}) {
        auto it = std::find_if(results.begin(), results.end(),
            [size](const BenchmarkResult& r) {
                return r.matrix_size == size && r.num_threads == 1;
            });
        
        if (it != results.end()) {
            std::cout << std::right << std::setw(6) << size << "×" << std::left << std::setw(6) << size
                     << ": " << std::fixed << std::setprecision(2) << it->bandwidth_gb_s
                     << " GB/s\n";
        }
    }
    
    // Save detailed results
    std::ofstream csv("bench_openmp_results_ultracomprehensive.csv");
    csv << "matrix_size,num_threads,time_ms,gflops,bandwidth_gb_s,efficiency,run_num\n";
    for (const auto& r : results) {
        csv << r.matrix_size << "," << r.num_threads << "," 
            << r.time_ms << "," << r.gflops << "," << r.bandwidth_gb_s << ","
            << r.efficiency << "," << r.run_num << "\n";
    }
    csv.close();
    
    std::cout << "\n✅ Results saved to bench_openmp_results_ultracomprehensive.csv\n";
    std::cout << "   Total data points: " << results.size() << "\n\n";
    
    return 0;
}
