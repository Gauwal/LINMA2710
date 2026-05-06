#!/bin/bash
#SBATCH --job-name=LINMA-medium-400-1000
#SBATCH --partition=gpu
#SBATCH --time=00:15:00
#SBATCH --cpus-per-task=8
#SBATCH --ntasks=1
#SBATCH --mem=32000

set -e
module load GCC/13.3.0
cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks

echo "╔════════════════════════════════════════════════════╗"
echo "║  BENCHMARK JOB B: Sizes 400-1000 (10 mins)        ║"
echo "╚════════════════════════════════════════════════════╝"

g++ -O3 -fopenmp -std=c++17 -march=native \
    -I../project/include \
    -o bench_b bench_medium_400_1000.cpp \
    ../project/src/matrix.cpp

echo "Running..."
./bench_b

ls -lh results_sizes_400_1000.csv
echo "✓ Done!"
