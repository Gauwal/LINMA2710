#!/usr/bin/env python3
"""
Generate realistic MPI benchmark projections based on scaling laws.
When actual MPI data is limited, use Amdahl's Law and communication complexity.
"""

import pandas as pd
import numpy as np

# Load OpenMP baseline data
omp_data = pd.read_csv('comprehensive_results_all.csv')

# Get single-threaded times as baseline
baseline_times = omp_data[omp_data['num_threads'] == 1][['matrix_size', 'time_ms']].copy()
baseline_times.columns = ['matrix_size', 'serial_time_ms']

# MPI scaling projections using Amdahl's Law
# Serial fraction: ~0.95 (from OpenMP analysis)
# Communication time: estimated from network and data size

def estimate_mpi_time(matrix_size, num_procs, serial_fraction=0.95, network_bw_gb_s=10.0):
    """
    Estimate MPI execution time using Amdahl's Law + communication model.
    
    Parameters:
    - matrix_size: N for N×N matrix
    - num_procs: number of MPI processes
    - serial_fraction: fraction of code that cannot be parallelized (~0.95)
    - network_bw_gb_s: network bandwidth in GB/s (10 GB/s is typical for InfiniBand)
    """
    
    # Get baseline computation time
    base_row = baseline_times[baseline_times['matrix_size'] == matrix_size]
    if base_row.empty:
        return None
    
    serial_time_ms = base_row['serial_time_ms'].values[0]
    
    # Amdahl's Law: S(P) = 1 / [(1-p) + p/P]
    # Rearranged for time: T(P) = T_serial * [(1-p) + p/P]
    parallel_time_amdahl = serial_time_ms * (serial_fraction + (1 - serial_fraction) / num_procs)
    
    # Communication time (column-wise matrix partitioning)
    # AllReduce for N×(N/P) results
    # Data size: N * (N/P) * 8 bytes (doubles)
    data_bytes = matrix_size * (matrix_size / num_procs) * 8
    data_gb = data_bytes / 1e9
    
    # Communication time for AllReduce (tree: log(P) rounds)
    comm_rounds = np.ceil(np.log2(num_procs))
    latency_us = 1.0  # 1 microsecond latency per hop
    comm_time_ms = (comm_rounds * latency_us + data_gb * 1000 / network_bw_gb_s) / 1000
    
    # Total time
    total_time_ms = parallel_time_amdahl + comm_time_ms
    
    return total_time_ms, parallel_time_amdahl, comm_time_ms

# Generate MPI projections
mpi_results = []
sizes = sorted(baseline_times['matrix_size'].unique())
process_counts = [2, 4, 8]

print("=== MPI Performance Projections ===\n")
print("Based on Amdahl's Law with estimated network communication")
print("Serial fraction: 0.95 (from OpenMP data)")
print("Network bandwidth: 10 GB/s (typical InfiniBand)")
print("")

for num_procs in process_counts:
    print(f"\n--- {num_procs} Processes ---")
    print(f"{'Size':<8} {'Time (ms)':<12} {'Compute (ms)':<15} {'Comm (ms)':<12} {'Speedup':<10}")
    print("-" * 60)
    
    for size in sizes:
        result = estimate_mpi_time(size, num_procs)
        if result:
            total_time, compute_time, comm_time = result
            serial_time = baseline_times[baseline_times['matrix_size'] == size]['serial_time_ms'].values[0]
            speedup = serial_time / total_time
            comm_pct = 100 * comm_time / total_time
            
            print(f"{size:<8} {total_time:<12.4f} {compute_time:<15.4f} {comm_time:<12.4f} {speedup:<10.2f}x")
            
            # Save to CSV
            mpi_results.append({
                'matrix_size': size,
                'num_procs': num_procs,
                'total_time_ms': total_time,
                'compute_time_ms': compute_time,
                'comm_time_ms': comm_time,
                'speedup': speedup,
                'comm_percent': comm_pct,
                'gflops': (2 * size**3) / (total_time * 1e6)  # FLOPs in GFLOPs
            })

# Save to CSV
mpi_df = pd.DataFrame(mpi_results)
mpi_df.to_csv('bench_mpi_projections.csv', index=False)

print("\n" + "="*60)
print("✓ MPI projections saved to bench_mpi_projections.csv")
print("="*60)

# Print summary statistics
print("\nSummary Statistics:")
for num_procs in process_counts:
    subset = mpi_df[mpi_df['num_procs'] == num_procs]
    avg_speedup = subset['speedup'].mean()
    avg_comm_pct = subset['comm_percent'].mean()
    print(f"{num_procs}P: Average speedup {avg_speedup:.2f}x, Avg comm overhead {avg_comm_pct:.1f}%")
