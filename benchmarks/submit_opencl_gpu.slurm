#!/bin/bash
#SBATCH --job-name=bench_opencl_gpu
#SBATCH --partition=gpu
#SBATCH --nodes=1
#SBATCH --gpus=1
#SBATCH --cpus-per-task=2
#SBATCH --time=00:10:00
#SBATCH --output=opencl_gpu_%j.log

cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks

echo "=== OpenCL GPU Benchmark ===" 
./bench_opencl

echo ""
echo "Done!"
