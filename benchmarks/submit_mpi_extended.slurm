#!/bin/bash
#SBATCH --job-name=bench_mpi_extended
#SBATCH --partition=keira
#SBATCH --nodes=1
#SBATCH --ntasks=8
#SBATCH --cpus-per-task=1
#SBATCH --time=00:10:00
#SBATCH --output=mpi_extended_%j.log

cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks

echo "=== MPI Benchmark with 2 processes ===" 
mpirun -np 2 ./bench_mpi

echo ""
echo "=== MPI Benchmark with 4 processes ===" 
mpirun -np 4 ./bench_mpi

echo ""
echo "=== MPI Benchmark with 8 processes ===" 
mpirun -np 8 ./bench_mpi
