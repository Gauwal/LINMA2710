#!/bin/bash
#SBATCH --partition=gpu
#SBATCH --gres=gpu:1
#SBATCH --time=01:00:00
#SBATCH --mem-per-cpu=2000
#SBATCH --job-name=linma2710_bench
#SBATCH --output=benchmark_%j.log
#SBATCH --error=benchmark_%j.err

module load CUDA
module load Python

cd $SLURM_SUBMIT_DIR

echo "Node: $(hostname)"
echo "GPU Info:"
nvidia-smi

echo ""
echo "Starting benchmarks..."
bash run_all_benchmarks.sh

echo ""
echo "Job completed."
