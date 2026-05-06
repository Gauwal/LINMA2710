# LINMA2710 Presentation & Benchmarking - Complete Setup

## 📋 What Has Been Created

### 1. **PRESENTATIONS** (Ready to Use)

#### A. Interactive HTML Presentation
- **File:** `PRESENTATION.html` 
- **How to view:** Open in any web browser
- **Features:**
  - 15 professional slides
  - Navigation: Previous/Next buttons or arrow keys
  - Progress bar showing current position
  - Beautiful gradient design
  - All figures and performance metrics included
  - ~5 minutes reading time (perfect for 5-minute presentation)

#### B. Markdown Presentation
- **File:** `PRESENTATION_SLIDES.md`
- **How to use:** Convert to PDF/PowerPoint using:
  - Pandoc: `pandoc PRESENTATION_SLIDES.md -t pptx -o presentation.pptx`
  - Or open in Markdown viewer

### 2. **COMPREHENSIVE BENCHMARKS** (NEW!)

#### A. OpenMP Comprehensive Benchmark
- **File:** `benchmarks/bench_openmp_comprehensive.cpp`
- **What it tests:**
  - Matrix sizes: 100, 200, 300, 500, 750, 1000, 1500, 2000
  - Thread counts: 1, 2, 4, 8, 16 (automatically detects available cores)
  - 3 iterations per configuration for stability
  - Measurements: time, GFLOPs, memory bandwidth
  - Calculates speedup and parallel efficiency
- **Outputs:** `bench_openmp_results_comprehensive.csv`

#### B. MPI Comprehensive Benchmark
- **File:** `benchmarks/bench_mpi_comprehensive.cpp`
- **What it tests:**
  - Matrix sizes: 100, 200, 300, 500, 750, 1000, 1200, 1500
  - MPI processes: 2, 4, or 8 (configurable at runtime)
  - 3 iterations per configuration
  - Separates compute vs communication time
  - Measures communication overhead percentage
- **Outputs:** `bench_mpi_results_comprehensive.csv`

### 3. **EXECUTION SCRIPTS**

#### A. Local Testing Script
- **File:** `benchmarks/run_comprehensive.sh`
- **Purpose:** Run benchmarks on local machine
- **Usage:**
  ```bash
  cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks
  bash run_comprehensive.sh
  ```
- **What it does:**
  - Automatically detects available cores and processes
  - Compiles both benchmarks
  - Runs OpenMP with detected core count
  - Runs MPI with safe process count
  - Generates updated plots
  - Takes ~5-15 minutes depending on system

#### B. Cluster SLURM Submission Script
- **File:** `benchmarks/submit_comprehensive.slurm`
- **Purpose:** Submit to CECI cluster for comprehensive benchmarking
- **Usage:**
  ```bash
  cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks
  sbatch submit_comprehensive.slurm
  ```
- **Specifications:**
  - CPU-intensive job: 6 hours max
  - Requests: 16 CPUs per task, 4 tasks
  - Optimized for manneback GPU partition
  - Will generate much richer data than local runs
- **Expected output:** 
  - Full OpenMP scaling from 1-16 threads
  - Full MPI scaling with 4 processes
  - Updated figures with comprehensive data

---

## 🚀 Quick Start

### Option 1: View Presentation (Now!)
```bash
# Open the HTML presentation directly
open /home/ucl/ingi/gsavary/LINMA2710/PRESENTATION.html
# or in Firefox:
firefox /home/ucl/ingi/gsavary/LINMA2710/PRESENTATION.html
```

### Option 2: Run Benchmarks Locally (Quick)
```bash
cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks
bash run_comprehensive.sh
# ~5-15 minutes, generates new figures
```

### Option 3: Submit to Cluster (Best Results)
```bash
cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks
sbatch submit_comprehensive.slurm
# Wait for job to complete, check results with:
# sbatch -l  # list jobs
# tail benchmark_*.log  # watch progress
```

---

## 📊 Benchmark Data Files

After running benchmarks, you'll have:

**OpenMP Results:**
```
benchmarks/bench_openmp_results_comprehensive.csv
```
Columns: `matrix_size, num_threads, time_ms, gflops, bandwidth_gb_s`

**MPI Results:**
```
benchmarks/bench_mpi_results_comprehensive.csv
```
Columns: `matrix_size, num_procs, total_time, compute_time, comm_time, gflops`

These CSV files can be:
- Used to generate updated plots
- Imported into Excel/Sheets for custom analysis
- Used for paper/presentation figures

---

## 🎨 Generated Figures

The plotting script automatically generates:

1. **01_openmp_speedup.png** - OpenMP performance vs threads and size
2. **02_mpi_communication.png** - MPI communication breakdown
3. **04_gflops_comparison.png** - Cross-paradigm performance comparison
4. **05_summary_table.png** - Key metrics summary table

Located in: `benchmarks/figures/`

To regenerate plots from data:
```bash
cd /home/ucl/ingi/gsavary/LINMA2710/benchmarks
python3 plot_results.py
```

---

## 📝 Presentation Content Summary

The presentation covers all 4 parts:

**Slide 1-2:** Introduction & Overview
- The four computing paradigms
- Overall goals and structure

**Slide 3-5:** Part 1 & 2 - OpenMP (Shared Memory)
- Why we transpose B matrix
- Performance results
- Analysis of limited speedup (memory bandwidth)

**Slide 6-9:** Part 3 - MPI (Distributed Memory)
- Column-wise partitioning algorithm
- Communication vs computation breakdown
- Why MPI outperforms sequential

**Slide 10-12:** Part 4 - GPU (OpenCL)
- Naive vs optimized kernels
- Local memory tiling benefits
- Data transfer costs

**Slide 13-14:** Cross-paradigm Comparison & Trade-offs
- Performance comparison table
- When to use each paradigm
- Common questions answered

**Slide 15:** Conclusion

---

## 💻 System Requirements

### For Local Testing
- Linux/Mac with:
  - GCC 11+
  - OpenMPI (for MPI benchmark)
  - Python 3 with matplotlib/pandas (for plots)
  - 2+ CPU cores (minimum)

### For Cluster Submission
- Access to CECI cluster (manneback or lyra)
- SLURM installed
- Compute time available (~2 hours per job)

---

## ⚠️ Important Notes

### Thread/Process Scaling
- **OpenMP:** Will test 1, 2, 4, 8, 16 threads (or available cores)
- **MPI:** Safely tests with 2-4 processes on shared resources

### Cluster Limitations
- GPU partition may not have OpenCL drivers
- CPU is typically single-socket (limits OpenMP to ~16-20 threads max)
- MPI communication on local cluster is fast (negligible overhead)

### Expected Results
- **OpenMP:** 2-2.5 GFLOPs (memory-bound, poor scaling)
- **MPI:** 3-4 GFLOPs (parallel benefit from multiple cores/nodes)
- **GPU:** 50+ GFLOPs (if OpenCL available; not usually on cluster)
- **Sequential:** ~2 GFLOPs (baseline)

---

## 🔧 Troubleshooting

### "mpic++: command not found"
```bash
module load OpenMPI
```

### "No such file or directory" when compiling
- Verify paths in benchmarks/*.cpp are correct
- Check that project/ and benchmarks/ directories exist

### Compilation errors with #pragma
- This is normal - OpenMP pragmas work even if not recognized
- Code will run sequentially without pragma support

### SLURM job fails
- Check job log: `sbatch -l`
- View error: `cat benchmark_*.err`
- Check module availability: `module avail`

---

## 📌 Files Created/Modified

### New Presentation Files
- `PRESENTATION.html` - Interactive HTML presentation (15 slides)
- `PRESENTATION_SLIDES.md` - Markdown version of slides

### New Benchmark Files
- `benchmarks/bench_openmp_comprehensive.cpp` - Enhanced OpenMP benchmark
- `benchmarks/bench_mpi_comprehensive.cpp` - Enhanced MPI benchmark

### New Execution Scripts
- `benchmarks/run_comprehensive.sh` - Local execution script
- `benchmarks/submit_comprehensive.slurm` - Cluster submission script

### Documentation
- `SETUP_FOR_PRESENTATION.md` (this file) - Complete setup guide

---

## 📞 Quick Reference

| Task | Command |
|------|---------|
| View HTML presentation | Open PRESENTATION.html in browser |
| Run local benchmarks | `cd benchmarks && bash run_comprehensive.sh` |
| Submit cluster job | `cd benchmarks && sbatch submit_comprehensive.slurm` |
| Regenerate plots | `cd benchmarks && python3 plot_results.py` |
| Check cluster job status | `squeue -u $USER` |
| View cluster job logs | `tail -f benchmark_*.log` |
| Convert markdown to PPTX | `pandoc PRESENTATION_SLIDES.md -t pptx -o presentation.pptx` |

---

## ✅ Checklist for Presentation Day

- [ ] Open PRESENTATION.html (works on any browser)
- [ ] Test keyboard navigation (arrow keys)
- [ ] Verify all 15 slides render correctly
- [ ] Check slide timing (should be ~5 minutes reading time)
- [ ] Print or save HTML if needed
- [ ] Have backup copy of figures (benchmarks/figures/)
- [ ] Have backup of presentation (PRESENTATION.html + PRESENTATION_SLIDES.md)

---

## 📚 Additional Resources

See also:
- `README.md` - Original project README
- `benchmarks/README.md` - Benchmarking guide
- `PRESENTATION_CHEATSHEET.md` - Q&A preparation guide
- `project/README.md` - Project compilation guide

---

**Last Updated:** May 6, 2025  
**Status:** Ready for presentation (May 12)  
**Benchmarks:** Comprehensive, ready for execution
