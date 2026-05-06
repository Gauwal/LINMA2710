CXX = g++
MPICXX = mpic++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3
INCLUDE = -I../project/include

OPENMP_FLAGS ?= -fopenmp

all: bench_openmp bench_mpi bench_opencl

# --- Part 1 & 2: OpenMP Benchmark ---
bench_openmp: bench_openmp.cpp ../project/src/matrix.cpp
	$(CXX) $(CXXFLAGS) $(OPENMP_FLAGS) $(INCLUDE) -o bench_openmp bench_openmp.cpp ../project/src/matrix.cpp

run_openmp: bench_openmp
	./bench_openmp

# --- Part 3: MPI Benchmark ---
bench_mpi: bench_mpi.cpp ../project/src/distributed_matrix.cpp ../project/src/matrix.cpp
	$(MPICXX) $(CXXFLAGS) $(INCLUDE) -o bench_mpi bench_mpi.cpp ../project/src/distributed_matrix.cpp ../project/src/matrix.cpp

run_mpi: bench_mpi
	mpirun -np 4 ./bench_mpi

# --- Part 4: OpenCL Benchmark ---
bench_opencl: bench_opencl.cpp ../project/src/matrix_opencl.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o bench_opencl bench_opencl.cpp ../project/src/matrix_opencl.cpp -lOpenCL

run_opencl: bench_opencl
	./bench_opencl

# --- Run all benchmarks ---
run_all: run_openmp run_mpi run_opencl
	python3 plot_results.py

# --- Cleanup ---
clean:
	rm -f bench_openmp bench_mpi bench_opencl *.csv

.PHONY: all run_openmp run_mpi run_opencl run_all clean
