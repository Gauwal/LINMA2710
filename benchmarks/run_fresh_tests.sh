#!/bin/bash
#SBATCH --partition=gpu
#SBATCH --gres=gpu:1
#SBATCH --time=00:30:00
#SBATCH --mem-per-cpu=4000
#SBATCH --cpus-per-task=16
#SBATCH --job-name=LINMA_verify_claims
#SBATCH --output=verify_claims_%j.log
#SBATCH --error=verify_claims_%j.err

set -e

echo "=================================================="
echo "CLAIM VERIFICATION - FRESH HARDWARE TESTS"
echo "=================================================="
echo ""
echo "Hostname: $(hostname)"
echo "Date: $(date)"
echo "SLURM Job ID: $SLURM_JOB_ID"
echo ""

# Load required modules
echo "Loading modules..."
module load releases/2024a GCC/13.3.0
echo "✓ Modules loaded"
echo ""

# Check for GPU
echo "Checking GPU availability..."
nvidia-smi -L
echo ""

# Create test source files
echo "Creating test source files..."

# ==============================================================================
# TEST 1: OpenMP Speedup (Naive)
# ==============================================================================
cat > /tmp/test_openmp_naive.cpp << 'EOF'
#include <iostream>
#include <vector>
#include <chrono>
#include <cstring>
#include <omp.h>
#include <cstdlib>

using namespace std::chrono;

void matrix_multiply_seq(float *A, float *B, float *C, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            float sum = 0.0f;
            for (int k = 0; k < N; k++) {
                sum += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
        }
    }
}

void matrix_multiply_omp(float *A, float *B, float *C, int N) {
    #pragma omp parallel for collapse(2) if(N * N > 10000)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            float sum = 0.0f;
            for (int k = 0; k < N; k++) {
                sum += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
        }
    }
}

int main(int argc, char* argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    int threads = (argc > 2) ? atoi(argv[2]) : 1;
    int reps = (argc > 3) ? atoi(argv[3]) : 3;
    
    omp_set_num_threads(threads);
    
    float *A = new float[N*N];
    float *B = new float[N*N];
    float *C = new float[N*N];
    
    for (int i = 0; i < N*N; i++) {
        A[i] = 0.001f * (i % 10);
        B[i] = 0.001f * ((i*7) % 10);
    }
    
    memset(C, 0, N*N*sizeof(float));
    if (threads == 1)
        matrix_multiply_seq(A, B, C, N);
    else
        matrix_multiply_omp(A, B, C, N);
    
    double min_time_ms = 1e9;
    for (int rep = 0; rep < reps; rep++) {
        memset(C, 0, N*N*sizeof(float));
        auto start = high_resolution_clock::now();
        if (threads == 1)
            matrix_multiply_seq(A, B, C, N);
        else
            matrix_multiply_omp(A, B, C, N);
        auto end = high_resolution_clock::now();
        double time_ms = duration<double, std::milli>(end - start).count();
        min_time_ms = std::min(min_time_ms, time_ms);
    }
    
    printf("%d,%d,%.6f\n", N, threads, min_time_ms);
    
    delete[] A;
    delete[] B;
    delete[] C;
    return 0;
}
EOF

# ==============================================================================
# TEST 2: OpenMP with Transpose
# ==============================================================================
cat > /tmp/test_openmp_transpose.cpp << 'EOF'
#include <iostream>
#include <vector>
#include <chrono>
#include <cstring>
#include <omp.h>
#include <cstdlib>

using namespace std::chrono;

void matrix_transpose(float *A, float *AT, int N) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            AT[j*N + i] = A[i*N + j];
        }
    }
}

void matrix_multiply_seq_transposed(float *A, float *BT, float *C, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            float sum = 0.0f;
            for (int k = 0; k < N; k++) {
                sum += A[i * N + k] * BT[j * N + k];
            }
            C[i * N + j] = sum;
        }
    }
}

void matrix_multiply_omp_transposed(float *A, float *BT, float *C, int N) {
    #pragma omp parallel for collapse(2) if(N * N > 10000)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            float sum = 0.0f;
            for (int k = 0; k < N; k++) {
                sum += A[i * N + k] * BT[j * N + k];
            }
            C[i * N + j] = sum;
        }
    }
}

int main(int argc, char* argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    int threads = (argc > 2) ? atoi(argv[2]) : 1;
    int reps = (argc > 3) ? atoi(argv[3]) : 3;
    
    omp_set_num_threads(threads);
    
    float *A = new float[N*N];
    float *B = new float[N*N];
    float *BT = new float[N*N];
    float *C = new float[N*N];
    
    for (int i = 0; i < N*N; i++) {
        A[i] = 0.001f * (i % 10);
        B[i] = 0.001f * ((i*7) % 10);
    }
    
    matrix_transpose(B, BT, N);
    
    memset(C, 0, N*N*sizeof(float));
    if (threads == 1)
        matrix_multiply_seq_transposed(A, BT, C, N);
    else
        matrix_multiply_omp_transposed(A, BT, C, N);
    
    double min_time_ms = 1e9;
    for (int rep = 0; rep < reps; rep++) {
        memset(C, 0, N*N*sizeof(float));
        auto start = high_resolution_clock::now();
        if (threads == 1)
            matrix_multiply_seq_transposed(A, BT, C, N);
        else
            matrix_multiply_omp_transposed(A, BT, C, N);
        auto end = high_resolution_clock::now();
        double time_ms = duration<double, std::milli>(end - start).count();
        min_time_ms = std::min(min_time_ms, time_ms);
    }
    
    printf("%d,%d,%.6f\n", N, threads, min_time_ms);
    
    delete[] A;
    delete[] B;
    delete[] BT;
    delete[] C;
    return 0;
}
EOF

echo "✓ Test source files created"
echo ""

# ==============================================================================
# Compile tests
# ==============================================================================
echo "Compiling OpenMP tests..."
g++ -O3 -march=native -fopenmp -std=c++17 /tmp/test_openmp_naive.cpp -o /tmp/test_openmp_naive
g++ -O3 -march=native -fopenmp -std=c++17 /tmp/test_openmp_transpose.cpp -o /tmp/test_openmp_transpose
echo "✓ Compilation successful"
echo ""

# ==============================================================================
# TEST 1: OpenMP Naive Results
# ==============================================================================
echo "=========================================="
echo "TEST 1: OpenMP (NAIVE) Speedup Measurements"
echo "=========================================="
echo ""
echo "Size,1T(ms),4T(ms),8T(ms),16T(ms),Speedup@16T"

for size in 300 500 1000 2000; do
    result_1t=$(/tmp/test_openmp_naive $size 1 3 2>/dev/null | cut -d',' -f3)
    result_4t=$(/tmp/test_openmp_naive $size 4 3 2>/dev/null | cut -d',' -f3)
    result_8t=$(/tmp/test_openmp_naive $size 8 3 2>/dev/null | cut -d',' -f3)
    result_16t=$(/tmp/test_openmp_naive $size 16 3 2>/dev/null | cut -d',' -f3)
    
    speedup=$(echo "scale=2; $result_1t / $result_16t" | bc 2>/dev/null || echo "ERR")
    
    echo "$size,$result_1t,$result_4t,$result_8t,$result_16t,$speedup"
done
echo ""

# ==============================================================================
# TEST 2: OpenMP with Transpose
# ==============================================================================
echo "=========================================="
echo "TEST 2: OpenMP (TRANSPOSE) Speedup Measurements"
echo "=========================================="
echo ""
echo "Size,1T(ms),4T(ms),8T(ms),16T(ms),Speedup@16T"

for size in 300 500 1000 2000; do
    result_1t=$(/tmp/test_openmp_transpose $size 1 3 2>/dev/null | cut -d',' -f3)
    result_4t=$(/tmp/test_openmp_transpose $size 4 3 2>/dev/null | cut -d',' -f3)
    result_8t=$(/tmp/test_openmp_transpose $size 8 3 2>/dev/null | cut -d',' -f3)
    result_16t=$(/tmp/test_openmp_transpose $size 16 3 2>/dev/null | cut -d',' -f3)
    
    speedup=$(echo "scale=2; $result_1t / $result_16t" | bc 2>/dev/null || echo "ERR")
    
    echo "$size,$result_1t,$result_4t,$result_8t,$result_16t,$speedup"
done
echo ""

# ==============================================================================
# TEST 3: GPU Check
# ==============================================================================
echo "=========================================="
echo "TEST 3: GPU Information"
echo "=========================================="
echo ""
nvidia-smi --query-gpu=index,name,memory.total,compute_cap --format=csv,noheader
echo ""

echo "=========================================="
echo "TESTS COMPLETED"
echo "=========================================="
echo ""
echo "Results saved to:"
echo "  - Output: $SLURM_STDOUT"
echo "  - Errors: $SLURM_STDERR"
echo ""
