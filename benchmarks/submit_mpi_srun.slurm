#!/bin/bash
#SBATCH --job-name=bench_mpi_scaled
#SBATCH --partition=keira
#SBATCH --ntasks=4
#SBATCH --cpus-per-task=1
#SBATCH --time=00:15:00
#SBATCH --output=mpi_results_%j.log

cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks

# Load MPI module if available
module load openmpi 2>/dev/null || module load mpich 2>/dev/null || echo "Using system MPI"

echo "=== Running MPI Benchmark with 4 processes ===" 
echo "Running on $(hostname)"
echo "Number of processes: $SLURM_NTASKS"
echo ""

# Run the benchmark directly - the binary should handle MPI internally
srun ./bench_mpi

echo ""
echo "MPI benchmark completed"
