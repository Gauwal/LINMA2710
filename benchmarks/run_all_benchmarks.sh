#!/bin/bash
# Run all benchmarks on CECI cluster

# Request GPU allocation (manneback)
# salloc --partition=gpu --gres=gpu:1 --time=2:00:00 --mem-per-cpu=2000

# Load required modules
module load CUDA

cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks

echo "=== Building benchmarks ==="
make clean
make all

echo ""
echo "=== Running OpenMP benchmark (Part 1 & 2) ==="
./bench_openmp

echo ""
echo "=== Running MPI benchmark (Part 3) ==="
mpirun -np 4 ./bench_mpi

echo ""
echo "=== Running OpenCL benchmark (Part 4) ==="
./bench_opencl

echo ""
echo "=== Generating plots ==="
python3 plot_results.py

echo ""
echo "=== All benchmarks complete! ==="
ls -lh *.png *.csv
