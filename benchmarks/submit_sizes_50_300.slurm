#!/bin/bash
#SBATCH --job-name=LINMA-small-50-300
#SBATCH --partition=gpu
#SBATCH --time=00:10:00
#SBATCH --cpus-per-task=8
#SBATCH --ntasks=1
#SBATCH --mem=16000

set -e
module load GCC/13.3.0
cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks

echo "╔════════════════════════════════════════════════════╗"
echo "║  BENCHMARK JOB A: Sizes 50-300 (5 mins)           ║"
echo "╚════════════════════════════════════════════════════╝"

g++ -O3 -fopenmp -std=c++17 -march=native \
    -I../project/include \
    -o bench_a bench_small_50_300.cpp \
    ../project/src/matrix.cpp

echo "Running..."
./bench_a

ls -lh results_sizes_50_300.csv
echo "✓ Done!"
