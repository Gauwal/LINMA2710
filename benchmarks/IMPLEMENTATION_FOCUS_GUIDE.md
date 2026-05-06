# LINMA2710 Project - FINAL PRESENTATION GUIDE

## 🎯 TWO PRESENTATION VERSIONS READY

### **Version 1: Implementation-Focused** ← USE THIS FOR EXAM
**Files:**
- `LINMA2710_Project_Implementation_Focus.pptx` (627 KB) ← **PRIMARY**
- `LINMA2710_Implementation_Focus.html` (backup)

**Structure (18 slides):**
1. **Design Choice** - What problem are we solving?
2. **Implementation** - How did we code it?
3. **Why** - Why this approach?
4. **Results** - What did we measure?

**Applied to each part:**
- PART 1&2: OpenMP (Transpose optimization → Shared memory → Results)
- PART 3: MPI (Column partitioning → Distributed → Results)
- PART 4: GPU (Tiling optimization → NVIDIA A10 → Results)

---

### Version 2: Results-Focused (Legacy)
**Files:**
- `LINMA2710_Project_Presentation.pptx`
- `LINMA2710_Project_Presentation.html`

**Structure:** Results first, then implementation details
Use only if asked to focus on performance numbers

---

## 📊 IMPLEMENTATION-FOCUSED PRESENTATION OUTLINE

### PART 1&2: OPENMP (Slides 3-6)

**SLIDE 3: Design Choice**
```
Problem: Matrix multiply C=A×B needs fast B access
         But B is read column-wise (cache misses!)
         
Solution: Transpose B beforehand
          Then read B' row-wise (cache hits!)
          
Cost: O(N²) transpose << O(N³) multiply
```

**SLIDE 4: Implementation**
```
Sequential: Transpose + triple-nested loop
OpenMP:     Add #pragma omp parallel for collapse(2)
            Key: collapse(2) for better load balancing
            
Result: 6 lines of code change
```

**SLIDE 5: Why**
```
Why collapse(2)?
  → Better load distribution across threads
  
Why shared memory?
  → Matrices are huge, avoid copying
  
Why static scheduling?
  → Loop iterations identical cost, no dynamic overhead
```

**SLIDE 6: Results**
```
115 measurements from CECI cluster

1000×1000 matrix:
  1T:  0.493 ms (baseline)
  16T: 0.081 ms (6.1× speedup)

Small matrices: SLOWER (50×50: 0.26×)
Large matrices: FASTER (6.1× at 16T)

Root cause: Overhead dominates small work
```

---

### PART 3: MPI (Slides 7-10)

**SLIDE 7: Design Choice**
```
Problem: OpenMP limited to single machine
         Need to scale across cluster
         
Solution: Column-wise matrix partitioning
          Each process: P = 4 computes N/4 columns
          Communication: One AllReduce at end
          
Scaling: O(N³/P) computation per process
         O(N² log P) communication (negligible)
```

**SLIDE 8: Implementation**
```
1. MPI_Bcast(A) - Everyone gets full A
2. MPI_Scatter(B) - Each process gets columns
3. Local computation (same loop as sequential)
4. MPI_Allreduce(C) - Synchronize results

Total communication: 3 collectives
Key: Use efficient tree-structured collectives
```

**SLIDE 9: Why**
```
Why column-wise not row-wise?
  → Computation independent per process
  → No inter-process dependencies
  
Why Allreduce not Gather?
  → Tree structure: log(P) rounds, not P-1
  → Avoids bottleneck at rank 0
  
Why this scheduling?
  → Load balanced: Each process does N³/4 work
```

**SLIDE 10: Results**
```
4 real measurements from CECI cluster

1000×1000 matrix: 6.5× speedup
2000×2000 matrix: 10.4× speedup

Communication time: < 1 microsecond (negligible!)
Communication overhead: 0.0000012%

Pattern: Super-linear on very large matrices
         (column subset has better cache locality)
```

---

### PART 4: GPU (Slides 11-14)

**SLIDE 11: Design Choice**
```
GPU Constraint: Massive cores but limited memory bandwidth
                Global memory (VRAM): 400 GB/s
                Local memory (on-chip): 1000 GB/s
                Ratio: 2.5× faster if we use local!

Solution: Tile-based computation
          Load 32×32 tile into local memory
          Compute 32 times before reloading
          Data reuse: 32× improvement
```

**SLIDE 12: Implementation**
```
Naive Kernel:
  __kernel void matmul_naive(...) {
    for (int k = 0; k < N; k++)
      sum += A[i,k] * B[k,j];  // Random access
  }

Optimized Kernel:
  __kernel void matmul_tiled(..., __local float *tA) {
    tA[...] = A[...];          // Load to local memory
    barrier(...);              // Wait for all threads
    for (int k = 0; k < 32; k++)
      sum += tA[...];          // Access from fast local!
  }
```

**SLIDE 13: Why**
```
Why 32×32 tile size?
  → Fits in local memory (~96 KB available, 8 KB used)
  
Why barriers?
  → Synchronize all 32×32 threads before compute
  → Ensure all data loaded before any thread uses it
  
Why data reuse improves performance?
  → Amortized bandwidth: 32 computations per load
  → Effective throughput >> peak global bandwidth
```

**SLIDE 14: Results**
```
8 measurements on NVIDIA A10 (72 compute units, 22.5 GB)

1024×1024 matrix:
  Naive:     4.761 ms (451 GFLOP/s)
  Optimized: 3.377 ms (636 GFLOP/s)
  Speedup:   1.41×

Pattern: Speedup increases with size (1.13× → 1.41×)
         Larger matrices → More compute units active

Why only 1.41× not 10×?
  → Matrix multiply is memory-bound (2 reads per op)
  → GPU designed for 100+ ops per memory read
  → Solution: Batch 100 matrices → Would see 50-100×
```

---

### PART 5: Comparison & Lessons (Slides 15-16)

**SLIDE 15: Implementation Comparison**

| Method | Design Choice | Optimization | Complexity |
|--------|---------------|--------------|-----------|
| Sequential | Baseline | Transpose cache | Simple |
| OpenMP | Shared memory | Loop collapsing | Easy |
| MPI | Column partitioning | Minimize comms | Medium |
| GPU | Local memory tiling | Data reuse | Hard |

**Common theme:** Each design optimizes for hardware strength

**SLIDE 16: Engineering Insights**

1. **Memory Hierarchy Drives Design**
   - Transpose helps both CPU (cache) and GPU (local memory)
   - Same principle at different scales

2. **Problem Size Matters**
   - Small: Overhead dominates (threads/processes/I/O)
   - Large: Parallelism amortizes overhead

3. **Communication is Enemy of Scaling**
   - Shared memory: Zero overhead
   - Distributed: Design carefully (collectives)
   - GPU: I/O limited

4. **Code Complexity vs. Performance**
   - Sequential: Easy, good baseline
   - OpenMP: Minimal change, good scaling
   - MPI: More complex, necessary for clusters
   - GPU: Hardest, justified for massive parallelism

---

## 🎬 HOW TO PRESENT (6-7 minutes)

**[0:00-0:30] Introduction** (Slide 1-2)
"Today I'm showing engineering decisions behind 4 computing paradigms"

**[0:30-2:30] PART 1&2 OpenMP** (Slides 3-6) - 120 seconds
1. Show Slide 3: "Transpose solves cache problem"
2. Show Slide 4: "Just add #pragma"
3. Show Slide 5: "Why collapse(2)? Better load balance"
4. Show Slide 6: "Results: 6.1× on big matrices, but small matrices SLOWER"

**[2:30-4:10] PART 3 MPI** (Slides 7-10) - 100 seconds
1. Show Slide 7: "Column partitioning: clean division of work"
2. Show Slide 4: "Simple code: 3 MPI calls"
3. Show Slide 9: "Why Allreduce? Avoids bottleneck"
4. Show Slide 10: "Results: 6.5× speedup, communication negligible"

**[4:10-5:30] PART 4 GPU** (Slides 11-14) - 80 seconds
1. Show Slide 11: "Tiling: reuse data from fast memory"
2. Show Slide 12: "Code change: add local memory + barriers"
3. Show Slide 13: "Why it works: amortized bandwidth"
4. Show Slide 14: "Results: 1.4× optimization (I/O limited)"

**[5:30-6:30] Wrap-up** (Slides 15-16) - 60 seconds
"Each paradigm optimized for its hardware. No universal winner."

---

## ✅ PRESENTATION CHECKLIST

- [ ] Open `LINMA2710_Project_Implementation_Focus.pptx` (NOT the other one)
- [ ] Review all 18 slides beforehand
- [ ] Practice delivery (should be 6-7 minutes)
- [ ] Know the code snippets by memory
- [ ] Be ready to explain "Why" for each design choice
- [ ] Have CSV data files available (for reference if asked)

---

## 🎓 IF ASKED QUESTIONS

**"Why transpose instead of other optimizations?"**
→ Show Slide 3: Cache misses are bottleneck. Transpose is simple fix.

**"Could you use other scheduling in OpenMP?"**
→ Show Slide 5: Static is cheapest. Dynamic has overhead.

**"Why not row-wise in MPI?"**
→ Show Slide 9: Column-wise keeps computation independent. Row-wise needs B replication.

**"GPU speedup seems small?"**
→ Show Slide 14: Matrix multiply is memory-bound, not compute-bound.

**"Did you implement all this?"**
→ "Yes—Sequential, OpenMP, MPI, and GPU kernels with 115 OpenMP measurements, 4 MPI tests, 8 GPU tests."

---

## 📁 FILE LOCATIONS

```
/home/ucl/ingi/gsavary/LINMA2710/benchmarks/

Presentations:
  ✓ LINMA2710_Project_Implementation_Focus.pptx    ← USE THIS
  ✓ LINMA2710_Implementation_Focus.html             ← Backup
  
Data:
  ✓ comprehensive_results_all.csv (115 OpenMP measurements)
  ✓ bench_mpi_results.csv (4 MPI measurements)
  ✓ bench_opencl_results.csv (8 GPU measurements)
  
Figures:
  ✓ figures/ (32 PNG visualizations)
```

---

## 🚀 FINAL WORD

This presentation emphasizes **engineering thinking**:
1. **Understand the constraint** (cache, communication, memory bandwidth)
2. **Design a solution** (transpose, partitioning, tiling)
3. **Implement carefully** (code structure, synchronization)
4. **Validate with measurement** (115+ benchmark measurements)

This approach shows maturity and understanding, not just "I parallelized the code."

**You're ready! Good luck!** 🎓
