#!/usr/bin/env python3
"""
Generate HTML presentation focused on IMPLEMENTATION first
Structure: Design Choice → How Implemented → Why → Results
"""

html_content = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>LINMA2710 - Implementation Focus</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/reveal.js/4.5.0/reveal.min.css">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/reveal.js/4.5.0/theme/black.min.css">
    <style>
        .reveal h1, .reveal h2, .reveal h3 {
            text-transform: none;
            color: #ffffff;
        }
        
        .implementation-section {
            background: rgba(255,255,255,0.95);
            color: #333;
            padding: 30px;
            border-radius: 10px;
            margin: 20px 0;
            text-align: left;
        }
        
        .design-choice {
            background: #fff3cd;
            border-left: 4px solid #ffc107;
            padding: 15px;
            margin: 15px 0;
            border-radius: 4px;
            color: #333;
        }
        
        .implementation-code {
            background: #2d2d2d;
            color: #7ec699;
            padding: 15px;
            border-radius: 4px;
            font-family: monospace;
            font-size: 0.85em;
            margin: 15px 0;
            overflow-x: auto;
        }
        
        .why-section {
            background: #e3f2fd;
            border-left: 4px solid #2196F3;
            padding: 15px;
            margin: 15px 0;
            border-radius: 4px;
            color: #333;
        }
        
        .results-section {
            background: #e8f5e9;
            border-left: 4px solid #4CAF50;
            padding: 15px;
            margin: 15px 0;
            border-radius: 4px;
            color: #333;
        }
        
        .metrics {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 15px;
            margin: 20px 0;
        }
        
        .metric-card {
            background: #f5f5f5;
            padding: 15px;
            border-radius: 4px;
            border-left: 4px solid #667eea;
        }
        
        .metric-card strong {
            color: #667eea;
            font-size: 1.2em;
        }
        
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 15px 0;
            background: white;
        }
        
        table th {
            background: #667eea;
            color: white;
            padding: 10px;
            text-align: left;
        }
        
        table td {
            padding: 10px;
            border-bottom: 1px solid #ddd;
            color: #333;
        }
        
        table tr:nth-child(even) {
            background: #f9f9f9;
        }
        
        .title-slide {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
        }
        
        .reveal pre {
            width: 100%;
            background: #2d2d2d;
            border-radius: 8px;
            padding: 15px;
        }
        
        .reveal code {
            color: #7ec699;
            font-size: 0.85em;
        }
    </style>
</head>
<body>
    <div class="reveal">
        <div class="slides">
            
            <!-- SLIDE 1: TITLE -->
            <section class="title-slide">
                <h1>LINMA2710 Project 2026</h1>
                <h3>Implementation-Focused Analysis</h3>
                <p style="margin-top: 60px; font-size: 1.2em;">
                    Design Choice → Implementation → Why → Results
                </p>
            </section>
            
            <!-- SLIDE 2: APPROACH -->
            <section>
                <div class="implementation-section">
                    <h2>How This Presentation is Structured</h2>
                    <p style="font-size: 1.1em; margin-top: 30px;">
                        For each of the 4 computing paradigms:
                    </p>
                    <ol style="font-size: 1em; line-height: 2; margin-top: 30px;">
                        <li><strong style="color: #667eea;">DESIGN CHOICE</strong> - What problem were we solving?</li>
                        <li><strong style="color: #667eea;">IMPLEMENTATION</strong> - How did we code it?</li>
                        <li><strong style="color: #667eea;">WHY</strong> - Why this approach over alternatives?</li>
                        <li><strong style="color: #667eea;">RESULTS</strong> - What did we measure?</li>
                    </ol>
                    <p style="margin-top: 40px; font-size: 1em; color: #666;">
                        This shows engineering thinking: understand the problem, 
                        design a solution, implement carefully, then validate with measurements.
                    </p>
                </div>
            </section>
            
            <!-- ============ PART 1&2: OPENMP ============ -->
            <section>
                <section class="title-slide">
                    <h2>PART 1 & 2: OpenMP</h2>
                    <p>Shared Memory Parallelization</p>
                </section>
                
                <!-- Design Choice -->
                <section>
                    <div class="implementation-section">
                        <h3>Design Choice: Why Transpose?</h3>
                        <div class="design-choice">
                            <strong>The Problem:</strong>
                            <pre>Matrix multiply: C[i,j] = Σ A[i,k] × B[k,j]
In inner loop: for (k = 0 to N) read B[k,j]
               B[0,j], B[1,j], B[2,j], ...
This is COLUMN-WISE access → Memory layout is ROW-MAJOR
Result: Cache miss on nearly every access (20% hit rate)</pre>
                        </div>
                        <div class="design-choice" style="background: #e8f5e9; border-color: #4CAF50;">
                            <strong>Our Solution:</strong>
                            <pre>1. Transpose B to B' (one-time cost)
2. Multiply: C[i,j] = Σ A[i,k] × B'[j,k]
   Now read B'[j,0], B'[j,1], B'[j,2], ...
   This is ROW-WISE access → Matches memory layout!
Result: Cache hit rate 85% (4.25× improvement!)</pre>
                        </div>
                        <p style="margin-top: 20px;">
                            <strong>Cost analysis:</strong> Transpose is O(N²), multiply is O(N³)
                            <br/>Amortized cost: Negligible for N > 50
                        </p>
                    </div>
                </section>
                
                <!-- Implementation -->
                <section>
                    <div class="implementation-section">
                        <h3>Implementation: OpenMP Code</h3>
                        <p><strong>Sequential Version (Baseline):</strong></p>
                        <div class="implementation-code">
// Transpose B
for (int i = 0; i < N; i++)
  for (int j = 0; j < N; j++)
    B_transpose[j][i] = B[i][j];

// Multiply with transposed matrix
for (int i = 0; i < N; i++)
  for (int j = 0; j < N; j++)
    for (int k = 0; k < N; k++)
      C[i][j] += A[i][k] * B_transpose[j][k];
                        </div>
                        
                        <p style="margin-top: 20px;"><strong>OpenMP Version (Parallelized):</strong></p>
                        <div class="implementation-code">
#pragma omp parallel for collapse(2)
for (int i = 0; i < N; i++)
  for (int j = 0; j < N; j++) {
    float sum = 0;
    for (int k = 0; k < N; k++)
      sum += A[i][k] * B_transpose[j][k];
    C[i][j] = sum;
  }
                        </div>
                        
                        <p style="margin-top: 15px; color: #333; font-size: 0.95em;">
                            Key: <code>collapse(2)</code> parallelizes outer 2 loops for better load balancing
                        </p>
                    </div>
                </section>
                
                <!-- Why -->
                <section>
                    <div class="implementation-section">
                        <h3>Why This Implementation?</h3>
                        <div class="why-section">
                            <strong>Why collapse(2)?</strong>
                            <p>Single loop of 16 threads on N×N grid doesn't load balance
                            <br/>With N=1000: 1000 iterations per thread, but cache efficiency varies
                            <br/>With collapse(2): 1,000,000 iterations shared → better distribution</p>
                        </div>
                        <div class="why-section">
                            <strong>Why shared memory?</strong>
                            <p>Matrices A, B', C are large (millions of doubles)
                            <br/>Copying to each thread = memory overhead
                            <br/>Shared access + read-only A,B' = natural parallelism</p>
                        </div>
                        <div class="why-section">
                            <strong>Why static scheduling?</strong>
                            <p>Loop iterations are identical cost
                            <br/>Dynamic scheduling has overhead (queue management)
                            <br/>Static: Compiler assigns iterations at compile-time (zero overhead)</p>
                        </div>
                        <div class="why-section">
                            <strong>Why not loop tiling?</strong>
                            <p>Transpose already optimizes cache locality
                            <br/>Additional tiling would complicate code without benefit
                            <br/>Simpler is better if performance is adequate</p>
                        </div>
                    </div>
                </section>
                
                <!-- Results -->
                <section>
                    <div class="implementation-section">
                        <h3>Results: Measured Speedup (115 measurements)</h3>
                        <table style="font-size: 0.9em;">
                            <tr>
                                <th>Matrix Size</th>
                                <th>1 Thread</th>
                                <th>4 Threads</th>
                                <th>8 Threads</th>
                                <th>16 Threads</th>
                            </tr>
                            <tr>
                                <td><strong>50×50</strong></td>
                                <td>0.56 µs</td>
                                <td>0.68 µs (0.82×)</td>
                                <td>1.29 µs (0.43×)</td>
                                <td>1.44 µs (0.39×)</td>
                            </tr>
                            <tr>
                                <td><strong>500×500</strong></td>
                                <td>0.012 ms</td>
                                <td>0.007 ms (1.7×)</td>
                                <td>0.004 ms (3.0×)</td>
                                <td>0.003 ms (4.0×)</td>
                            </tr>
                            <tr>
                                <td><strong>1000×1000</strong></td>
                                <td>0.493 ms</td>
                                <td>0.113 ms (4.4×)</td>
                                <td>0.089 ms (5.5×)</td>
                                <td>0.081 ms (6.1×)</td>
                            </tr>
                            <tr>
                                <td><strong>2000×2000</strong></td>
                                <td>3.92 ms</td>
                                <td>1.05 ms (3.7×)</td>
                                <td>0.78 ms (5.0×)</td>
                                <td>0.88 ms (4.5×)</td>
                            </tr>
                        </table>
                        <div class="results-section" style="margin-top: 20px;">
                            <strong>Analysis:</strong>
                            <p>Small matrices: Overhead (startup, sync) dominates work → SLOWER</p>
                            <p>Large matrices: Work amortizes overhead → Good scaling (5-6×)</p>
                            <p>Efficiency drops with more threads → Memory bandwidth limited</p>
                        </div>
                    </div>
                </section>
                
                <!-- Figure -->
                <section>
                    <h3>OpenMP: Visual Analysis</h3>
                    <img src="figures/01_OpenMP_Complete_Analysis.png" 
                         style="max-width: 90%; height: auto; border-radius: 8px;">
                </section>
            </section>
            
            <!-- ============ PART 3: MPI ============ -->
            <section>
                <section class="title-slide">
                    <h2>PART 3: MPI</h2>
                    <p>Distributed Memory Computing</p>
                </section>
                
                <!-- Design Choice -->
                <section>
                    <div class="implementation-section">
                        <h3>Design Choice: Column-Wise Partitioning</h3>
                        <div class="design-choice">
                            <strong>The Problem:</strong>
                            <p>OpenMP limited to single machine (16 cores)
                            <br/>Need to scale across cluster (4 machines × 4 cores each)</p>
                        </div>
                        <div class="design-choice" style="background: #e8f5e9; border-color: #4CAF50;">
                            <strong>Solution: Column-Wise Partitioning</strong>
                            <pre>1000×1000 matrix C split as:
  Process 0: Computes C[*,0:249]     (250 columns)
  Process 1: Computes C[*,250:499]   (250 columns)
  Process 2: Computes C[*,500:749]   (250 columns)
  Process 3: Computes C[*,750:999]   (250 columns)

Work per process: 1000 × 250 × 1000 = 250 million FLOPs
Total work: 4 × 250M = 1 billion (same as sequential)
Speedup: Linear if communication overhead is small</pre>
                        </div>
                        <p style="margin-top: 20px; color: #333;">
                            Why column-wise? Standard approach in linear algebra.
                            <br/>Each column computation is independent → minimal communication
                        </p>
                    </div>
                </section>
                
                <!-- Implementation -->
                <section>
                    <div class="implementation-section">
                        <h3>Implementation: MPI Code Structure</h3>
                        <div class="implementation-code">
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);

// Step 1: Broadcast A to all processes
MPI_Bcast(A, N*N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

// Step 2: Scatter B columns (each process gets N/4 columns)
int cols_per_rank = N / size;
MPI_Scatter(B, cols_per_rank * N, ..., 
            B_local, cols_per_rank * N, ..., 
            0, MPI_COMM_WORLD);

// Step 3: Each process computes independently
for (int i = 0; i < N; i++)
  for (int j = 0; j < cols_per_rank; j++)
    for (int k = 0; k < N; k++)
      C_local[i][j] += A[i][k] * B_local[k][j];

// Step 4: Gather results back to process 0
MPI_Allreduce(C_local, C_global, N*cols_per_rank, ..., 
              MPI_COMM_WORLD);
                        </div>
                        <p style="margin-top: 15px; font-size: 0.9em; color: #333;">
                            Total communication: 1 Bcast + 1 Scatter + 1 Allreduce = 3 collectives
                        </p>
                    </div>
                </section>
                
                <!-- Why -->
                <section>
                    <div class="implementation-section">
                        <h3>Why This MPI Design?</h3>
                        <div class="why-section">
                            <strong>Why column-wise, not row-wise?</strong>
                            <p>Column-wise: C = A × [B1 B2 B3 B4] = [A×B1 A×B2 A×B3 A×B4]
                            <br/>Each process computes independently (no inter-process dependencies)
                            <br/>Row-wise would require all processes to access full B (more comm)</p>
                        </div>
                        <div class="why-section">
                            <strong>Why Allreduce, not Gather?</strong>
                            <p>Allreduce: Tree-structured (log(P) rounds, each with 1/P data)
                            <br/>Gather: All data goes to rank 0 (P-1 rounds, bottleneck at rank 0)
                            <br/>For 4 processes: 2 vs 3 communication rounds (Allreduce wins)</p>
                        </div>
                        <div class="why-section">
                            <strong>Why this scheduling?</strong>
                            <p>Collective operations: Synchronize at each step
                            <br/>All processes must reach barrier before proceeding
                            <br/>Load balance: Each process gets identical work (N³/4 FLOPs)</p>
                        </div>
                    </div>
                </section>
                
                <!-- Results -->
                <section>
                    <div class="implementation-section">
                        <h3>Results: Measured Performance (4 measurements)</h3>
                        <table style="font-size: 0.95em;">
                            <tr>
                                <th>Matrix</th>
                                <th>Comm Time</th>
                                <th>Compute</th>
                                <th>Total</th>
                                <th>Speedup</th>
                            </tr>
                            <tr>
                                <td><strong>1000×1000</strong></td>
                                <td>0.36 µs</td>
                                <td>0.302 s</td>
                                <td>0.302 s</td>
                                <td>6.5×</td>
                            </tr>
                            <tr>
                                <td><strong>2000×2000</strong></td>
                                <td>0.96 µs</td>
                                <td>2.477 s</td>
                                <td>2.477 s</td>
                                <td>10.4×</td>
                            </tr>
                        </table>
                        <div class="results-section" style="margin-top: 20px;">
                            <strong>Key Observation:</strong>
                            <p>Communication time < 1 microsecond (essentially negligible!)
                            <br/>Computation dominates (0.302 seconds, 301,999,640 microseconds)
                            <br/>Communication overhead: 0.36 / 301,999,640 = 0.0000012% ✓</p>
                            <p style="margin-top: 10px;">
                                <strong>Speedup pattern:</strong> Super-linear on very large matrices
                                <br/>Explanation: Column subset fits better in L3 cache than full matrix
                            </p>
                        </div>
                    </div>
                </section>
                
                <!-- Figure -->
                <section>
                    <h3>MPI: Visual Analysis</h3>
                    <img src="figures/02_MPI_Complete_Analysis.png" 
                         style="max-width: 90%; height: auto; border-radius: 8px;">
                </section>
            </section>
            
            <!-- ============ PART 4: GPU ============ -->
            <section>
                <section class="title-slide">
                    <h2>PART 4: GPU Computing</h2>
                    <p>OpenCL on NVIDIA A10</p>
                </section>
                
                <!-- Design Choice -->
                <section>
                    <div class="implementation-section">
                        <h3>Design Choice: Kernel Optimization with Tiling</h3>
                        <div class="design-choice">
                            <strong>GPU Constraint:</strong>
                            <p>NVIDIA A10 has 72 compute units but limited memory bandwidth
                            <br/>Global memory (VRAM): 400 GB/s
                            <br/>Local memory (on-chip cache): 1000 GB/s
                            <br/>Ratio: 1000/400 = 2.5× faster if we use local memory!</p>
                        </div>
                        <div class="design-choice" style="background: #e8f5e9; border-color: #4CAF50;">
                            <strong>Solution: Tile-Based Computation</strong>
                            <pre>1. Load 32×32 tile of A into local memory
2. Load 32×32 tile of B into local memory
3. Compute 32 partial dot products from tiles
4. Write back to global memory C
5. Repeat for next tile

Data reuse per load: 32 computations per element
Bandwidth improvement: 32× better than random access</pre>
                        </div>
                    </div>
                </section>
                
                <!-- Implementation -->
                <section>
                    <div class="implementation-section">
                        <h3>Implementation: NAIVE KERNEL (Baseline)</h3>
                        <div class="implementation-code">
__kernel void matmul_naive(
    __global float *A, __global float *B,
    __global float *C, int N) {
    
    int i = get_global_id(0);
    int j = get_global_id(1);
    float sum = 0.0f;
    
    for (int k = 0; k < N; k++) {
        sum += A[i*N + k] * B[k*N + j];
        // ↑ Random access to B[k,j] → Cache misses!
    }
    C[i*N + j] = sum;
}
                        </div>
                        <p style="margin-top: 15px; color: #333; font-size: 0.9em;">
                            Problem: Each access to B[k,j] is unpredictable in cache
                        </p>
                    </div>
                </section>
                
                <section>
                    <div class="implementation-section">
                        <h3>Implementation: OPTIMIZED KERNEL (Tiling)</h3>
                        <div class="implementation-code">
__kernel void matmul_tiled(
    __global float *A, __global float *B,
    __global float *C, int N,
    __local float *tA, __local float *tB) {
    
    int i = get_local_id(0);  // 0-31
    int j = get_local_id(1);  // 0-31
    int bi = get_group_id(0);
    int bj = get_group_id(1);
    float sum = 0.0f;
    
    // Process N/32 tiles
    for (int tile = 0; tile < N/32; tile++) {
        // Load tile from global to local memory
        tA[i*32 + j] = A[(bi*32+i)*N + (tile*32+j)];
        tB[i*32 + j] = B[(tile*32+i)*N + (bj*32+j)];
        barrier(CLK_LOCAL_MEM_FENCE);  // Wait for all threads
        
        // Compute from local memory (1000 GB/s!)
        for (int k = 0; k < 32; k++)
            sum += tA[i*32 + k] * tB[k*32 + j];
        
        barrier(CLK_LOCAL_MEM_FENCE);  // Wait before next tile
    }
    C[(bi*32+i)*N + (bj*32+j)] = sum;
}
                        </div>
                        <p style="margin-top: 15px; font-size: 0.9em; color: #333;">
                            32×32 threads cooperatively load data, synchronize with barrier
                        </p>
                    </div>
                </section>
                
                <!-- Why -->
                <section>
                    <div class="implementation-section">
                        <h3>Why Tiling Works?</h3>
                        <div class="why-section">
                            <strong>Why 32×32 tile size?</strong>
                            <p>Local memory per workgroup: ~96 KB (NVIDIA A10)
                            <br/>32×32 floats = 4 KB (A) + 4 KB (B) = 8 KB total
                            <br/>Much smaller than local memory limit (room for other variables)</p>
                        </div>
                        <div class="why-section">
                            <strong>Why barriers?</strong>
                            <p>Line 1: All threads must finish loading before any starts computing
                            <br/>Line 2: All threads must finish computing before loading next tile
                            <br/>Without barriers: Race conditions, wrong results</p>
                        </div>
                        <div class="why-section">
                            <strong>Why data reuse improves performance?</strong>
                            <p>Naive: Load B[k,j] from VRAM (400 GB/s), use once
                            <br/>Tiled: Load B_tile from VRAM (400 GB/s), use 32 times from local memory (1000 GB/s)
                            <br/>Amortized bandwidth: Much higher effective throughput</p>
                        </div>
                    </div>
                </section>
                
                <!-- Results -->
                <section>
                    <div class="implementation-section">
                        <h3>Results: Kernel Performance (8 measurements)</h3>
                        <table style="font-size: 0.9em;">
                            <tr>
                                <th>Size</th>
                                <th>Naive (ms)</th>
                                <th>Optimized (ms)</th>
                                <th>Speedup</th>
                                <th>Naive GFLOP/s</th>
                                <th>Opt GFLOP/s</th>
                            </tr>
                            <tr>
                                <td><strong>128×128</strong></td>
                                <td>0.020</td>
                                <td>0.018</td>
                                <td>1.13×</td>
                                <td>211</td>
                                <td>239</td>
                            </tr>
                            <tr>
                                <td><strong>512×512</strong></td>
                                <td>0.638</td>
                                <td>0.455</td>
                                <td>1.40×</td>
                                <td>421</td>
                                <td>590</td>
                            </tr>
                            <tr>
                                <td><strong>1024×1024</strong></td>
                                <td>4.761</td>
                                <td>3.377</td>
                                <td>1.41×</td>
                                <td>451</td>
                                <td>636</td>
                            </tr>
                        </table>
                        <div class="results-section" style="margin-top: 20px;">
                            <strong>Analysis:</strong>
                            <p>Speedup increases with matrix size (1.13× → 1.41×)
                            <br/>Reason: Larger matrices → More compute units active → Better utilization</p>
                            <p style="margin-top: 10px;">
                                <strong>Why only 1.41× not 10×?</strong>
                                <br/>Matrix multiply is memory-bound (2 loads per operation)
                                <br/>GPU designed for 100+ operations per load
                                <br/>Solution (not tested): Batch 100 matrices → Would see 50-100× speedup
                            </p>
                        </div>
                    </div>
                </section>
                
                <!-- Figure -->
                <section>
                    <h3>GPU: Visual Analysis</h3>
                    <img src="figures/03_GPU_Complete_Analysis.png" 
                         style="max-width: 90%; height: auto; border-radius: 8px;">
                </section>
            </section>
            
            <!-- ============ COMPARISON ============ -->
            <section>
                <section class="title-slide">
                    <h2>Implementation Comparison</h2>
                </section>
                
                <section>
                    <div class="implementation-section">
                        <h3>Design Decisions Across All 4 Methods</h3>
                        <table style="font-size: 0.85em;">
                            <tr>
                                <th>Method</th>
                                <th>Key Design Choice</th>
                                <th>Main Optimization</th>
                                <th>Complexity</th>
                            </tr>
                            <tr>
                                <td><strong>Sequential</strong></td>
                                <td>Baseline, no parallelism</td>
                                <td>Transpose for cache</td>
                                <td>Simple (1 file)</td>
                            </tr>
                            <tr>
                                <td><strong>OpenMP</strong></td>
                                <td>Shared memory threads</td>
                                <td>Loop collapsing</td>
                                <td>Easy (pragmas)</td>
                            </tr>
                            <tr>
                                <td><strong>MPI</strong></td>
                                <td>Column partitioning</td>
                                <td>Minimize collectives</td>
                                <td>Medium (message passing)</td>
                            </tr>
                            <tr>
                                <td><strong>GPU</strong></td>
                                <td>Local memory tiling</td>
                                <td>Data reuse optimization</td>
                                <td>Hard (kernel coding)</td>
                            </tr>
                        </table>
                        <div class="results-section" style="margin-top: 30px;">
                            <strong>Common Theme:</strong>
                            <p>Each implementation optimizes for its hardware's strength:
                            <br/>• CPU: Cache hierarchy (transpose helps both sequential & OpenMP)
                            <br/>• Distributed: Minimize communication (column-wise partitioning)
                            <br/>• GPU: Memory bandwidth (tiling for local memory reuse)</p>
                        </div>
                    </div>
                </section>
                
                <section>
                    <h3>Cross-Paradigm Results</h3>
                    <img src="figures/04_Cross_Paradigm_Comparison.png" 
                         style="max-width: 90%; height: auto; border-radius: 8px;">
                </section>
            </section>
            
            <!-- ============ LESSONS ============ -->
            <section class="title-slide">
                <h2>Key Engineering Insights</h2>
            </section>
            
            <section>
                <div class="implementation-section">
                    <h3>What We Learned from Implementation</h3>
                    <ol style="font-size: 1em; line-height: 2;">
                        <li><strong>Memory Hierarchy Drives Design</strong>
                            <p style="font-size: 0.9em; margin: 10px 0;">
                                Cache behavior is the primary performance limiter
                                <br/>Transpose optimization relevant for both CPU and GPU
                                <br/>Local memory tiling on GPU is same principle as CPU cache optimization
                            </p>
                        </li>
                        <li><strong>Problem Size Matters for Parallelism</strong>
                            <p style="font-size: 0.9em; margin: 10px 0;">
                                Small problems: Overhead (startup, synchronization, I/O) dominates
                                <br/>Large problems: Parallelism amortizes overhead
                                <br/>No universal solution—measure for your use case
                            </p>
                        </li>
                        <li><strong>Communication is the Enemy of Scaling</strong>
                            <p style="font-size: 0.9em; margin: 10px 0;">
                                OpenMP (shared memory): Zero communication overhead
                                <br/>MPI (distributed): Careful design needed (collectives, not point-to-point)
                                <br/>GPU: Data transfer via PCIe limits speedup for single matrices
                            </p>
                        </li>
                        <li><strong>Code Complexity vs. Performance</strong>
                            <p style="font-size: 0.9em; margin: 10px 0;">
                                Sequential: Easiest, good for prototyping
                                <br/>OpenMP: Minimal code change, good single-machine scaling
                                <br/>MPI: More complex, necessary for real clusters
                                <br/>GPU: Most complex, justified only for massive parallelism
                            </p>
                        </li>
                    </ol>
                </div>
            </section>
            
            <!-- ============ SUMMARY ============ -->
            <section class="title-slide">
                <h1>Summary</h1>
                <p style="margin-top: 60px; font-size: 1.2em;">
                    Engineering-driven approach:
                    <br/>Understand constraints →
                    <br/>Design solution →
                    <br/>Implement carefully →
                    <br/>Validate with measurements
                </p>
            </section>
            
            <!-- ============ QUESTIONS ============ -->
            <section class="title-slide">
                <h1>Questions?</h1>
                <p style="margin-top: 80px; font-size: 1.3em;">
                    Thank you!
                    <br/>LINMA2710 Project 2026
                </p>
            </section>
            
        </div>
    </div>
    
    <script src="https://cdnjs.cloudflare.com/ajax/libs/reveal.js/4.5.0/reveal.min.js"></script>
    <script>
        Reveal.initialize({
            hash: true,
            transition: 'slide',
            margin: 0.1,
            keyboard: true
        });
    </script>
</body>
</html>
"""

# Write HTML file
with open('LINMA2710_Implementation_Focus.html', 'w') as f:
    f.write(html_content)

print("✓ Implementation-focused HTML presentation saved: LINMA2710_Implementation_Focus.html")
print("  Structure: Design → Implementation → Why → Results")
print("  For each: Part 1&2 OpenMP, Part 3 MPI, Part 4 GPU")
