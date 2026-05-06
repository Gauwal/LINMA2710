# LINMA2710 Project - Presentation Quick Reference Card

## 5-Minute Presentation Structure

```
[0:00-0:30] Introduction (Table: CPU sequential → OpenMP → Distributed (MPI) → GPU (OpenCL))
[0:30-2:00] Part 1&2: OpenMP
            - Design choice: Transpose for cache locality + SIMD auto-vectorization
            - Results: Show 01_openmp_speedup.png
            - Key number: "X× speedup on Y threads"
            - Amdahl's law: sequential fraction limits parallelization

[2:00-3:30] Part 3: MPI
            - Column-wise matrix partitioning
            - Results: Show 02_mpi_communication.png
            - Key number: "Z% communication overhead for 1000×1000 matrix"
            - Explain multiplyTransposed and MPI_Allreduce

[3:30-4:30] Part 4: OpenCL
            - Two kernel implementations (naive vs optimized with local memory)
            - Results: Show 03_opencl_kernels.png
            - Key number: "W× speedup with tiling/local memory"
            - GPU device specs from benchmark

[4:30-5:00] Summary & Trade-offs
            - Show 04_gflops_comparison.png
            - Each paradigm has different purpose
            - Conclusion: understanding all 4 is valuable
```

---

## Things to Have Ready

### On Computer
- [ ] Slides (PDF submitted to Moodle)
- [ ] All 5 PNG figures in `figures/` directory
- [ ] CSV files with raw data for Q&A
- [ ] Open one terminal with `ls figures/` visible

### In Your Head
- [ ] OpenMP speedup number
- [ ] MPI communication overhead %
- [ ] OpenCL optimization speedup
- [ ] GPU device name and 2 specs
- [ ] Why each approach was chosen

### Answer Templates for Q&A (min 5 min)

**"Why did you choose to transpose the matrix in multiplication?"**
> "Transpose improves cache locality. Normally we'd read matrix B column-wise (bad cache), but transposing lets us read both sequentially (cache-friendly). This also enables SIMD auto-vectorization."

**"What was the communication overhead in MPI?"**
> "For 1000×1000 matrices, communication was approximately Z% of total time. This is because each process does O(n²) computation but only O(n) data transfer, so larger matrices amortize communication cost."

**"Why is the optimized OpenCL kernel faster?"**
> "The optimized kernel uses local memory (shared cache) and work-group tiling. Instead of repeated global memory access, we load tiles into fast local memory and do all computation there. NVIDIA/AMD GPUs have limited global bandwidth, so local memory is ~10-100× faster."

**"Did you consider other optimizations?"**
> "Yes, we could try: [1] Kernel fusion, [2] pinned memory for MPI, [3] vectorization pragmas, [4] reduced precision (float32), etc. Time constraints limited what we implemented."

**"How does this scale to larger matrices?"**
> "Larger matrices favor GPU and MPI because [1] communication overhead becomes negligible, [2] GPU memory bandwidth is better utilized. Smaller matrices favor simple OpenMP."

**"Why use each paradigm?"**
> "OpenMP: Simple, shared memory, good for small-medium on single machine. MPI: Scales across nodes, but communication cost. OpenCL: Massive parallelism, but data transfer overhead."

---

## Key Numbers To Know

| Paradigm | Configuration | Matrix Size | Key Metric | Value |
|----------|---|---|---|---|
| OpenMP | 8 threads | 1000×1000 | Speedup | **___ x** |
| OpenMP | 8 threads | 1000×1000 | GFLOPs | **___ GFLOP/s** |
| MPI | 4 processes | 1000×1000 | Comm % | **____ %** |
| OpenCL | Optimized | 512×512 | Speedup vs Naive | **___ x** |
| GPU Device | - | - | Name | **________** |
| GPU Device | - | - | Compute Units | **___** |

*(Fill in from your benchmark output)*

---

## Figure Reference

### Figure 1: OpenMP Speedup (`01_openmp_speedup.png`)
- Left panel: Speedup vs threads for different sizes
- Right panel: GFLOPs vs matrix size
- **What to say:** "Speedup increases with threads but plateaus (Amdahl's law). Small matrices don't scale well due to overhead."

### Figure 2: MPI Communication (`02_mpi_communication.png`)
- Left panel: Stacked bar chart (compute vs comm time)
- Right panel: Communication percentage vs size
- **What to say:** "Communication overhead decreases for larger matrices. Even at 1000×1000, it's only Z%, showing that local work is well-balanced."

### Figure 3: OpenCL Kernels (`03_opencl_kernels.png`)
- Left panel: Naive vs Optimized kernel timing
- Right panel: Speedup curve
- **What to say:** "Optimization with local memory and tiling provides W× speedup. Launch overhead becomes less significant for larger matrices."

### Figure 4: Cross-Paradigm (`04_gflops_comparison.png`)
- All three paradigms on same chart
- **What to say:** "Each paradigm excels at different scales. OpenMP good for single-node, MPI for multi-node, OpenCL for GPU acceleration."

### Figure 5: Summary Table (`05_summary_table.png`)
- Key metrics from each paradigm
- **What to say:** "This table summarizes the best performance achieved in each category."

---

## Potential Problem Questions & Answers

**Q: "Did you measure power consumption?"**
> A: "We focused on performance metrics. Power measurement would require additional tools (nvidia-smi dmon, codecarbon), which is in the PROFILING_GUIDE if you're interested."

**Q: "How does weak scaling compare to strong scaling?"**
> A: "We primarily measured strong scaling (fixed problem, increase resources). Weak scaling (increase problem with resources) shows better efficiency for MPI since communication doesn't grow as fast."

**Q: "What's the memory bandwidth limitation?"**
> A: "Modern CPUs: ~50-100 GB/s, GPUs: ~500-900 GB/s. This is why GPUs excel at memory-intensive operations."

**Q: "Why not use other APIs (CUDA, HIP) instead of OpenCL?"**
> A: "OpenCL is more portable (works on AMD, Intel, NVIDIA). CUDA is GPU-only but sometimes faster."

**Q: "What about MPI communication patterns?"**
> A: "We use MPI_Allreduce in multiplyTransposed. Could optimize with MPI_Allgatherv or reduce contention with different tree topology."

---

## Last-Minute Checklist (Before Oral Exam)

- [ ] 5 minute timer set
- [ ] All slides in presentation order
- [ ] One figure from each part on slide
- [ ] Numbers memorized (speedup, comm %, etc.)
- [ ] One sentence explanation for each paradigm choice
- [ ] Answers to 3-4 potential questions rehearsed
- [ ] Examples of code snippets ready (kernel code, pragma, MPI call)
- [ ] Terminal with `ls figures/` accessible for live demo if asked

---

## Presentation Day Timeline

| Time | Action |
|------|--------|
| -15 min | Arrive at exam room, take seat |
| -10 min | Open slides, test projector |
| 0 min | Start timer, begin introduction |
| 5 min | Finish presentation |
| 5-10 min | Questions from evaluators |
| Done | Thank them, ask if anything else |

---

## What Evaluators Want to Hear

✓ You understand your design choices
✓ You have real benchmarking data
✓ You can explain performance bottlenecks
✓ You know the trade-offs between paradigms
✓ You understand the underlying computer science (Amdahl's law, memory hierarchy, etc.)

---

## Emergency Fallback Answers

If you forget something:

**Q: "What was your speedup?"**
> A: "I can see it in Figure 1. Let me point to that graph..."

**Q: "How much communication overhead?"**
> A: "It's shown in Figure 2. The communication percentage decreases as matrix size increases, which makes sense because..."

**Q: "Why did you choose this optimization?"**
> A: "Our goal was to demonstrate the key trade-offs in each paradigm. By implementing both naive and optimized versions, we can show the impact of optimization strategies like local memory usage."

**Q: "What would you do differently?"**
> A: "In retrospect, I could have measured power consumption or done weak-scaling analysis. These are in my PROFILING_GUIDE as possible future work."

---

## Room Setup Tips

- Arrive early to test projector
- Have backup PDF on USB + email to yourself
- Bring a printed copy of figures (emergency fallback)
- Use terminal with big fonts if doing live demo
- Keep water nearby (dry mouth from nervousness!)

---

Good luck! You've got this! 🚀
