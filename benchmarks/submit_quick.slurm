#!/bin/bash
#SBATCH --job-name=LINMA-quick-omp
#SBATCH --partition=gpu
#SBATCH --time=00:15:00
#SBATCH --cpus-per-task=8
#SBATCH --ntasks=1
#SBATCH --mem=8000

set -e

module load GCC/13.3.0

cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks

g++ -O3 -fopenmp -std=c++17 -march=native \
    -I../project/include \
    -o quick_bench_omp quick_benchmark_openmp.cpp \
    ../project/src/matrix.cpp

echo "Starting quick benchmark..."
./quick_bench_omp

echo "✓ Done! Check quick_openmp_results.csv"
