#include "distributed_matrix.hpp"
#include <stdexcept>
#include <algorithm>
#include <cmath>

// The matrix is split by columns across MPI processes.
// Each process stores a local Matrix with a subset of columns.
// Columns are distributed as evenly as possible.

DistributedMatrix::DistributedMatrix(const Matrix& matrix, int numProcs)
    : globalRows(matrix.numRows()),       // Store the total number of rows (same as original matrix)
      globalCols(matrix.numCols()),       // Store the total number of columns in the global scope
      localCols(0),                       // Will hold the exact number of columns assigned to this MPI rank
      startCol(0),                        // Global index identifying the start of this rank's partition
      numProcesses(numProcs),             // The total number of execution nodes/processes
      rank(0),                            // Identifies the current node executing this code
      localData(matrix.numRows(), 1)      // Placeholder allocation; will be recreated with proper size shortly
{
    // Retrieve the unique ID (rank) of the current MPI process within the global communicator group
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Compute integer division to find the minimum number of columns each process will receive.
    int baseCols = globalCols / numProcs;
    // Compute the leftover columns that couldn't be evenly divided.
    int remainder = globalCols % numProcs;
    
    // Load Balancing Strategy:
    // Processes (ranks) with an ID less than 'remainder' will absorb exactly 1 extra column.
    // E.g., if we have 10 cols over 3 processes -> Remainder is 1. Rank 0 gets 4. Rank 1,2 get 3.
    localCols = (rank < remainder) ? (baseCols + 1) : baseCols;
    
    // Determine where this process's column ownership starts in the global mathematical sense.
    if (rank < remainder) {
        // If we are in the "heavy" group, every previous rank has (baseCols + 1) columns.
        startCol = rank * (baseCols + 1);
    } else {
        // If we are in the "normal" group, we must account for 'remainder' ranks having 1 extra column,
        // plus the remaining ranks before us having 'baseCols' columns.
        startCol = remainder * (baseCols + 1) + (rank - remainder) * baseCols;
    }
    
    // Allocate the local chunk memory (using the standard Matrix class).
    localData = Matrix(globalRows, localCols);
    
    // Fill the process-local chunk by reading out the assigned column slice from the initialized global matrix.
    for (int i = 0; i < globalRows; ++i) {
        for (int j = 0; j < localCols; ++j) {
            // (startCol + j) translates back to the real global column index
            localData.set(i, j, matrix.get(i, startCol + j));
        }
    }
}

DistributedMatrix::DistributedMatrix(const DistributedMatrix& other)
    : globalRows(other.globalRows),         // Copy structural sizes
      globalCols(other.globalCols),
      localCols(other.localCols),           // Copy MPI partition dimensions
      startCol(other.startCol),
      numProcesses(other.numProcesses),
      rank(other.rank),                     // Same rank execution context
      localData(other.localData)            // Use the standard Matrix copy constructor internally
{
}

// Simple accessors for structural properties
int DistributedMatrix::numRows() const { return globalRows; }
int DistributedMatrix::numCols() const { return globalCols; }
const Matrix& DistributedMatrix::getLocalData() const { return localData; }

// --- Memory Access operations ---

double DistributedMatrix::get(int i, int j) const
{
    // Check if global column `j` falls natively within this MPI process's block
    int localJ = localColIndex(j);
    // If it maps between [0, localCols[, then this process has the authority to return it
    if (localJ >= 0 && localJ < localCols) {
        return localData.get(i, localJ);
    }
    // According to tests, accessing unassigned columns should abort explicitly
    throw std::out_of_range("Col is not on this process");
}

void DistributedMatrix::set(int i, int j, double value)
{
    // Map to a local offset
    int localJ = localColIndex(j);
    if (localJ >= 0 && localJ < localCols) {
        // Safe to modify local memory segment 
        localData.set(i, localJ, value);
    } else {
        // Reject writes to memory supposedly ruled by other nodes
        throw std::out_of_range("Col is not on this process");
    }
}

// Convert an array offset existing on this rank into its abstract global position
int DistributedMatrix::globalColIndex(int localColIdx) const
{
    return startCol + localColIdx;
}

// Calculate the relative coordinate of a target global column assuming THIS rank holds it
int DistributedMatrix::localColIndex(int globalColIdx) const
{
    return globalColIdx - startCol; 
}

// Logic algorithm determining which process controls a specific mathematical column
int DistributedMatrix::ownerProcess(int globalColIdx) const
{
    int baseCols = globalCols / numProcesses;
    int remainder = globalCols % numProcesses;
    
    // The threshold determines where exactly the shift from "heavy" processes (baseCols+1)
    // to "normal" processes (baseCols) occurs internally
    int threshold = remainder * (baseCols + 1);
    
    if (globalColIdx < threshold) {
        // Determine the ID within the heavy block
        return globalColIdx / (baseCols + 1);
    } else {
        // Offset beyond threshold mapped evenly to normal-size block nodes
        return remainder + (globalColIdx - threshold) / baseCols;
    }
}

// Element-wise operations taking advantage of the identical distribution layout
void DistributedMatrix::fill(double value)
{
    // Fully parallel: Every rank immediately loops over its memory
    localData.fill(value);
}

DistributedMatrix DistributedMatrix::operator+(const DistributedMatrix& other) const
{
    // Because both `this` and `other` use uniform block-column spacing,
    // their local variables conceptually align perfectly without network sharing!
    DistributedMatrix result(*this);
    result.localData = this->localData + other.localData;
    return result;
}

DistributedMatrix DistributedMatrix::operator-(const DistributedMatrix& other) const
{
    DistributedMatrix result(*this);
    result.localData = this->localData - other.localData; // Purely local CPU execution
    return result;
}

DistributedMatrix DistributedMatrix::operator*(double scalar) const
{
    DistributedMatrix result(*this);
    result.localData = this->localData * scalar;
    return result;
}

Matrix DistributedMatrix::transpose() const
{
    // A transposed distributed matrix would flip (split-by-rows).
    // Instead we collapse the object into an unfragmented Matrix layout first.
    Matrix fullMatrix = gather();
    
    // Defer down to native standard Matrix algorithm.
    return fullMatrix.transpose();
}

void DistributedMatrix::sub_mul(double scalar, const DistributedMatrix& other)
{
    // Directly mutate local array
    this->localData.sub_mul(scalar, other.localData);
}

DistributedMatrix DistributedMatrix::apply(const std::function<double(double)>& func) const
{
    DistributedMatrix result(*this);
    result.localData = this->localData.apply(func); // Relies on standard library mapping
    return result;
}

DistributedMatrix DistributedMatrix::applyBinary(
    const DistributedMatrix& a,
    const DistributedMatrix& b,
    const std::function<double(double, double)>& func)
{
    DistributedMatrix result(a); // Copies distribution layout safely
    // Fallback nested loop across all owned elements using function handler 
    for (int i = 0; i < result.localData.numRows(); ++i) {
        for (int j = 0; j < result.localData.numCols(); ++j) {
            result.localData.set(i, j, func(a.localData.get(i, j), b.localData.get(i, j)));
        }
    }
    return result;
}

// Single node layout left `*` DistLayout right
DistributedMatrix multiply(const Matrix& left, const DistributedMatrix& right)
{
    // The formula says C = A * B
    // A represents `left` entirely contained on all nodes
    // B represents `right` fragmented into vectors
    // To compute a single column of C, we only need A and the identical column inside B.
    // Hence we perform calculation concurrently, producing purely localized matrices.
    DistributedMatrix result(right); 
    result.globalRows = left.numRows(); // Because A matrix defines the new ROW count
    result.localData = left * right.localData; // Uses fully localized multiplication
    return result;
}

// DistLayout `*` DistLayout^T (Gradient accumulations)
Matrix DistributedMatrix::multiplyTransposed(const DistributedMatrix& other) const
{
    // Given `this` as A = [A0 | A1 | A2] and `other` as B = [B0 | B1 | B2]
    // The transposition calculates A * B^T
    // Equation property dictates that A*B^T == sum_i( A_i * B_i^T )
    
    // 1. Partial matrix multiplication happens completely detached inside each node
    Matrix localProduct = this->localData * other.localData.transpose();
    
    // 2. Prepare MPI Buffers representing the full output resolution dimensions
    Matrix globalProduct(globalRows, other.globalRows);
    
    std::vector<double> sendBuf(globalRows * other.globalRows);
    std::vector<double> recvBuf(globalRows * other.globalRows);
    
    // Move 2D objects out into flat Contiguous structures for reliable binary transmission
    for (int i = 0; i < globalRows; ++i) {
        for (int j = 0; j < other.globalRows; ++j) {
            sendBuf[i * other.globalRows + j] = localProduct.get(i, j);
        }
    }
    
    // 3. SYNCHRONIZATION POINT
    // Every single process broadcasts their slice arrays outwards AND inwards globally
    // utilizing MPI operation `MPI_SUM` over floats (`MPI_DOUBLE`).
    MPI_Allreduce(sendBuf.data(), recvBuf.data(), globalRows * other.globalRows, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    
    // Explode sum flattened output map back into Standard output
    for (int i = 0; i < globalRows; ++i) {
        for (int j = 0; j < other.globalRows; ++j) {
            globalProduct.set(i, j, recvBuf[i * other.globalRows + j]);
        }
    }
    
    return globalProduct; // Identical object duplicated implicitly across all nodes
}

double DistributedMatrix::sum() const
{
    // Phase 1: Independent summing iteration isolating node's own footprint mapping
    double localSum = 0.0;
    for (int i = 0; i < localData.numRows(); ++i) {
        for (int j = 0; j < localData.numCols(); ++j) {
            localSum += localData.get(i, j);
        }
    }
    
    // Phase 2: Inter-Process sync combining singular scalar into overall volume count
    double globalSum = 0.0;
    MPI_Allreduce(&localSum, &globalSum, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    
    return globalSum;
}

// Convert from distributed MPI logic into a complete centralized Standard Layout
Matrix DistributedMatrix::gather() const
{
    // We cannot use standard MPI_Allgather, because column quantity varies across nodes
    // Node zero might have 4, Node one might have 3.
    
    // Step 1: Tell everyone simultaneously how many exact columns I personally hold 
    std::vector<int> colCounts(numProcesses);
    int cols = localCols;
    MPI_Allgather(&cols, 1, MPI_INT, colCounts.data(), 1, MPI_INT, MPI_COMM_WORLD);
    
    std::vector<int> recvCounts(numProcesses);      // Represents raw Array Length elements per node
    std::vector<int> displacements(numProcesses);   // Pointer Start Index mapped to a big flat sequence
    
    int currentDisp = 0;
    for (int p = 0; p < numProcesses; ++p) {
        recvCounts[p] = colCounts[p] * globalRows;  // Num Elements = N_cols_on_P * Global_Rows
        displacements[p] = currentDisp;             // Track the offset 
        currentDisp += recvCounts[p];               // Push offset iterator
    }
    
    std::vector<double> sendBuf(localCols * globalRows);
    
    // Step 2: Encode multi-d matrix block column by column (because columns stay contiguous together)
    int idx = 0;
    for (int j = 0; j < localCols; ++j) {
        for (int i = 0; i < globalRows; ++i) {
            sendBuf[idx++] = localData.get(i, j);
        }
    }
    
    // Step 3: Run GATHER-VARIABLE network instruction 
    std::vector<double> recvBuf(globalCols * globalRows);
    MPI_Allgatherv(sendBuf.data(), localCols * globalRows, MPI_DOUBLE, 
                   recvBuf.data(), recvCounts.data(), displacements.data(), MPI_DOUBLE, MPI_COMM_WORLD);
                   
    // Step 4: Deconstruct contiguous result arrays down onto mapped layout properly organized
    Matrix result(globalRows, globalCols);
    idx = 0;
    for (int p = 0; p < numProcesses; ++p) {
        int processCols = colCounts[p];
        int startGlobalCol = 0; // Where on earth does Process P's column window begin?
        for(int k=0; k<p; ++k) startGlobalCol += colCounts[k];
        
        for (int j = 0; j < processCols; ++j) {
            for (int i = 0; i < globalRows; ++i) {
                // Restore into actual structural place
                result.set(i, startGlobalCol + j, recvBuf[idx++]);
            }
        }
    }
    
    return result;
}

// Takes a Standard standalone Matrix constructed upon the 'SRC' root node
// and overwrites identical copies down into all network peers.
void sync_matrix(Matrix *matrix, int rank, int src)
{
    int dims[2];
    if (rank == src) {
        // Root declares size structure properties
        dims[0] = matrix->numRows();
        dims[1] = matrix->numCols();
    }
    // Broadcast dimensional identifiers to all generic ranks
    MPI_Bcast(dims, 2, MPI_INT, src, MPI_COMM_WORLD);
    
    // Reconstruct recipient placeholders against those exact sizes
    if (rank != src) {
        *matrix = Matrix(dims[0], dims[1]);
    }
    
    // Load up 1-Dimensional stream representations
    std::vector<double> buf(dims[0] * dims[1]);
    if (rank == src) {
        int idx = 0;
        for (int i = 0; i < dims[0]; ++i) {
            for (int j = 0; j < dims[1]; ++j) {
                buf[idx++] = matrix->get(i, j);
            }
        }
    }
    
    // Data transmission payload!
    MPI_Bcast(buf.data(), dims[0] * dims[1], MPI_DOUBLE, src, MPI_COMM_WORLD);
    
    // Translate flat representations upon receivers backwards onto Standard memory objects
    if (rank != src) {
        int idx = 0;
        for (int i = 0; i < dims[0]; ++i) {
            for (int j = 0; j < dims[1]; ++j) {
                matrix->set(i, j, buf[idx++]);
            }
        }
    }
}
