#!/bin/bash
#SBATCH --job-name=test_opencl
#SBATCH --ntasks=1
#SBATCH --time=10:00
#SBATCH --mem-per-cpu=2000
#SBATCH --partition=gpu
#SBATCH --gres=gpu:1
#SBATCH --output=test_opencl.out
#SBATCH --error=test_opencl.err

echo "=========================================="
echo "Testing OpenCL Matrix Operations"
echo "=========================================="
echo "Node: $(hostname)"
echo "GPU Device:"
clinfo 2>/dev/null | head -20 || echo "(clinfo not available)"
echo ""

# Load necessary modules
module load CUDA 2>/dev/null || echo "CUDA module not available (may not be needed)"

# Build the project
echo "Building test_opencl..."
cd /home/ucl/ingi/gsavary/LINMA2710/project
make clean
make test_opencl 2>&1

if [ ! -f test_opencl ]; then
    echo "Build failed!"
    exit 1
fi

echo ""
echo "=========================================="
echo "Running test_opencl..."
echo "=========================================="
./test_opencl 2>&1

echo ""
echo "=========================================="
echo "Test completed"
echo "=========================================="
