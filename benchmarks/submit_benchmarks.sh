#!/bin/bash
# SLURM submission script for cluster
#SBATCH --partition=gpu
#SBATCH --gres=gpu:1
#SBATCH --time=01:00:00
#SBATCH --mem-per-cpu=2000
#SBATCH --output=benchmark_%j.log

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
