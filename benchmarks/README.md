# Benchmarking Guide for LINMA2710 Project

This directory contains comprehensive benchmarking and profiling tools for the LINMA2710 project, designed to gather performance data and generate presentation-quality figures.

## Quick Start on Cluster

### Interactive Session (Quick Testing)
```bash
# Connect to cluster
ssh manneback

# Request GPU allocation
salloc --partition=gpu --gres=gpu:1 --time=2:00:00 --mem-per-cpu=2000

# Load modules
module load CUDA
module load Python

# Go to benchmarks directory
cd /path/to/LINMA2710/benchmarks

# Generate profiling setup (one-time)
python3 generate_profiling_setup.py

# Build and run all benchmarks
bash run_all_benchmarks.sh
```

### Batch Job Submission (Recommended)
```bash
# Submit to queue
sbatch submit_manneback.slurm

# Monitor progress
squeue -u $USER
tail -f benchmark_*.log

# Check results when done
ls -lh figures/
```

---

## Benchmark Programs

### 1. **bench_openmp** - Part 1 & 2 (OpenMP Parallelism)

**What it tests:**
- Sequential matrix multiplication (1 thread)
- Parallel matrix multiplication (2-16 threads)
- Multiple matrix sizes (100 to 2000)

**Key metrics:**
- Execution time (ms)
- GFLOPs (Giga Floating Point Operations Per Second)
- Speedup vs single thread

**Output:** `bench_openmp_results.csv`

**Run:**
```bash
./bench_openmp
# or
make run_openmp
```

---

### 2. **bench_mpi** - Part 3 (Distributed Computing)

**What it tests:**
- Distributed matrix multiplication via MPI
- Communication overhead measurement
- `multiplyTransposed` operation

**Key metrics:**
- Total time, compute time, communication time (seconds)
- Communication overhead percentage
- GFLOPs

**Output:** `bench_mpi_results.csv`

**Run:**
```bash
mpirun -np 4 ./bench_mpi
# or
make run_mpi
```

**Note:** Adjust `-np 4` to match available processes.

---

### 3. **bench_opencl** - Part 4 (GPU Computing)

**What it tests:**
- Naive OpenCL kernel (simple matrix multiplication)
- Optimized OpenCL kernel (with local memory, tiling)
- GPU device capabilities

**Key metrics:**
- Kernel execution time (ms)
- GFLOPs
- Speedup of optimized vs naive

**Output:** `bench_opencl_results.csv`

**Run:**
```bash
./bench_opencl
# or
make run_opencl
```

**Note:** Automatically selects GPU if available, falls back to CPU.

---

## Building

### Build all benchmarks:
```bash
make all
```

### Build individual benchmarks:
```bash
make bench_openmp
make bench_mpi
make bench_opencl
```

### Clean up:
```bash
make clean
```

---

## Running and Visualization

### Run all benchmarks and generate plots:
```bash
bash run_all_benchmarks.sh
```

This will:
1. Build all benchmarks
2. Run OpenMP benchmark
3. Run MPI benchmark (4 processes)
4. Run OpenCL benchmark
5. Generate plots automatically

### Generate plots from existing results:
```bash
python3 plot_results.py
```

### Output figures:
All plots are saved to `figures/` directory:
- `01_openmp_speedup.png` - OpenMP scaling analysis
- `02_mpi_communication.png` - MPI communication breakdown
- `03_opencl_kernels.png` - GPU kernel comparison
- `04_gflops_comparison.png` - Cross-paradigm performance
- `05_summary_table.png` - Key metrics summary

---

## Advanced Profiling

### Generate profiling setup:
```bash
python3 generate_profiling_setup.py
```

This creates:
- `PROFILING_GUIDE.md` - Detailed profiling instructions
- `submit_manneback.slurm` - Manneback submission script
- `submit_lyra.slurm` - Lyra submission script

### Profile OpenMP with Linux perf:
```bash
# Build with debug symbols
g++ -std=c++17 -O3 -g -fopenmp -I../project/include \
    -o bench_openmp_perf bench_openmp.cpp ../project/src/matrix.cpp

# Record profile
perf record -g ./bench_openmp_perf

# View report
perf report
```

### Profile MPI:
```bash
# Run with timing info
mpirun -np 4 ./bench_mpi | tee mpi_output.txt

# Check communication breakdown in output
grep -E "Total|Compute|Comm|%"  mpi_output.txt
```

### Profile OpenCL GPU (NVIDIA):
```bash
# Simple GPU query
nvidia-smi

# Monitor during execution
# Terminal 1:
nvidia-smi dmon

# Terminal 2:
./bench_opencl
```

---

## Understanding the Results

### OpenMP (Part 1 & 2)

**Expected behavior:**
- Speedup increases with threads (up to a limit)
- Small matrices: overhead dominates, little/no speedup
- Large matrices: good speedup until memory bandwidth limit
- Peak at ~75% of number of cores (due to hyperthreading costs)

**Presentation talking points:**
- Show speedup curve and identify break-even point
- Discuss Amdahl's law and why speedup plateaus
- Compare with theoretical maximum (# threads)

### MPI (Part 3)

**Expected behavior:**
- Communication time grows slower than computation for large matrices
- For N=1000: communication might be 20-40% of total time
- For N=2000: communication might be 10-20% of total time
- Speedup depends on local computation time vs network latency

**Presentation talking points:**
- Show communication breakdown chart
- Explain column partitioning strategy
- Discuss MPI_Allreduce cost in multiplyTransposed
- Compare actual speedup with ideal (N_proc × speedup)

### OpenCL (Part 4)

**Expected behavior:**
- Optimized kernel 2-4× faster than naive (depends on GPU)
- GFLOPs depends heavily on GPU memory bandwidth
- Data transfer overhead significant for small matrices
- Larger matrices amortize transfer cost

**Presentation talking points:**
- Show two kernel implementations side-by-side
- Explain optimization technique (local memory, tiling, work groups)
- Compare GPU performance to CPU (different paradigms!)
- Discuss when GPU is worthwhile (data size, launch overhead)

---

## Presentation Tips

### What to highlight in slides:

1. **Part 1 & 2 (OpenMP)**
   - Speedup graph with ideal line
   - Identify where parallelization overhead kicks in
   - Peak performance metric

2. **Part 3 (MPI)**
   - Communication vs computation breakdown
   - Scalability analysis
   - Distributed algorithm explanation

3. **Part 4 (OpenCL)**
   - Naive vs Optimized kernel code snippets
   - Performance improvement (speedup)
   - GPU device information

4. **Cross-Paradigm Comparison**
   - GFLOPs chart (all three paradigms)
   - Trade-offs: simplicity vs performance vs scalability
   - When to use each approach

### Important metrics to memorize:

- OpenMP: **Speedup at max threads** (e.g., "7× on 8 threads")
- MPI: **Communication overhead %** (e.g., "15% communication for 2000×2000 matrix")
- OpenCL: **Speedup of optimized kernel** (e.g., "3× faster than naive")

---

## Troubleshooting

### "No OpenCL devices found"
- Ensure CUDA module is loaded: `module load CUDA`
- Check GPU availability: `nvidia-smi`
- Some systems may only have CPU support

### "MPI not working"
- Ensure OpenMPI or MPICH is loaded
- Check: `which mpic++`
- For 4 processes, ensure 4 cores are available

### "Python plotting fails"
- Install matplotlib: `pip install matplotlib pandas`
- Or: `module load Python` then `pip3 install --user matplotlib pandas`

### Results look wrong
- Check that matrix sizes are reasonable (200-2000)
- Ensure benchmark ran long enough (should take 1-2 minutes total)
- Look at CSV files directly: `cat bench_openmp_results.csv`

---

## Important Notes for Cluster Usage

1. **Time limits:** Benchmarks should complete in <1 hour total
2. **Memory:** 2000 MB per CPU should be sufficient
3. **GPU allocation:** Always use `--gres=gpu:1` when requesting GPUs
4. **Module loading:** Always load CUDA before running GPU code
5. **Output files:** All CSV and PNG files stay in the benchmarks directory

---

## File Organization

```
benchmarks/
├── bench_openmp.cpp              # OpenMP benchmark source
├── bench_mpi.cpp                 # MPI benchmark source
├── bench_opencl.cpp              # OpenCL benchmark source
├── plot_results.py               # Plot generation script
├── generate_profiling_setup.py   # Profiling setup generator
├── Makefile                      # Build configuration
├── run_all_benchmarks.sh         # Main benchmark runner
├── submit_benchmarks.sh          # Cluster submission script
├── submit_manneback.slurm        # Manneback SLURM script
├── submit_lyra.slurm             # Lyra SLURM script
│
├── bench_openmp_results.csv      # OpenMP results (generated)
├── bench_mpi_results.csv         # MPI results (generated)
├── bench_opencl_results.csv      # OpenCL results (generated)
│
└── figures/                      # Generated plots
    ├── 01_openmp_speedup.png
    ├── 02_mpi_communication.png
    ├── 03_opencl_kernels.png
    ├── 04_gflops_comparison.png
    └── 05_summary_table.png
```

---

## Questions?

Refer to `PROFILING_GUIDE.md` for detailed profiling instructions or contact the teaching assistants:
- Benoit Loucheur: benoit.loucheur@uclouvain.be
- Antonin Oswald: antonin.oswald@uclouvain.be
- Amir Bayat: amir.bayat@uclouvain.be
