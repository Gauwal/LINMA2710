#include "distributed_matrix.hpp"
#include <stdexcept>
#include <algorithm>

DistributedMatrix::DistributedMatrix(const Matrix& matrix, int numProcs)
    : globalRows(matrix.numRows()),
      globalCols(matrix.numCols()),
      localCols(0),
      startCol(0),
      numProcesses(numProcs),
      rank(0),
      localData(matrix.numRows(), 1)
{
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int base = globalCols / numProcs;
    int rem = globalCols % numProcs;
    
    localCols = base + (rank < rem ? 1 : 0);
    startCol = (rank < rem) ? rank * (base + 1) : rem * (base + 1) + (rank - rem) * base;
    
    localData = Matrix(globalRows, localCols);
    
    for (int i = 0; i < globalRows; ++i) {
        for (int j = 0; j < localCols; ++j) {
            localData.set(i, j, matrix.get(i, startCol + j));
        }
    }
}

DistributedMatrix::DistributedMatrix(const DistributedMatrix& other)
    : globalRows(other.globalRows),
      globalCols(other.globalCols),
      localCols(other.localCols),
      startCol(other.startCol),
      numProcesses(other.numProcesses),
      rank(other.rank),
      localData(other.localData)
{
}

int DistributedMatrix::numRows() const { return globalRows; }
int DistributedMatrix::numCols() const { return globalCols; }
const Matrix& DistributedMatrix::getLocalData() const { return localData; }

double DistributedMatrix::get(int i, int j) const {
    int localJ = j - startCol; if(localJ >= 0 && localJ < localCols) return localData.get(i, localJ); throw std::out_of_range("Col not on proc");
}

void DistributedMatrix::set(int i, int j, double value) {
    int localJ = j - startCol; if(localJ >= 0 && localJ < localCols) { localData.set(i, localJ, value); return; } throw std::out_of_range("Col not on proc");
}

int DistributedMatrix::globalColIndex(int localColIdx) const {
    return startCol + localColIdx;
}

int DistributedMatrix::localColIndex(int globalColIdx) const {
    return globalColIdx - startCol;
}

int DistributedMatrix::ownerProcess(int globalColIdx) const {
    int base = globalCols / numProcesses;
    int rem = globalCols % numProcesses;
    int threshold = rem * (base + 1);
    
    if (globalColIdx < threshold) {
        return globalColIdx / (base + 1);
    } else {
        return rem + (globalColIdx - threshold) / base;
    }
}

void DistributedMatrix::fill(double value) {
    localData.fill(value);
}

DistributedMatrix DistributedMatrix::operator+(const DistributedMatrix& other) const {
    DistributedMatrix result(*this);
    result.localData = this->localData + other.localData;
    return result;
}

DistributedMatrix DistributedMatrix::operator-(const DistributedMatrix& other) const {
    DistributedMatrix result(*this);
    result.localData = this->localData - other.localData;
    return result;
}

DistributedMatrix DistributedMatrix::operator*(double scalar) const {
    DistributedMatrix result(*this);
    result.localData = this->localData * scalar;
    return result;
}

Matrix DistributedMatrix::transpose() const {
    return gather().transpose();
}

void DistributedMatrix::sub_mul(double scalar, const DistributedMatrix& other) {
    this->localData.sub_mul(scalar, other.localData);
}

DistributedMatrix DistributedMatrix::apply(const std::function<double(double)>& func) const {
    DistributedMatrix result(*this);
    result.localData = this->localData.apply(func);
    return result;
}

DistributedMatrix DistributedMatrix::applyBinary(
    const DistributedMatrix& a,
    const DistributedMatrix& b,
    const std::function<double(double, double)>& func)
{
    DistributedMatrix result(a);
    for (int i = 0; i < result.localData.numRows(); ++i) {
        for (int j = 0; j < result.localData.numCols(); ++j) {
            result.localData.set(i, j, func(a.localData.get(i, j), b.localData.get(i, j)));
        }
    }
    return result;
}

DistributedMatrix multiply(const Matrix& left, const DistributedMatrix& right) {
    DistributedMatrix result(right);
    result.globalRows = left.numRows();
    result.localData = left * right.localData;
    return result;
}

Matrix DistributedMatrix::multiplyTransposed(const DistributedMatrix& other) const {
    Matrix localProduct = this->localData * other.localData.transpose();
    Matrix globalProduct(globalRows, other.globalRows);
    
    std::vector<double> sendBuf(globalRows * other.globalRows);
    std::vector<double> recvBuf(globalRows * other.globalRows);
    
    for (int i = 0; i < globalRows; ++i) {
        for (int j = 0; j < other.globalRows; ++j) {
            sendBuf[i * other.globalRows + j] = localProduct.get(i, j);
        }
    }
    
    MPI_Allreduce(sendBuf.data(), recvBuf.data(), globalRows * other.globalRows, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    
    for (int i = 0; i < globalRows; ++i) {
        for (int j = 0; j < other.globalRows; ++j) {
            globalProduct.set(i, j, recvBuf[i * other.globalRows + j]);
        }
    }
    
    return globalProduct;
}

double DistributedMatrix::sum() const {
    double localSum = 0.0;
    for (int i = 0; i < localData.numRows(); ++i) {
        for (int j = 0; j < localData.numCols(); ++j) {
            localSum += localData.get(i, j);
        }
    }
    
    double globalSum = 0.0;
    MPI_Allreduce(&localSum, &globalSum, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    return globalSum;
}

Matrix DistributedMatrix::gather() const {
    std::vector<int> counts(numProcesses);
    std::vector<int> displs(numProcesses);
    
    int base = globalCols / numProcesses;
    int rem = globalCols % numProcesses;

    for (int p = 0; p < numProcesses; ++p) {
        counts[p] = (base + (p < rem ? 1 : 0)) * globalRows;
    }

    displs[0] = 0;
    for (int p = 1; p < numProcesses; ++p) {
        displs[p] = displs[p - 1] + counts[p - 1];
    }
    
    std::vector<double> sendBuf(localCols * globalRows);
    int idx = 0;
    for (int j = 0; j < localCols; ++j) {
        for (int i = 0; i < globalRows; ++i) {
            sendBuf[idx++] = localData.get(i, j);
        }
    }
    
    std::vector<double> recvBuf(globalCols * globalRows);
    MPI_Allgatherv(sendBuf.data(), localCols * globalRows, MPI_DOUBLE, 
                   recvBuf.data(), counts.data(), displs.data(), MPI_DOUBLE, MPI_COMM_WORLD);
                   
    Matrix result(globalRows, globalCols);
    idx = 0;
    for (int p = 0; p < numProcesses; ++p) {
        int processCols = base + (p < rem ? 1 : 0);
        int startGlobalCol = (p < rem) ? p * (base + 1) : rem * (base + 1) + (p - rem) * base;
        
        for (int j = 0; j < processCols; ++j) {
            for (int i = 0; i < globalRows; ++i) {
                result.set(i, startGlobalCol + j, recvBuf[idx++]);
            }
        }
    }
    
    return result;
}

void sync_matrix(Matrix *matrix, int rank, int src) {
    int dims[2];
    if (rank == src) {
        dims[0] = matrix->numRows();
        dims[1] = matrix->numCols();
    }
    MPI_Bcast(dims, 2, MPI_INT, src, MPI_COMM_WORLD);
    
    if (rank != src) {
        *matrix = Matrix(dims[0], dims[1]);
    }
    
    std::vector<double> buf(dims[0] * dims[1]);
    if (rank == src) {
        int idx = 0;
        for (int i = 0; i < dims[0]; ++i) {
            for (int j = 0; j < dims[1]; ++j) {
                buf[idx++] = matrix->get(i, j);
            }
        }
    }
    
    MPI_Bcast(buf.data(), dims[0] * dims[1], MPI_DOUBLE, src, MPI_COMM_WORLD);
    
    if (rank != src) {
        int idx = 0;
        for (int i = 0; i < dims[0]; ++i) {
            for (int j = 0; j < dims[1]; ++j) {
                matrix->set(i, j, buf[idx++]);
            }
        }
    }
}
