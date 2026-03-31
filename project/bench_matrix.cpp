#include <iostream>
#include <chrono>
#include <vector>
#include "matrix.hpp"
#ifdef _OPENMP
#include <omp.h>
#endif

void run_benchmark() {
    std::cout << "--- Matrix Multiplication Benchmark ---" << std::endl;
    std::vector<int> sizes = {128, 512, 1024};
    std::vector<int> threads = {1, 2, 4, 8};

    for (int size : sizes) {
        Matrix A(size, size);
        Matrix B(size, size);
        A.fill(1.5);
        B.fill(2.0);

        for (int t : threads) {
#ifdef _OPENMP
            omp_set_num_threads(t);
#endif
            auto start = std::chrono::high_resolution_clock::now();
            
            Matrix C = A * B;
            
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;
            
            std::cout << "Size: " << size << "x" << size 
                      << " | Threads: " << t 
                      << " | Time: " << duration.count() << " ms" << std::endl;
        }
        std::cout << "---------------------------------------" << std::endl;
    }
}

int main() {
    run_benchmark();
    return 0;
}
