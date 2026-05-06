#!/bin/bash
#SBATCH --job-name=LINMA2710-ultra-comprehensive
#SBATCH --partition=gpu
#SBATCH --time=08:00:00
#SBATCH --cpus-per-task=16
#SBATCH --ntasks=4
#SBATCH --mem-per-cpu=2000
#SBATCH --output=ultra_benchmark_%j.log
#SBATCH --error=ultra_benchmark_%j.err

set -e

echo "╔════════════════════════════════════════════════════════════════════╗"
echo "║           LINMA2710 ULTRA-COMPREHENSIVE BENCHMARK SUITE           ║"
echo "║              Extensive Data Collection & Analysis                  ║"
echo "╚════════════════════════════════════════════════════════════════════╝"
echo ""
echo "Job ID: $SLURM_JOB_ID"
echo "Start time: $(date)"
echo "Nodes: $SLURM_NNODES"
echo "CPUs per task: $SLURM_CPUS_PER_TASK"
echo "Tasks: $SLURM_NTASKS"
echo ""

# Load required modules
module load GCC/13.3.0
module load OpenMPI
module load CUDA/12.8.0

# Create output directories
mkdir -p ultra_benchmark_results
mkdir -p figures

cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks

echo "═══════════════════════════════════════════════════════════════════════"
echo "COMPILATION PHASE"
echo "═══════════════════════════════════════════════════════════════════════"
echo ""

# Compile OpenMP ultra-comprehensive
echo "Compiling OpenMP ultra-comprehensive benchmark..."
g++ -O3 -fopenmp -std=c++17 -march=native -ffast-math \
    -I/home/ucl/ingi/gsavary/LINMA2710/project/include \
    -o bench_omp_ultra bench_openmp_ultracomprehensive.cpp \
    /home/ucl/ingi/gsavary/LINMA2710/project/src/matrix.cpp \
    2>&1 | tee compile_omp_ultra.log

if [ ! -f bench_omp_ultra ]; then
    echo "❌ OpenMP compilation failed!"
    exit 1
fi
echo "✓ OpenMP compiled"

# Compile MPI ultra-comprehensive
echo "Compiling MPI ultra-comprehensive benchmark..."
mpic++ -O3 -std=c++17 -march=native -ffast-math \
    -I/home/ucl/ingi/gsavary/LINMA2710/project/include \
    -o bench_mpi_ultra bench_mpi_ultracomprehensive.cpp \
    /home/ucl/ingi/gsavary/LINMA2710/project/src/matrix.cpp \
    /home/ucl/ingi/gsavary/LINMA2710/project/src/distributed_matrix.cpp \
    2>&1 | tee compile_mpi_ultra.log

if [ ! -f bench_mpi_ultra ]; then
    echo "❌ MPI compilation failed!"
    exit 1
fi
echo "✓ MPI compiled"

echo ""
echo "═══════════════════════════════════════════════════════════════════════"
echo "BENCHMARKING PHASE"
echo "═══════════════════════════════════════════════════════════════════════"
echo ""

# Run OpenMP ultra-comprehensive
echo "🚀 RUNNING: OpenMP Ultra-Comprehensive Benchmark"
echo "   (21 matrix sizes × up to 16 threads = ~100+ configurations)"
echo "   Collecting ~500+ individual measurements..."
echo ""

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
time ./bench_omp_ultra 2>&1 | tee ultra_benchmark_results/openmp_ultra.log

if [ -f bench_openmp_results_ultracomprehensive.csv ]; then
    echo "✓ OpenMP results: $(wc -l < bench_openmp_results_ultracomprehensive.csv) data points"
    cp bench_openmp_results_ultracomprehensive.csv ultra_benchmark_results/
else
    echo "❌ OpenMP results file not found!"
fi

echo ""
echo "🚀 RUNNING: MPI Ultra-Comprehensive Benchmark"
echo "   (21 matrix sizes × $SLURM_NTASKS processes = 21 configurations)"
echo "   Collecting ~100+ individual measurements..."
echo ""

time mpirun --oversubscribe -np $SLURM_NTASKS ./bench_mpi_ultra 2>&1 | tee ultra_benchmark_results/mpi_ultra.log

if [ -f bench_mpi_results_ultracomprehensive.csv ]; then
    echo "✓ MPI results: $(wc -l < bench_mpi_results_ultracomprehensive.csv) data points"
    cp bench_mpi_results_ultracomprehensive.csv ultra_benchmark_results/
else
    echo "❌ MPI results file not found!"
fi

echo ""
echo "═══════════════════════════════════════════════════════════════════════"
echo "ANALYSIS & VISUALIZATION PHASE"
echo "═══════════════════════════════════════════════════════════════════════"
echo ""

# Check if Python is available
if command -v python3 &> /dev/null; then
    echo "Generating comprehensive visualizations..."
    python3 comprehensive_analysis.py 2>&1 | tee ultra_benchmark_results/analysis.log
    
    if [ $? -eq 0 ]; then
        echo "✓ Analysis complete"
        echo ""
        echo "Generated figures:"
        ls -lh figures/*.png | awk '{print "  " $9 " (" $5 ")"}'
    else
        echo "⚠ Analysis script had issues (see log)"
    fi
else
    echo "⚠ Python3 not available, skipping visualization"
fi

echo ""
echo "═══════════════════════════════════════════════════════════════════════"
echo "DATA SUMMARY"
echo "═══════════════════════════════════════════════════════════════════════"
echo ""

echo "CSV Files Generated:"
ls -lh bench_*_results_ultracomprehensive.csv 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'

echo ""
echo "Analysis Figures:"
ls -lh figures/A*.png figures/B*.png figures/C*.png 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'

echo ""
echo "╔════════════════════════════════════════════════════════════════════╗"
echo "║                    ✅ BENCHMARK COMPLETE                          ║"
echo "╚════════════════════════════════════════════════════════════════════╝"
echo ""
echo "📊 Data Files:"
echo "   OpenMP: bench_openmp_results_ultracomprehensive.csv"
echo "   MPI:    bench_mpi_results_ultracomprehensive.csv"
echo ""
echo "🎨 Visualization Files:"
echo "   A1: OpenMP speedup and scaling (figures/A1_*.png)"
echo "   A2: OpenMP strong/weak scaling (figures/A2_*.png)"
echo "   B1: MPI communication analysis (figures/B1_*.png)"
echo "   B2: MPI efficiency analysis (figures/B2_*.png)"
echo "   C:  Summary and insights (figures/C_*.png)"
echo ""
echo "📁 Backup Location: ultra_benchmark_results/"
echo "   - All CSV files"
echo "   - Compilation logs"
echo "   - Benchmark logs"
echo "   - Analysis logs"
echo ""
echo "⏱ Completion time: $(date)"
echo ""
echo "Next steps:"
echo "  1. Review the CSV files for detailed data"
echo "  2. Check the generated figures in figures/"
echo "  3. Use data to answer benchmark questions with real numbers!"
echo ""

exit 0
