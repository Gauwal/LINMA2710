#!/bin/bash
#SBATCH --job-name=LINMA-comp-B
#SBATCH --partition=gpu
#SBATCH --time=00:25:00
#SBATCH --cpus-per-task=16
#SBATCH --ntasks=1
#SBATCH --mem=48000

set -e
module load GCC/13.3.0

cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks

echo "╔═══════════════════════════════════════════════════════════╗"
echo "║  JOB B: Comprehensive Benchmark Sizes 450-1000           ║"
echo "║  OpenMP: 7 sizes × adaptive threads × 5 runs each        ║"
echo "╚═══════════════════════════════════════════════════════════╝"
echo ""

g++ -O3 -fopenmp -std=c++17 -march=native \
    -I../project/include \
    -o comp_job_b comp_job_b_450_1000.cpp \
    ../project/src/matrix.cpp

echo "Starting benchmarks..."
./comp_job_b

ls -lh comprehensive_results_450_1000.csv
echo "✓ Job B complete!"
