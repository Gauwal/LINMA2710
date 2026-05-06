
# LINMA2710 Project - Profiling Guide

## Cluster Setup

### Step 1: Connect to cluster
```bash
ssh manneback
```

### Step 2: Request GPU allocation (INTERACTIVE SESSION)
```bash
# Manneback
salloc --partition=gpu --gres=gpu:1 --time=2:00:00 --mem-per-cpu=2000

# Lyra
salloc --partition=batch --gres=gpu:1 --time=2:00:00 --mem-per-cpu=2000
```

### Step 3: Load modules
```bash
module load CUDA
module load Python  # for plotting
```

### Step 4: Run benchmarks
```bash
cd /path/to/LINMA2710/benchmarks
bash run_all_benchmarks.sh
```

---

## Part 1 & 2: OpenMP Profiling

### Option A: Use `perf` (Linux Performance Tool)
```bash
cd /path/to/LINMA2710/benchmarks
# Build with debug symbols and optimizations
g++ -std=c++17 -O3 -g -fopenmp -I../project/include \
    -o bench_openmp_perf bench_openmp.cpp ../project/src/matrix.cpp

# Run with perf
perf record -g ./bench_openmp_perf
perf report
```

### Option B: Use Linux profilers
```bash
# vtune (if available)
vtune -collect hotspots -result-dir r1 ./bench_openmp

# Callgrind
valgrind --tool=callgrind ./bench_openmp
kcachegrind callgrind.out.<pid>
```

### Option C: Simple timing with environment variables
```bash
# Set thread count explicitly
export OMP_NUM_THREADS=8
export OMP_PROC_BIND=true
./bench_openmp
```

---

## Part 3: MPI Profiling

### Option A: Use MPI built-in profiling
```bash
# Run with timing
MPICH_TIMING=1 mpirun -np 4 ./bench_mpi

# Run with tracing (generates trace files)
mpirun -np 4 ./bench_mpi > mpi_output.txt 2>&1
```

### Option B: Score-P (if available on cluster)
```bash
# Compile with Score-P
scorep-mpic++ -O3 -I../project/include -o bench_mpi_sp \
    bench_mpi.cpp ../project/src/distributed_matrix.cpp ../project/src/matrix.cpp

# Run
mpirun -np 4 ./bench_mpi_sp
```

### Option C: Manual instrumentation (already in code)
The benchmark code includes `MPI_Wtime()` for timing. Review the output carefully:
- Communication time should be reasonable (< 50% for large matrices)
- If > 70%, consider optimizing communication pattern

---

## Part 4: OpenCL GPU Profiling

### Option A: NVIDIA profiling (on nvidia GPUs)
```bash
# Check GPU
nvidia-smi

# Profile with nvprof (older systems)
nvprof ./bench_opencl

# Profile with nsys (newer systems)
nsys profile -o bench_opencl_profile ./bench_opencl
nsys-ui bench_opencl_profile.nsys-rep  # View in GUI

# Profile with Nsight Compute (detailed analysis)
ncu -o bench_opencl_detailed ./bench_opencl
```

### Option B: OpenCL command-line profiling
Enable profiling in queue: CL_QUEUE_PROFILING_ENABLE
(Already enabled in benchmark code)

### Option C: AMD GPU Profiling (on AMD GPUs)
```bash
# rocprof
rocprof --hsa-trace ./bench_opencl
rocprof --stats ./bench_opencl
```

---

## Part 4: Power Consumption Analysis (Optional but Impressive)

### Option A: Use nvidia-smi for power
```bash
# Monitor GPU power in real-time
nvidia-smi dmon

# In another terminal, run benchmark
./bench_opencl
```

### Option B: Use `codecarbon` package
```bash
pip install codecarbon

# Create a wrapper script:
# python benchmark_with_carbon.py
```

### Option C: Measure CPU power
```bash
# On systems with perf and energy counters
perf stat -e power/energy-cores/ ./bench_openmp
```

---

## Summary of Key Metrics to Collect

### For Presentation Slides:

1. **OpenMP (Part 1&2)**
   - [ ] Speedup vs threads (for sizes 500, 1000, 2000)
   - [ ] Strong scaling curve
   - [ ] Break-even point (where parallelization hurts)
   - [ ] GFLOPs peak performance

2. **MPI (Part 3)**
   - [ ] Communication breakdown (% time in MPI ops)
   - [ ] Speedup vs processes
   - [ ] Communication overhead vs matrix size
   - [ ] Amdahl's law prediction vs reality

3. **OpenCL (Part 4)**
   - [ ] Naive vs Optimized kernel timing
   - [ ] GFLOPs for both kernels
   - [ ] Kernel launch overhead measurement
   - [ ] Data transfer time vs computation time
   - [ ] Power consumption (if possible)

4. **Cross-Paradigm Comparison**
   - [ ] GFLOPs comparison chart
   - [ ] Speedup vs implementation
   - [ ] Scalability analysis
