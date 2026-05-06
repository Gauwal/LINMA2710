#!/bin/bash
#SBATCH --job-name=LINMA2710-comprehensive
#SBATCH --partition=gpu
#SBATCH --time=06:00:00
#SBATCH --cpus-per-task=16
#SBATCH --ntasks=4
#SBATCH --mem-per-cpu=2000
#SBATCH --output=benchmark_%j.log
#SBATCH --error=benchmark_%j.err

echo "=========================================="
echo "LINMA2710 Comprehensive Benchmark Suite"
echo "=========================================="
echo "Job ID: $SLURM_JOB_ID"
echo "Nodes: $SLURM_NNODES"
echo "CPUs per task: $SLURM_CPUS_PER_TASK"
echo "Tasks: $SLURM_NTASKS"
echo "Start time: $(date)"
echo ""

# Load required modules
module load GCC/13.3.0
module load OpenMPI
module load CUDA/12.8.0

# Create output directory
mkdir -p benchmark_results

echo "Compiling benchmarks..."
cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks

# Compile OpenMP comprehensive benchmark
echo "  Compiling OpenMP comprehensive..."
g++ -O3 -fopenmp -std=c++17 \
    -I/home/ucl/ingi/gsavary/LINMA2710/project/include \
    -o bench_openmp_comp bench_openmp_comprehensive.cpp \
    /home/ucl/ingi/gsavary/LINMA2710/project/src/matrix.cpp \
    2>&1 | tee compile_openmp.log

# Compile MPI comprehensive benchmark
echo "  Compiling MPI comprehensive..."
mpic++ -O3 -std=c++17 \
    -I/home/ucl/ingi/gsavary/LINMA2710/project/include \
    -o bench_mpi_comp bench_mpi_comprehensive.cpp \
    /home/ucl/ingi/gsavary/LINMA2710/project/src/matrix.cpp \
    /home/ucl/ingi/gsavary/LINMA2710/project/src/distributed_matrix.cpp \
    2>&1 | tee compile_mpi.log

if [ ! -f bench_openmp_comp ] || [ ! -f bench_mpi_comp ]; then
    echo "ERROR: Compilation failed!"
    exit 1
fi

echo "✓ Compilation successful"
echo ""

# Run OpenMP benchmark with multiple threads
echo "=========================================="
echo "RUNNING: OpenMP Benchmark"
echo "=========================================="
echo "Testing with 1 to $SLURM_CPUS_PER_TASK threads..."
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
./bench_openmp_comp

echo ""
echo "OpenMP results saved to: bench_openmp_results_comprehensive.csv"
ls -lh bench_openmp_results_comprehensive.csv

echo ""
echo "=========================================="
echo "RUNNING: MPI Benchmark"
echo "=========================================="
echo "Testing with $SLURM_NTASKS processes..."
mpirun --oversubscribe -np $SLURM_NTASKS ./bench_mpi_comp

echo ""
echo "MPI results saved to: bench_mpi_results_comprehensive.csv"
ls -lh bench_mpi_results_comprehensive.csv

# Copy results to backup
echo ""
echo "=========================================="
echo "BACKUP: Copying results to benchmark_results/"
cp bench_openmp_results_comprehensive.csv benchmark_results/
cp bench_mpi_results_comprehensive.csv benchmark_results/
cp compile_openmp.log benchmark_results/
cp compile_mpi.log benchmark_results/

echo "✓ All results backed up"
echo ""

# Generate new plots if python available
if command -v python3 &> /dev/null; then
    echo "=========================================="
    echo "GENERATING: Updated Plots"
    echo "=========================================="
    python3 plot_results.py 2>&1 | tee plot_results.log
    
    echo ""
    echo "Plots generated:"
    ls -lh figures/*.png 2>/dev/null || echo "No plots generated"
fi

echo ""
echo "=========================================="
echo "BENCHMARK SUMMARY"
echo "=========================================="
echo "Completion time: $(date)"
echo ""
echo "Generated files:"
echo "  - bench_openmp_results_comprehensive.csv"
echo "  - bench_mpi_results_comprehensive.csv"
echo "  - benchmark_results/ (backup)"
echo "  - figures/01_*.png through 05_*.png (updated plots)"
echo ""
echo "To view results:"
echo "  OpenMP:  cat bench_openmp_results_comprehensive.csv"
echo "  MPI:     cat bench_mpi_results_comprehensive.csv"
echo "  Plots:   Display figures/ directory files"
echo ""

exit 0
