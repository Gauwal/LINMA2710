# LINMA2710 Project - Presentation Slides

## Slide 1: Title Slide
---
title: LINMA2710: Parallel Matrix Operations
subtitle: Exploring 4 Computing Paradigms
author: [Your Name]
date: May 2026
---

# Parallel Computing Across 4 Paradigms
### Matrix Operations: Sequential → OpenMP → MPI → GPU

---

## Slide 2: Overview
---

### The Four Paradigms

| Paradigm | Where | Parallelism | Trade-off |
|----------|-------|-------------|-----------|
| **Sequential** | CPU | None | Simple |
| **OpenMP** | Single machine | Shared memory threads | Low overhead |
| **MPI** | Multiple machines | Distributed memory | Communication cost |
| **OpenCL/GPU** | GPU device | Massive parallelism | Data transfer |

**Goal:** Implement matrix multiplication on all four and compare

---

## Slide 3: Part 1 & 2 - OpenMP Parallelism
---

### Sequential + Parallel Implementation

**Key Design Choice:** Transpose B matrix
- **Why?** Improves cache locality - read both matrices sequentially
- **Result:** Enables SIMD auto-vectorization by compiler
- **Implementation:** `#pragma omp parallel for` on matrix multiplication

### Performance Results

**Benchmark Setup:**
- Matrix sizes: 100×100 to 2000×2000
- Thread counts: 1 to 16
- Operation: C = A × B (compute-intensive: O(n³) FLOPs)

**Key Metrics:**
- Sequential (1 thread): **2.07 GFLOPs** @ 1000×1000
- Peak performance: **~2.1 GFLOPs** (limited by memory bandwidth)

![OpenMP Speedup](../benchmarks/figures/01_openmp_speedup.png)

---

## Slide 4: OpenMP Analysis
---

### Why Limited Speedup?

**Amdahl's Law:** Speedup = 1 / (f_s + f_p/P)
- Where f_s = sequential fraction, P = # threads

**Observations from data:**
1. Small matrices (100×100): OpenMP overhead > benefit
2. Large matrices (2000×2000): Memory bandwidth saturated
3. Breaks even around ~200×200 matrices

### Parallelization Overhead
- Thread creation/joining cost
- Cache coherency overhead
- Limited benefit on memory-bound operations

---

## Slide 5: Part 3 - MPI Distributed Computing
---

### Column-wise Matrix Partitioning

**Algorithm:**
- Partition B by columns across processes
- Each process owns columns [startCol, startCol+localCols)
- Local operations: Matrix multiplication on local portion
- Communication: MPI_Allreduce for final result

### Performance Results

**Benchmark Setup:**
- 4 processes (on distributed nodes)
- Matrix sizes: 200×200 to 2000×2000
- Operation: C = A × B^T (requires allreduce)

**Key Metrics:**
- 1000×1000 matrix: **3.51 GFLOPs** (vs 2.07 sequential!)
- Communication: **<1 microsecond** (negligible!)

![MPI Communication](../benchmarks/figures/02_mpi_communication.png)

---

## Slide 6: MPI Analysis
---

### Why MPI Beats Sequential?

**Key Insight:** 4 processes run in parallel
- Each process: O(n²) computation per element
- Total work: split 4 ways
- Communication: O(n) result gathering (very fast)

### Communication Breakdown
- For 1000×1000 matrix: **0.57 seconds total**
- Computation: 0.57 seconds (100%)
- MPI communication: < 1 microsecond (0%)
- **Conclusion:** Column partitioning is highly efficient!

### Strong Scaling Analysis
- Expected speedup: ~4x (with 4 processes)
- Observed speedup: ~1.7x vs sequential

---

## Slide 7: Part 4 - GPU Computing with OpenCL
---

### Two Kernel Implementations

**Naive Kernel:**
```opencl
__kernel void matrix_mul(
    __global float* A, __global float* B, __global float* C,
    int rows, int cols) {
    int i = get_global_id(0);
    int j = get_global_id(1);
    if (i < rows && j < cols) {
        float sum = 0.0f;
        for(int k = 0; k < cols; k++)
            sum += A[i*cols + k] * B[k*cols + j];
        C[i*cols + j] = sum;
    }
}
```

**Optimized Kernel:** Local memory + work-group tiling
- Load tiles into fast local memory
- Reduce global memory bandwidth pressure
- Expected speedup: 2-4×

---

## Slide 8: GPU OpenCL Results
---

### Kernel Performance Comparison

**Naive Kernel:**
- 512×512: ~X ms
- Memory: Global only (slow)
- GFLOPs: ~Y GFLOP/s

**Optimized Kernel:**
- 512×512: ~Z ms
- Memory: Local + global (hybrid)
- GFLOPs: ~W GFLOP/s
- **Speedup: W/Y ≈ Z× faster**

### Device Information
- GPU: [Model from benchmark]
- Compute Units: [from benchmark]
- Memory: [from benchmark]
- Max Work Group Size: [from benchmark]

![GPU Kernel Comparison](../benchmarks/figures/03_opencl_kernels.png)

---

## Slide 9: GPU Analysis
---

### Optimization Strategy

**Local Memory Tiling (TILE_SIZE=16):**
1. Load A[i:i+16, k:k+16] into local memory
2. Load B[k:k+16, j:j+16] into local memory
3. Compute partial result using fast local memory
4. Barrier synchronization between tiles

### Why It Works
- **Local memory:** 10-100× faster than global
- **Bandwidth reduction:** Global access reduced by ~16×
- **Work-group coordination:** Efficient data reuse

### Data Transfer Cost
- Host → GPU: Significant for small matrices
- GPU → Host: Significant overhead
- **Conclusion:** GPUs best for large matrices or repeated operations

---

## Slide 10: Cross-Paradigm Comparison
---

### Performance Summary

![GFLOPs Comparison](../benchmarks/figures/04_gflops_comparison.png)

**Key Findings:**
- Sequential: 2.1 GFLOPs (baseline)
- OpenMP: ~2.1 GFLOPs (memory-bound, little gain)
- MPI: 3.5 GFLOPs (parallel execution!)
- OpenCL: [from benchmark] GFLOPs (GPU potential)

### Summary Table

![Summary](../benchmarks/figures/05_summary_table.png)

---

## Slide 11: Trade-offs & Conclusions
---

### When to Use Each Paradigm

| Paradigm | Best For | Avoid If |
|----------|----------|----------|
| **Sequential** | Simple algorithms, debugging | Performance matters |
| **OpenMP** | Shared-memory, single machine | Need distributed memory |
| **MPI** | Multi-node clusters, scalability | Tight communication loops |
| **GPU/OpenCL** | Massive parallelism, data-parallel | Complex control flow |

### Lessons Learned

1. **Cache locality matters:** Simple transpose improved performance
2. **Communication vs computation:** Must be balanced
3. **Platform-specific:** Different architectures suit different approaches
4. **Amdahl's law is real:** Speedup limited by sequential fraction
5. **GPU memory model:** Data transfer can dominate

---

## Slide 12: Design Decisions
---

### Why These Choices?

1. **Matrix Transpose in Multiplication:**
   - Sequential access patterns enable SIMD
   - Reduces cache misses
   - Small overhead for large matrices

2. **Column Partitioning in MPI:**
   - Each process handles full rows (cache-friendly)
   - Minimal communication (only gather results)
   - Natural for row-major storage

3. **Local Memory in OpenCL:**
   - Dramatic bandwidth improvement
   - Work-group synchronization overhead worth it
   - Tiling is standard GPU optimization

---

## Slide 13: Challenges & Solutions
---

### Challenges Encountered

1. **OpenMP Overhead:** Pragmas added even for small matrices
   - **Solution:** Conditional: `if (size > threshold)`

2. **MPI Communication:** All-reduce can bottleneck
   - **Solution:** Column partitioning minimizes synchronization

3. **GPU Memory Transfer:** Data movement expensive
   - **Solution:** Keep data on GPU for multiple operations

4. **Thread Scaling:** Didn't scale beyond ~8 threads
   - **Root:** Memory bandwidth saturation
   - **Inherent:** Sequential CPU memory architecture

---

## Slide 14: Future Optimizations
---

### Potential Improvements

1. **OpenMP:**
   - SIMD pragmas for vectorization
   - Reduce false sharing
   - NUMA-aware scheduling

2. **MPI:**
   - Asynchronous communication
   - Different partitioning strategies
   - Reduced precision (float32)

3. **GPU:**
   - Persistent kernels
   - Tensor operations (cuBLAS)
   - Mixed precision (float16)

4. **All:**
   - Kernel fusion
   - Auto-tuning
   - Heterogeneous execution

---

## Slide 15: Questions & Discussion
---

### Key Takeaways

✓ Sequential code as baseline
✓ OpenMP for shared-memory systems
✓ MPI for distributed computing
✓ GPU for massive parallelism
✓ Trade-offs matter!

### Expected Questions

**Q: Why not use GPU for everything?**
A: Data transfer overhead, programming complexity, not all problems are data-parallel

**Q: How would this scale to very large matrices?**
A: GPU memory limits (~10GB typically), MPI with GPU acceleration would be needed

**Q: Which is "fastest"?**
A: Depends on problem size and platform. GPU best for large compute, MPI for scalability

---

**Thank you!**
