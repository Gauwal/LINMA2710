#!/bin/bash
# Quick local benchmarking script (for development)
# This can run locally without SLURM

set -e

echo "=========================================="
echo "LINMA2710 Quick Benchmark (Local)"
echo "=========================================="

cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks

# Create output directory
mkdir -p benchmark_results

echo "Building benchmarks..."

# Check if project files exist
if [ ! -f /home/ucl/ingi/gsavary/LINMA2710/project/src/matrix.cpp ]; then
    echo "ERROR: Project files not found!"
    exit 1
fi

# Build OpenMP benchmark
if [ ! -f bench_openmp_comp ]; then
    echo "  Building OpenMP..."
    g++ -O3 -fopenmp -std=c++17 \
        -I/home/ucl/ingi/gsavary/LINMA2710/project/include \
        -o bench_openmp_comp bench_openmp_comprehensive.cpp \
        /home/ucl/ingi/gsavary/LINMA2710/project/src/matrix.cpp
fi

# Build MPI benchmark
if [ ! -f bench_mpi_comp ]; then
    echo "  Building MPI..."
    which mpic++ > /dev/null 2>&1 || module load OpenMPI
    mpic++ -O3 -std=c++17 \
        -I/home/ucl/ingi/gsavary/LINMA2710/project/include \
        -o bench_mpi_comp bench_mpi_comprehensive.cpp \
        /home/ucl/ingi/gsavary/LINMA2710/project/src/matrix.cpp \
        /home/ucl/ingi/gsavary/LINMA2710/project/src/distributed_matrix.cpp
fi

echo "✓ Build complete"
echo ""

# Get core count
CORES=$(nproc)
THREADS=$((CORES > 8 ? 8 : CORES))
MPI_PROCS=$((CORES > 4 ? 4 : 2))

echo "System info:"
echo "  Available cores: $CORES"
echo "  Using for OpenMP: $THREADS threads"
echo "  Using for MPI: $MPI_PROCS processes"
echo ""

# Run OpenMP
echo "=========================================="
echo "Running OpenMP Benchmark ($THREADS threads max)"
echo "=========================================="
export OMP_NUM_THREADS=$THREADS
time ./bench_openmp_comp

echo ""
cp bench_openmp_results_comprehensive.csv benchmark_results/

# Run MPI
echo ""
echo "=========================================="
echo "Running MPI Benchmark ($MPI_PROCS processes)"
echo "=========================================="
module load OpenMPI 2>/dev/null || true
time mpirun --oversubscribe -np $MPI_PROCS ./bench_mpi_comp

echo ""
cp bench_mpi_results_comprehensive.csv benchmark_results/

# Plot results
if command -v python3 &> /dev/null; then
    echo ""
    echo "=========================================="
    echo "Generating plots..."
    echo "=========================================="
    python3 plot_results.py
fi

echo ""
echo "✓ Complete!"
echo ""
echo "Results files:"
ls -lh bench_*_results_comprehensive.csv benchmark_results/

echo ""
echo "View plots:"
ls -lh figures/*.png 2>/dev/null || echo "(No plots generated)"
