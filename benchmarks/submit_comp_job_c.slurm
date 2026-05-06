#!/bin/bash
#SBATCH --job-name=LINMA-comp-C
#SBATCH --partition=gpu
#SBATCH --time=00:35:00
#SBATCH --cpus-per-task=16
#SBATCH --ntasks=1
#SBATCH --mem=64000

set -e
module load GCC/13.3.0

cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks

echo "╔═══════════════════════════════════════════════════════════╗"
echo "║  JOB C: Comprehensive Benchmark Sizes 1100-2000          ║"
echo "║  OpenMP: 8 sizes × adaptive threads × 5 runs each        ║"
echo "╚═══════════════════════════════════════════════════════════╝"
echo ""

g++ -O3 -fopenmp -std=c++17 -march=native \
    -I../project/include \
    -o comp_job_c comp_job_c_1100_2000.cpp \
    ../project/src/matrix.cpp

echo "Starting benchmarks..."
./comp_job_c

ls -lh comprehensive_results_1100_2000.csv
echo "✓ Job C complete!"
