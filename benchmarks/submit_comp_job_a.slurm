#!/bin/bash
#SBATCH --job-name=LINMA-comp-A
#SBATCH --partition=gpu
#SBATCH --time=00:20:00
#SBATCH --cpus-per-task=16
#SBATCH --ntasks=1
#SBATCH --mem=32000

set -e
module load GCC/13.3.0

cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks

echo "╔═══════════════════════════════════════════════════════════╗"
echo "║  JOB A: Comprehensive Benchmark Sizes 50-400             ║"
echo "║  OpenMP: 8 sizes × adaptive threads × 5 runs each        ║"
echo "╚═══════════════════════════════════════════════════════════╝"
echo ""

g++ -O3 -fopenmp -std=c++17 -march=native \
    -I../project/include \
    -o comp_job_a comp_job_a_50_400.cpp \
    ../project/src/matrix.cpp

echo "Starting benchmarks..."
./comp_job_a

ls -lh comprehensive_results_50_400.csv
echo "✓ Job A complete!"
