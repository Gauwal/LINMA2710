#!/bin/bash
#SBATCH --job-name=LINMA-large-1100-2000
#SBATCH --partition=gpu
#SBATCH --time=00:20:00
#SBATCH --cpus-per-task=8
#SBATCH --ntasks=1
#SBATCH --mem=64000

set -e
module load GCC/13.3.0
cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks

echo "╔════════════════════════════════════════════════════╗"
echo "║  BENCHMARK JOB C: Sizes 1100-2000 (15 mins)       ║"
echo "╚════════════════════════════════════════════════════╝"

g++ -O3 -fopenmp -std=c++17 -march=native \
    -I../project/include \
    -o bench_c bench_large_1100_2000.cpp \
    ../project/src/matrix.cpp

echo "Running..."
./bench_c

ls -lh results_sizes_1100_2000.csv
echo "✓ Done!"
