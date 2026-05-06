#!/bin/bash

echo "╔════════════════════════════════════════════════════════════════════╗"
echo "║        LINMA2710 ULTRA-COMPREHENSIVE BENCHMARK - LOCAL RUN        ║"
echo "║              Extensive Data Collection & Analysis                  ║"
echo "╚════════════════════════════════════════════════════════════════════╝"
echo ""

cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks

# Create output directories
mkdir -p ultra_benchmark_results
mkdir -p figures

# Load modules if available
module load GCC/13.3.0 2>/dev/null || true
module load OpenMPI 2>/dev/null || true

echo "═══════════════════════════════════════════════════════════════════════"
echo "COMPILATION PHASE"
echo "═══════════════════════════════════════════════════════════════════════"
echo ""

# Compile OpenMP
echo "Compiling OpenMP ultra-comprehensive benchmark..."
if g++ -O3 -fopenmp -std=c++17 -march=native \
    -I/home/ucl/ingi/gsavary/LINMA2710/project/include \
    -o bench_omp_ultra bench_openmp_ultracomprehensive.cpp \
    /home/ucl/ingi/gsavary/LINMA2710/project/src/matrix.cpp \
    2>&1 | tee compile_omp_ultra.log; then
    echo "✓ OpenMP compiled successfully"
else
    echo "❌ OpenMP compilation failed"
    exit 1
fi

# Compile MPI if available
if command -v mpic++ &> /dev/null; then
    echo "Compiling MPI ultra-comprehensive benchmark..."
    if mpic++ -O3 -std=c++17 -march=native \
        -I/home/ucl/ingi/gsavary/LINMA2710/project/include \
        -o bench_mpi_ultra bench_mpi_ultracomprehensive.cpp \
        /home/ucl/ingi/gsavary/LINMA2710/project/src/matrix.cpp \
        /home/ucl/ingi/gsavary/LINMA2710/project/src/distributed_matrix.cpp \
        2>&1 | tee compile_mpi_ultra.log; then
        echo "✓ MPI compiled successfully"
        MPI_AVAILABLE=1
    else
        echo "⚠ MPI compilation failed"
        MPI_AVAILABLE=0
    fi
else
    echo "⚠ MPI compiler not available"
    MPI_AVAILABLE=0
fi

echo ""
echo "═══════════════════════════════════════════════════════════════════════"
echo "BENCHMARKING PHASE"
echo "═══════════════════════════════════════════════════════════════════════"
echo ""

# Get core count
CORES=$(nproc 2>/dev/null || echo 4)
THREADS=$((CORES > 16 ? 16 : CORES))
MPI_PROCS=$((CORES > 4 ? 4 : 2))

echo "System info:"
echo "  Available cores: $CORES"
echo "  Using for OpenMP: $THREADS threads max"
if [ $MPI_AVAILABLE -eq 1 ]; then
    echo "  Using for MPI: $MPI_PROCS processes"
fi
echo ""

# Run OpenMP
echo "🚀 RUNNING: OpenMP Ultra-Comprehensive Benchmark"
echo "   (21 matrix sizes × up to $THREADS threads)"
echo "   This will take 15-30 minutes..."
echo ""

export OMP_NUM_THREADS=$THREADS
time ./bench_omp_ultra 2>&1 | tee ultra_benchmark_results/openmp_ultra.log

if [ -f bench_openmp_results_ultracomprehensive.csv ]; then
    DATA_LINES=$(wc -l < bench_openmp_results_ultracomprehensive.csv)
    echo "✓ OpenMP complete: $DATA_LINES data points collected"
    cp bench_openmp_results_ultracomprehensive.csv ultra_benchmark_results/
    echo ""
else
    echo "❌ OpenMP results file not generated"
fi

# Run MPI
if [ $MPI_AVAILABLE -eq 1 ]; then
    echo "🚀 RUNNING: MPI Ultra-Comprehensive Benchmark"
    echo "   (21 matrix sizes × $MPI_PROCS processes)"
    echo "   This will take 10-20 minutes..."
    echo ""
    
    time mpirun --oversubscribe -np $MPI_PROCS ./bench_mpi_ultra 2>&1 | tee ultra_benchmark_results/mpi_ultra.log
    
    if [ -f bench_mpi_results_ultracomprehensive.csv ]; then
        DATA_LINES=$(wc -l < bench_mpi_results_ultracomprehensive.csv)
        echo "✓ MPI complete: $DATA_LINES data points collected"
        cp bench_mpi_results_ultracomprehensive.csv ultra_benchmark_results/
        echo ""
    else
        echo "❌ MPI results file not generated"
    fi
fi

echo "═══════════════════════════════════════════════════════════════════════"
echo "ANALYSIS & VISUALIZATION PHASE"
echo "═══════════════════════════════════════════════════════════════════════"
echo ""

if command -v python3 &> /dev/null; then
    echo "Generating comprehensive visualizations..."
    echo "   (This creates 5 separate analysis figures)"
    echo ""
    
    if python3 comprehensive_analysis.py 2>&1 | tee ultra_benchmark_results/analysis.log; then
        echo ""
        echo "✓ Analysis complete!"
        echo ""
        echo "Generated figures:"
        ls -lh figures/*.png 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'
    else
        echo "❌ Analysis had errors (see log for details)"
    fi
else
    echo "⚠ Python3 not available, skipping visualization"
fi

echo ""
echo "╔════════════════════════════════════════════════════════════════════╗"
echo "║                     ✅ BENCHMARK COMPLETE                         ║"
echo "╚════════════════════════════════════════════════════════════════════╝"
echo ""
echo "📊 Data Files Generated:"
ls -lh bench_*_results_ultracomprehensive.csv 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'
echo ""
echo "🎨 Visualization Files:"
ls -lh figures/A*.png figures/B*.png figures/C*.png 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'
echo ""
echo "📁 All results backed up in: ultra_benchmark_results/"
echo ""
echo "📈 Data Summary:"
if [ -f bench_openmp_results_ultracomprehensive.csv ]; then
    echo "  OpenMP: $(tail -1 bench_openmp_results_ultracomprehensive.csv)"
fi
if [ -f bench_mpi_results_ultracomprehensive.csv ]; then
    echo "  MPI:    $(tail -1 bench_mpi_results_ultracomprehensive.csv)"
fi
echo ""
echo "✨ Your benchmarks are now ready for analysis and presentation!"
echo ""
