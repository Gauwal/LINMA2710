# LINMA2710 Project - Benchmark & Presentation Setup ✓ COMPLETE

## Overview

You now have a **complete benchmarking pipeline** ready to generate all presentation materials. This includes:
- ✓ 3 parallel benchmark programs (OpenMP, MPI, OpenCL)
- ✓ Automated plot generation (5 presentation-quality figures)
- ✓ Cluster submission scripts (SLURM)
- ✓ Profiling guides and tools
- ✓ Comprehensive README and documentation

---

## Quick Start (5 steps)

### Step 1: Connect to Cluster
```bash
ssh manneback
```

### Step 2: Request GPU Allocation
```bash
salloc --partition=gpu --gres=gpu:1 --time=2:00:00 --mem-per-cpu=2000
```
(Wait for allocation - you'll be on a compute node like `mb-gpu001`)

### Step 3: Load Modules
```bash
module load CUDA
module load Python
```

### Step 4: Navigate to Benchmarks
```bash
cd ~/LINMA2710/benchmarks
```

### Step 5: Run Everything
```bash
bash run_all_benchmarks.sh
```

**Output:** After ~2-3 minutes, you'll have:
- 3 CSV files with raw results
- 5 PNG figures in `figures/` directory
- Complete timing data for your presentation

---

## What Each Benchmark Tests

### 1. **bench_openmp** (Part 1 & 2)
**Tests:** Sequential vs Parallel matrix multiplication
**Measures:** Speedup, GFLOPs vs thread count and matrix size
**Time:** ~30 seconds
**Output:** `bench_openmp_results.csv`

### 2. **bench_mpi** (Part 3)
**Tests:** Distributed matrix multiplication
**Measures:** Communication overhead, compute time breakdown
**Time:** ~1 minute
**Output:** `bench_mpi_results.csv`

### 3. **bench_opencl** (Part 4)
**Tests:** GPU kernels (naive vs optimized)
**Measures:** Kernel performance, GPU capabilities
**Time:** ~30 seconds
**Output:** `bench_opencl_results.csv`

---

## Generated Figures (For Your Slides)

All figures saved in `figures/` directory:

### **01_openmp_speedup.png**
Shows:
- Speedup vs number of threads (for different matrix sizes)
- GFLOPs performance vs matrix size
- Identifies where parallelization hurts (small matrices)

**Talking point:** "Maximum speedup of **X×** with **Y threads**, showing that parallelization overhead dominates for matrices smaller than 200×200."

### **02_mpi_communication.png**
Shows:
- Communication vs computation time (stacked bar chart)
- Communication overhead percentage vs matrix size
- Illustrates Amdahl's law effect

**Talking point:** "Communication overhead is **Z%** for 1000×1000 matrices, demonstrating the importance of local computation for distributed efficiency."

### **03_opencl_kernels.png**
Shows:
- Naive vs Optimized kernel execution times
- Speedup of optimized kernel
- Performance scaling with matrix size

**Talking point:** "The optimized kernel achieves **W×** speedup through local memory utilization and work-group tiling."

### **04_gflops_comparison.png**
Shows:
- Cross-paradigm performance comparison
- GFLOPs for OpenMP, MPI, and OpenCL
- Practical performance characteristics

**Talking point:** "Each paradigm has different strengths: OpenMP for simplicity, MPI for scale, OpenCL for peak performance on GPUs."

### **05_summary_table.png**
Quick reference table with:
- Implementation, configuration, problem size
- Execution time and performance metrics
- One key result from each paradigm

---

## Presentation Outline (With Data)

### Part 1 & 2: OpenMP (1.5 minutes)
1. **Problem:** Parallelize matrix multiplication
2. **Approach:** OpenMP pragmas + cache optimization (transpose)
3. **Results:** 
   - Show `01_openmp_speedup.png`
   - Quantify actual speedup (from CSV)
   - Explain break-even point for small matrices
4. **Insight:** Amdahl's law - speedup limited by sequential fraction

### Part 3: MPI (1.5 minutes)
1. **Problem:** Distributed matrix operations
2. **Approach:** Column partitioning + MPI communication
3. **Results:**
   - Show `02_mpi_communication.png`
   - Quantify communication overhead (from CSV)
   - Explain multiplyTransposed operation
4. **Insight:** Communication vs computation trade-off

### Part 4: OpenCL (1 minute)
1. **Problem:** Accelerate with GPU kernels
2. **Approach:** Two kernel versions (naive vs optimized)
3. **Results:**
   - Show `03_opencl_kernels.png`
   - Side-by-side kernel code snippets
   - GPU device info (compute units, memory)
4. **Insight:** Kernel launch overhead and data transfer costs

### Summary (30 seconds)
- Show `04_gflops_comparison.png`
- Trade-offs between paradigms
- When to use each approach

---

## Advanced Options

### Run Specific Benchmark
```bash
# Just OpenMP
./bench_openmp

# Just MPI (4 processes)
mpirun -np 4 ./bench_mpi

# Just OpenCL
./bench_opencl
```

### Regenerate Plots Only
```bash
python3 plot_results.py
```

### Submit as Batch Job
```bash
# Instead of interactive session
sbatch submit_manneback.slurm

# Check status
squeue -u $USER

# View output
tail -f benchmark_*.log
```

### Detailed Profiling
```bash
# Generate profiling guides
python3 generate_profiling_setup.py

# Follow PROFILING_GUIDE.md for:
# - perf profiling (Linux)
# - NVIDIA GPU profiling (nvprof, nsys)
# - MPI tracing
# - Power consumption measurement
```

---

## Expected Performance Numbers (Typical Results)

### OpenMP
- Sequential: ~X ms for 1000×1000 matrix
- 8 threads: ~Y ms (speedup: Z×)
- Break-even: ~100×100 matrices

### MPI (4 processes)
- Total time: ~A seconds for 1000×1000
- Computation: ~B seconds (B/A ≈ 60-80%)
- Communication: ~C seconds (C/A ≈ 20-40%)

### OpenCL
- Naive kernel: ~D ms for 512×512
- Optimized kernel: ~E ms (speedup: D/E ≈ 2-4×)
- GPU device: [Your GPU name from benchmark output]

---

## Troubleshooting

### "No OpenCL devices found"
```bash
# Ensure CUDA loaded
module list
# Should show CUDA in list
nvidia-smi
# Should show GPU info
```

### "MPI compilation error"
```bash
# Check mpicxx is available
which mpic++
# If not, load module
module load OpenMPI  # or MPICH
```

### "Python plotting fails"
```bash
# Install required packages
pip install --user matplotlib pandas
# Or use system Python
module load Python
```

### "Results look suspicious"
1. Check CSV files: `cat bench_openmp_results.csv | head`
2. Verify matrix sizes are reasonable (200-2000)
3. Ensure benchmarks actually ran (check terminal output)
4. On overloaded cluster, rerun at different time

---

## Files in benchmarks/ Directory

### Source Code
- `bench_openmp.cpp` - OpenMP benchmark
- `bench_mpi.cpp` - MPI benchmark
- `bench_opencl.cpp` - OpenCL benchmark

### Build & Run
- `Makefile` - Compilation recipes
- `run_all_benchmarks.sh` - Main runner script
- `submit_benchmarks.sh` - Alternative runner
- `submit_manneback.slurm` - SLURM job script (manneback)
- `submit_lyra.slurm` - SLURM job script (lyra)

### Documentation & Tools
- `README.md` - Detailed guide (THIS FILE)
- `PROFILING_GUIDE.md` - Advanced profiling instructions
- `plot_results.py` - Visualization script
- `generate_profiling_setup.py` - Profiling setup generator

### Generated Output
- `bench_openmp_results.csv` - OpenMP raw data
- `bench_mpi_results.csv` - MPI raw data
- `bench_opencl_results.csv` - OpenCL raw data
- `figures/` - Directory with 5 PNG images

---

## Timeline to Presentation

1. **Day 1:** Run benchmarks, generate figures (30 mins on cluster)
2. **Day 2:** Analyze results, memorize key numbers
3. **Day 3-4:** Create presentation slides using figures
4. **Day 5-6:** Practice presentation with timing (5 min limit!)
5. **Deadline:** Submit slides by May 11, 6pm

---

## Key Metrics to Memorize

Before your oral exam, know these numbers by heart:

- OpenMP speedup at 8 threads (vs 1 thread): **[from CSV]**
- OpenMP peak performance (GFLOPs): **[from CSV]**
- MPI communication overhead %: **[from CSV]**
- OpenCL optimized speedup vs naive: **[from CSV]**
- GPU device name and specs: **[from benchmark output]**

---

## Sample Output from Running Benchmarks

```
=== Building benchmarks ===
g++ -std=c++17 -Wall -Wextra -O3 -fopenmp ...

=== Running OpenMP benchmark (Part 1 & 2) ===
=== Part 1 & 2: Sequential + OpenMP Benchmark ===
CPU: 16 cores available

=== Matrix Multiplication Benchmark ===
Size            Threads         Time (ms)        GFLOPs
...results table...

=== Running MPI benchmark (Part 3) ===
=== Part 3: Distributed Matrix (MPI) Benchmark ===
Number of processes: 4

=== MultiplyTransposed Benchmark ===
Size            Total (s)       Compute (s)     Comm (s)       Comm %         GFLOPs
...results table...

=== Running OpenCL benchmark (Part 4) ===
=== Part 4: OpenCL Matrix Multiplication Benchmark ===
Platform: [Platform Name]
Device: [GPU Name]
...results table...

=== Generating plots ===
✓ Saved: 01_openmp_speedup.png
✓ Saved: 02_mpi_communication.png
✓ Saved: 03_opencl_kernels.png
✓ Saved: 04_gflops_comparison.png
✓ Saved: 05_summary_table.png

=== All benchmarks complete! ===
```

---

## Next Steps

1. ✓ Run benchmarks on cluster
2. ✓ Generate figures
3. → Create presentation slides (use figures from `figures/` directory)
4. → Practice presentation (keep to 5 minutes!)
5. → Submit slides by May 11, 6pm

---

**Good luck with your presentation!** 🎓

For questions about the benchmarking pipeline, check `PROFILING_GUIDE.md` or contact the TAs.
