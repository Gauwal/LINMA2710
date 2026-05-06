#include "distributed_matrix.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <fstream>
#include <mpi.h>
#include <algorithm>

struct BenchmarkResult {
    int matrix_size;
    int num_procs;
    double total_time;
    double compute_time;
    double comm_time;
    double gflops;
};

std::vector<BenchmarkResult> results;

void benchmark_mpi_multiply(int size, int num_runs = 3) {
    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    
    DistributedMatrix A(size, size, MPI_COMM_WORLD);
    DistributedMatrix B(size, size, MPI_COMM_WORLD);
    
    if (rank == 0) {
        A.fill_root(1.0);
        B.fill_root(2.0);
    }
    A.broadcast_root(0);
    B.broadcast_root(0);
    
    std::vector<double> total_times;
    std::vector<double> compute_times;
    std::vector<double> comm_times;
    
    for (int run = 0; run < num_runs; run++) {
        MPI_Barrier(MPI_COMM_WORLD);
        
        // Total time
        double t_start = MPI_Wtime();
        
        // Compute time (just local multiplication)
        double c_start = MPI_Wtime();
        DistributedMatrix C = A.multiply_transposed(B);
        double c_end = MPI_Wtime();
        double compute_time = c_end - c_start;
        
        // All reduce (communication)
        double comm_start = MPI_Wtime();
        C.allreduce_sum();
        double comm_end = MPI_Wtime();
        double comm_time = comm_end - comm_start;
        
        double t_end = MPI_Wtime();
        double total_time = t_end - t_start;
        
        total_times.push_back(total_time);
        compute_times.push_back(compute_time);
        comm_times.push_back(comm_time);
    }
    
    // Use median
    std::sort(total_times.begin(), total_times.end());
    std::sort(compute_times.begin(), compute_times.end());
    std::sort(comm_times.begin(), comm_times.end());
    
    double med_total = total_times[total_times.size() / 2];
    double med_compute = compute_times[compute_times.size() / 2];
    double med_comm = comm_times[comm_times.size() / 2];
    
    double flops = 2.0 * size * size * size;
    double gflops = flops / (med_total * 1e9);
    
    BenchmarkResult res;
    res.matrix_size = size;
    res.num_procs = num_procs;
    res.total_time = med_total;
    res.compute_time = med_compute;
    res.comm_time = med_comm;
    res.gflops = gflops;
    results.push_back(res);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    
    if (rank == 0) {
        std::cout << "=== COMPREHENSIVE MPI Benchmark ===" << std::endl;
        std::cout << "Running with " << num_procs << " processes" << std::endl << std::endl;
    }
    
    std::vector<int> sizes = {100, 200, 300, 500, 750, 1000, 1200, 1500};
    
    // Warmup
    if (rank == 0) std::cout << "Warmup..." << std::endl;
    benchmark_mpi_multiply(100, 1);
    results.clear();
    
    if (rank == 0) {
        std::cout << "\n=== DETAILED MPI MATRIX MULTIPLICATION BENCHMARK ===" << std::endl;
        std::cout << "Running " << sizes.size() << " sizes × 3 iterations each" << std::endl << std::endl;
        
        std::cout << std::left << std::setw(12) << "Size" 
                  << std::left << std::setw(12) << "Procs"
                  << std::left << std::setw(15) << "Total (ms)" 
                  << std::left << std::setw(15) << "Compute (ms)"
                  << std::left << std::setw(15) << "Comm (ms)"
                  << std::left << std::setw(15) << "GFLOPs" << std::endl;
        std::cout << std::string(84, '-') << std::endl;
    }
    
    for (int size : sizes) {
        benchmark_mpi_multiply(size, 3);
        
        if (rank == 0) {
            BenchmarkResult& res = results.back();
            double total_ms = res.total_time * 1000;
            double compute_ms = res.compute_time * 1000;
            double comm_ms = res.comm_time * 1000;
            double comm_pct = (comm_ms / total_ms) * 100.0;
            
            std::cout << std::left << std::setw(12) << size
                      << std::left << std::setw(12) << num_procs
                      << std::left << std::setw(15) << std::fixed << std::setprecision(3) << total_ms
                      << std::left << std::setw(15) << std::setprecision(3) << compute_ms
                      << std::left << std::setw(15) << std::setprecision(3) << comm_ms << "(" 
                      << std::setprecision(1) << comm_pct << "%)"
                      << std::left << std::setw(15) << std::setprecision(2) << res.gflops << std::endl;
        }
    }
    
    if (rank == 0) {
        std::cout << std::string(84, '-') << std::endl;
        
        std::cout << "\n=== COMMUNICATION ANALYSIS ===" << std::endl;
        std::cout << "Percentage of time spent in MPI communication:" << std::endl;
        for (const auto& r : results) {
            double comm_pct = (r.comm_time / r.total_time) * 100.0;
            std::cout << "  " << r.matrix_size << "×" << r.matrix_size << ": " 
                     << std::fixed << std::setprecision(1) << comm_pct << "%" << std::endl;
        }
        
        std::cout << "\n=== SPEEDUP POTENTIAL ===" << std::endl;
        std::cout << "With " << num_procs << " processes, expected ~" << num_procs 
                 << "x speedup (if perfectly parallel)" << std::endl;
        for (const auto& r : results) {
            std::cout << "  " << r.matrix_size << "×" << r.matrix_size << ": " 
                     << std::fixed << std::setprecision(2) << r.gflops << " GFLOPs" << std::endl;
        }
        
        // Save results to CSV
        std::ofstream csv("bench_mpi_results_comprehensive.csv");
        csv << "matrix_size,num_procs,total_time,compute_time,comm_time,gflops\n";
        for (const auto& r : results) {
            csv << r.matrix_size << "," << r.num_procs << "," 
                << r.total_time << "," << r.compute_time << "," 
                << r.comm_time << "," << r.gflops << "\n";
        }
        csv.close();
        
        std::cout << "\n✓ Results saved to bench_mpi_results_comprehensive.csv" << std::endl;
    }
    
    MPI_Finalize();
    return 0;
}
