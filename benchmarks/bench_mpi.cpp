#include "distributed_matrix.hpp"
#include "matrix.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <fstream>
#include <mpi.h>

struct MPI_BenchResult {
    int matrix_size;
    int num_procs;
    double total_time;
    double compute_time;
    double comm_time;
    double gflops;
};

std::vector<MPI_BenchResult> results;

void benchmark_multiply_transposed(int size, int rank, int num_procs) {
    // Create distributed matrices
    Matrix A(size, size);
    Matrix B(size, size);
    A.fill(1.0);
    B.fill(2.0);
    
    DistributedMatrix dA(A, num_procs);
    DistributedMatrix dB(B, num_procs);
    
    // Synchronize before benchmark
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Total time
    double t_start = MPI_Wtime();
    
    // Compute time (local computation)
    double t_compute_start = MPI_Wtime();
    Matrix C = dA.multiplyTransposed(dB);
    double t_compute_end = MPI_Wtime();
    
    double t_end = MPI_Wtime();
    
    // Synchronize after benchmark
    MPI_Barrier(MPI_COMM_WORLD);
    
    double total_time = t_end - t_start;
    double compute_time = t_compute_end - t_compute_start;
    double comm_time = total_time - compute_time;
    
    // FLOPs: local computation + allreduce
    double flops = 2.0 * size * size * size;
    double gflops = flops / (total_time * 1e9);
    
    if (rank == 0) {
        MPI_BenchResult res;
        res.matrix_size = size;
        res.num_procs = num_procs;
        res.total_time = total_time;
        res.compute_time = compute_time;
        res.comm_time = comm_time;
        res.gflops = gflops;
        results.push_back(res);
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    
    if (rank == 0) {
        std::cout << "=== Part 3: Distributed Matrix (MPI) Benchmark ===" << std::endl;
        std::cout << "Number of processes: " << num_procs << std::endl << std::endl;
    }
    
    // Test only with available processes
    std::vector<int> sizes = {200, 500, 1000, 2000};
    
    if (rank == 0) {
        std::cout << "=== MultiplyTransposed Benchmark ===" << std::endl;
        std::cout << std::left << std::setw(15) << "Size" 
                  << std::left << std::setw(15) << "Total (s)" 
                  << std::left << std::setw(15) << "Compute (s)" 
                  << std::left << std::setw(15) << "Comm (s)"
                  << std::left << std::setw(15) << "Comm %"
                  << std::left << std::setw(15) << "GFLOPs" << std::endl;
        std::cout << std::string(90, '-') << std::endl;
    }
    
    for (int size : sizes) {
        benchmark_multiply_transposed(size, rank, num_procs);
    }
    
    if (rank == 0) {
        for (const auto& r : results) {
            double comm_percent = (r.comm_time / r.total_time) * 100.0;
            std::cout << std::left << std::setw(15) << r.matrix_size
                      << std::left << std::setw(15) << std::fixed << std::setprecision(4) << r.total_time
                      << std::left << std::setw(15) << std::setprecision(4) << r.compute_time
                      << std::left << std::setw(15) << std::setprecision(4) << r.comm_time
                      << std::left << std::setw(15) << std::fixed << std::setprecision(1) << comm_percent
                      << std::left << std::setw(15) << std::setprecision(2) << r.gflops << std::endl;
        }
        std::cout << std::string(90, '-') << std::endl;
        
        // Save results to CSV
        std::ofstream csv("bench_mpi_results.csv");
        csv << "matrix_size,num_procs,total_time,compute_time,comm_time,gflops\n";
        for (const auto& r : results) {
            csv << r.matrix_size << "," << r.num_procs << "," 
                << r.total_time << "," << r.compute_time << "," 
                << r.comm_time << "," << r.gflops << "\n";
        }
        csv.close();
        std::cout << "\nResults saved to bench_mpi_results.csv" << std::endl;
    }
    
    MPI_Finalize();
    return 0;
}
