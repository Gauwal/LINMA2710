#!/usr/bin/env python3
"""
Plotting script for LINMA2710 project benchmarks.
Generates publication-quality figures for the presentation.
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os
import sys

# Set style for better-looking plots
plt.style.use('seaborn-v0_8-darkgrid')
plt.rcParams['figure.figsize'] = (12, 8)
plt.rcParams['font.size'] = 11
plt.rcParams['lines.linewidth'] = 2.5

output_dir = "figures"
if not os.path.exists(output_dir):
    os.makedirs(output_dir)

def load_csv(filename):
    """Load CSV file if it exists."""
    if os.path.exists(filename):
        return pd.read_csv(filename)
    else:
        print(f"Warning: {filename} not found")
        return None

def plot_openmp_speedup():
    """Plot OpenMP speedup vs threads and matrix size."""
    df = load_csv("bench_openmp_results.csv")
    if df is None:
        return
    
    fig, axes = plt.subplots(1, 2, figsize=(14, 5))
    
    # Plot 1: Speedup vs Threads for each size
    ax = axes[0]
    for size in sorted(df['matrix_size'].unique()):
        size_data = df[df['matrix_size'] == size].sort_values('num_threads')
        baseline_time = size_data[size_data['num_threads'] == 1]['time_ms'].values[0]
        speedup = baseline_time / size_data['time_ms']
        ax.plot(size_data['num_threads'], speedup, marker='o', label=f"Size {size}x{size}")
    
    ax.axline((1, 1), slope=1, color='r', linestyle='--', alpha=0.5, label="Ideal speedup")
    ax.set_xlabel("Number of Threads")
    ax.set_ylabel("Speedup")
    ax.set_title("OpenMP Speedup vs Number of Threads")
    ax.legend()
    ax.grid(True, alpha=0.3)
    
    # Plot 2: GFLOPs vs Matrix Size
    ax = axes[1]
    for threads in sorted(df['num_threads'].unique()):
        thread_data = df[df['num_threads'] == threads].sort_values('matrix_size')
        ax.plot(thread_data['matrix_size'], thread_data['gflops'], 
                marker='s', label=f"{threads} thread(s)")
    
    ax.set_xlabel("Matrix Size (N×N)")
    ax.set_ylabel("Performance (GFLOPs)")
    ax.set_title("Matrix Multiplication Performance")
    ax.set_xscale('log')
    ax.legend()
    ax.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(f"{output_dir}/01_openmp_speedup.png", dpi=300, bbox_inches='tight')
    print("✓ Saved: 01_openmp_speedup.png")
    plt.close()

def plot_mpi_communication():
    """Plot MPI communication breakdown."""
    df = load_csv("bench_mpi_results.csv")
    if df is None:
        return
    
    fig, axes = plt.subplots(1, 2, figsize=(14, 5))
    
    # Plot 1: Execution time breakdown
    ax = axes[0]
    df_sorted = df.sort_values('matrix_size')
    x = np.arange(len(df_sorted))
    width = 0.35
    
    compute_time = df_sorted['compute_time'].values
    comm_time = df_sorted['comm_time'].values
    
    ax.bar(x - width/2, compute_time, width, label="Computation", alpha=0.8)
    ax.bar(x + width/2, comm_time, width, label="Communication", alpha=0.8)
    
    ax.set_xlabel("Matrix Size (N×N)")
    ax.set_ylabel("Time (seconds)")
    ax.set_title("MPI: Computation vs Communication Time")
    ax.set_xticks(x)
    ax.set_xticklabels(df_sorted['matrix_size'].values)
    ax.legend()
    ax.grid(True, alpha=0.3, axis='y')
    
    # Plot 2: Communication percentage
    ax = axes[1]
    comm_percent = (df_sorted['comm_time'] / df_sorted['total_time'] * 100).values
    compute_percent = 100 - comm_percent
    
    ax.plot(df_sorted['matrix_size'], comm_percent, marker='o', 
            label="Communication %", linewidth=2.5, markersize=8, color='#d62728')
    ax.fill_between(df_sorted['matrix_size'], 0, comm_percent, alpha=0.2, color='#d62728')
    
    ax.set_xlabel("Matrix Size (N×N)")
    ax.set_ylabel("Percentage (%)")
    ax.set_title("Communication Overhead vs Matrix Size")
    ax.set_ylim([0, 100])
    ax.grid(True, alpha=0.3)
    ax.legend()
    
    plt.tight_layout()
    plt.savefig(f"{output_dir}/02_mpi_communication.png", dpi=300, bbox_inches='tight')
    print("✓ Saved: 02_mpi_communication.png")
    plt.close()

def plot_opencl_kernels():
    """Plot OpenCL kernel comparison."""
    df = load_csv("bench_opencl_results.csv")
    if df is None:
        return
    
    fig, axes = plt.subplots(1, 2, figsize=(14, 5))
    
    # Plot 1: Kernel performance comparison
    ax = axes[0]
    sizes = sorted(df['matrix_size'].unique())
    naive_times = []
    opt_times = []
    
    for size in sizes:
        naive_row = df[(df['matrix_size'] == size) & (df['kernel_type'] == 'naive')]
        opt_row = df[(df['matrix_size'] == size) & (df['kernel_type'] == 'optimized')]
        
        if not naive_row.empty:
            naive_times.append(naive_row['time_ms'].values[0])
        if not opt_row.empty:
            opt_times.append(opt_row['time_ms'].values[0])
    
    x = np.arange(len(sizes))
    width = 0.35
    
    ax.bar(x - width/2, naive_times, width, label="Naive Kernel", alpha=0.8)
    ax.bar(x + width/2, opt_times, width, label="Optimized Kernel", alpha=0.8)
    
    ax.set_xlabel("Matrix Size (N×N)")
    ax.set_ylabel("Execution Time (ms)")
    ax.set_title("OpenCL: Kernel Performance Comparison")
    ax.set_xticks(x)
    ax.set_xticklabels(sizes)
    ax.legend()
    ax.grid(True, alpha=0.3, axis='y')
    
    # Plot 2: Speedup of optimized kernel
    ax = axes[1]
    if len(naive_times) > 0 and len(opt_times) > 0:
        speedup = np.array(naive_times) / np.array(opt_times)
        ax.plot(sizes, speedup, marker='s', linewidth=2.5, markersize=8, 
                color='#2ca02c', label="Speedup")
        ax.axhline(y=1, color='r', linestyle='--', alpha=0.5, label="No speedup")
        ax.fill_between(sizes, 1, speedup, alpha=0.2, color='#2ca02c')
        
        ax.set_xlabel("Matrix Size (N×N)")
        ax.set_ylabel("Speedup (Naive / Optimized)")
        ax.set_title("Optimization Benefits")
        ax.legend()
        ax.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(f"{output_dir}/03_opencl_kernels.png", dpi=300, bbox_inches='tight')
    print("✓ Saved: 03_opencl_kernels.png")
    plt.close()

def plot_gflops_comparison():
    """Compare GFLOPs across all implementations."""
    openmp_df = load_csv("bench_openmp_results.csv")
    mpi_df = load_csv("bench_mpi_results.csv")
    opencl_df = load_csv("bench_opencl_results.csv")
    
    fig, ax = plt.subplots(figsize=(12, 6))
    
    # OpenMP (best case: max threads)
    if openmp_df is not None:
        max_threads = openmp_df['num_threads'].max()
        openmp_best = openmp_df[openmp_df['num_threads'] == max_threads].sort_values('matrix_size')
        ax.plot(openmp_best['matrix_size'], openmp_best['gflops'], 
                marker='o', linewidth=2.5, markersize=8, label=f"OpenMP ({max_threads} threads)")
    
    # MPI (4 processes)
    if mpi_df is not None:
        mpi_df_sorted = mpi_df.sort_values('matrix_size')
        ax.plot(mpi_df_sorted['matrix_size'], mpi_df_sorted['gflops'],
                marker='s', linewidth=2.5, markersize=8, label="MPI (4 processes)")
    
    # OpenCL (optimized kernel)
    if opencl_df is not None:
        opencl_opt = opencl_df[opencl_df['kernel_type'] == 'optimized'].sort_values('matrix_size')
        ax.plot(opencl_opt['matrix_size'], opencl_opt['gflops'],
                marker='^', linewidth=2.5, markersize=8, label="OpenCL (Optimized)")
    
    ax.set_xlabel("Matrix Size (N×N)")
    ax.set_ylabel("Performance (GFLOPs)")
    ax.set_title("Performance Comparison: OpenMP vs MPI vs OpenCL")
    ax.set_xscale('log')
    ax.legend(fontsize=10)
    ax.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(f"{output_dir}/04_gflops_comparison.png", dpi=300, bbox_inches='tight')
    print("✓ Saved: 04_gflops_comparison.png")
    plt.close()

def create_summary_table():
    """Create a summary table with key metrics."""
    fig, ax = plt.subplots(figsize=(12, 6))
    ax.axis('tight')
    ax.axis('off')
    
    # Collect key metrics
    data = []
    
    # OpenMP
    openmp_df = load_csv("bench_openmp_results.csv")
    if openmp_df is not None:
        max_threads = openmp_df['num_threads'].max()
        size_1000 = openmp_df[(openmp_df['matrix_size'] == 1000) & 
                              (openmp_df['num_threads'] == max_threads)]
        if not size_1000.empty:
            data.append(["OpenMP", f"{max_threads} threads", "1000×1000",
                        f"{size_1000['time_ms'].values[0]:.2f} ms",
                        f"{size_1000['gflops'].values[0]:.1f} GFLOPs"])
    
    # MPI
    mpi_df = load_csv("bench_mpi_results.csv")
    if mpi_df is not None:
        size_1000 = mpi_df[mpi_df['matrix_size'] == 1000]
        if not size_1000.empty:
            comm_percent = (size_1000['comm_time'].values[0] / 
                           size_1000['total_time'].values[0] * 100)
            data.append(["MPI", "4 processes", "1000×1000",
                        f"{size_1000['total_time'].values[0]:.4f} s",
                        f"{comm_percent:.1f}% comm overhead"])
    
    # OpenCL
    opencl_df = load_csv("bench_opencl_results.csv")
    if opencl_df is not None:
        size_512 = opencl_df[(opencl_df['matrix_size'] == 512) & 
                             (opencl_df['kernel_type'] == 'optimized')]
        if not size_512.empty:
            data.append(["OpenCL", "Optimized", "512×512",
                        f"{size_512['time_ms'].values[0]:.2f} ms",
                        f"{size_512['gflops'].values[0]:.1f} GFLOPs"])
    
    columns = ["Paradigm", "Configuration", "Problem Size", "Time", "Performance"]
    
    table = ax.table(cellText=data, colLabels=columns, cellLoc='center', loc='center',
                    colWidths=[0.15, 0.2, 0.15, 0.2, 0.2])
    table.auto_set_font_size(False)
    table.set_fontsize(10)
    table.scale(1, 2.5)
    
    # Style header
    for i in range(len(columns)):
        table[(0, i)].set_facecolor('#40466e')
        table[(0, i)].set_text_props(weight='bold', color='white')
    
    # Style rows
    colors = ['#f0f0f0', '#ffffff']
    for i in range(1, len(data) + 1):
        for j in range(len(columns)):
            table[(i, j)].set_facecolor(colors[i % 2])
    
    plt.title("Summary of Key Benchmarks", fontsize=14, weight='bold', pad=20)
    plt.savefig(f"{output_dir}/05_summary_table.png", dpi=300, bbox_inches='tight')
    print("✓ Saved: 05_summary_table.png")
    plt.close()

def main():
    """Generate all plots."""
    print("=" * 60)
    print("LINMA2710 Project - Benchmark Visualization")
    print("=" * 60)
    print()
    
    print("Checking for benchmark results...")
    if not os.path.exists("bench_openmp_results.csv") and \
       not os.path.exists("bench_mpi_results.csv") and \
       not os.path.exists("bench_opencl_results.csv"):
        print("No benchmark results found. Run benchmarks first!")
        print("  $ make run_all")
        return
    
    print()
    print("Generating plots...")
    plot_openmp_speedup()
    plot_mpi_communication()
    plot_opencl_kernels()
    plot_gflops_comparison()
    create_summary_table()
    
    print()
    print("=" * 60)
    print("All plots saved to:", output_dir)
    print("=" * 60)

if __name__ == "__main__":
    main()
