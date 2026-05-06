#!/bin/bash
# Quick local OpenMP benchmark - runs in ~5-10 minutes

set -e

cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks

echo "╔════════════════════════════════════════════════════════╗"
echo "║     QUICK LOCAL BENCHMARK - 5-10 minute runtime       ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""

# Compile
echo "Compiling quick benchmark..."
g++ -O3 -fopenmp -std=c++17 -march=native \
    -I../project/include \
    -o quick_bench_omp quick_benchmark_openmp.cpp \
    ../project/src/matrix.cpp 2>&1 | head -20

if [ ! -f quick_bench_omp ]; then
    echo "❌ Compilation failed!"
    exit 1
fi

echo "✓ Compiled successfully"
echo ""

# Run
echo "Running benchmark (4 matrix sizes × 4 thread counts = 16 tests)..."
time ./quick_bench_omp

echo ""
echo "Results saved to quick_openmp_results.csv"
echo ""
ls -lh quick_openmp_results.csv

