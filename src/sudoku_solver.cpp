#include "sudoku_solver.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <omp.h>

// BitMaskState implementation
BitMaskState::BitMaskState(int N) {
    rowMask.resize(N, 0);
    colMask.resize(N, 0);
    blockMask.resize(N, 0);
}

void BitMaskState::set(int N, int blockSize, int row, int col, int value) {
    // Validate value to prevent overflow (max value for uint32_t is 31)
    if (value < 1 || value > 31) {
        return;  // Invalid value, skip
    }
    int blockIdx = (row / blockSize) * blockSize + (col / blockSize);
    rowMask[row] |= (1u << value);
    colMask[col] |= (1u << value);
    blockMask[blockIdx] |= (1u << value);
}

void BitMaskState::unset(int N, int blockSize, int row, int col, int value) {
    // Validate value to prevent overflow
    if (value < 1 || value > 31) {
        return;  // Invalid value, skip
    }
    int blockIdx = (row / blockSize) * blockSize + (col / blockSize);
    rowMask[row] &= ~(1u << value);
    colMask[col] &= ~(1u << value);
    blockMask[blockIdx] &= ~(1u << value);
}

bool BitMaskState::canPlace(int N, int blockSize, int row, int col, int value) const {
    // Validate value to prevent overflow
    if (value < 1 || value > 31) {
        return false;  // Invalid value
    }
    int blockIdx = (row / blockSize) * blockSize + (col / blockSize);
    uint32_t mask = (1u << value);
    return !(rowMask[row] & mask) && !(colMask[col] & mask) && !(blockMask[blockIdx] & mask);
}

// Constructor
SudokuSolver::SudokuSolver(int N) : N(N), numSolutions(0), runningTime(0.0) {
    blockSize = static_cast<int>(sqrt(N));
    
    // Validate that N is a perfect square
    if (blockSize * blockSize != N) {
        std::cerr << "Warning: N=" << N << " is not a perfect square. "
                  << "Sudoku constraints may not work correctly." << std::endl;
    }
    
    // Validate that N is within bitmask range (uint32_t supports up to 31)
    if (N > 31) {
        std::cerr << "Error: N=" << N << " exceeds maximum supported size of 31 "
                  << "for bitmask optimization. Please use N <= 25." << std::endl;
    }
    
    board.resize(N * N, 0);
}

// Load board from vector
void SudokuSolver::loadBoard(const std::vector<int>& boardData) {
    if (boardData.size() != static_cast<size_t>(N * N)) {
        std::cerr << "Error: Board size mismatch!" << std::endl;
        return;
    }
    board = boardData;
}

// Helper method to get flat index
int SudokuSolver::getIndex(int row, int col) const {
    return row * N + col;
}

// Check if value exists in row
bool SudokuSolver::isInRow(int row, int value) const {
    for (int col = 0; col < N; ++col) {
        if (board[getIndex(row, col)] == value) {
            return true;
        }
    }
    return false;
}

// Check if value exists in column
bool SudokuSolver::isInCol(int col, int value) const {
    for (int row = 0; row < N; ++row) {
        if (board[getIndex(row, col)] == value) {
            return true;
        }
    }
    return false;
}

// Check if value exists in block
bool SudokuSolver::isInBlock(int row, int col, int value) const {
    int blockRow = (row / blockSize) * blockSize;
    int blockCol = (col / blockSize) * blockSize;
    
    for (int r = blockRow; r < blockRow + blockSize; ++r) {
        for (int c = blockCol; c < blockCol + blockSize; ++c) {
            if (board[getIndex(r, c)] == value) {
                return true;
            }
        }
    }
    return false;
}

// Check if placing value at (row, col) is valid
bool SudokuSolver::isValid(int row, int col, int value) const {
    return !isInRow(row, value) && !isInCol(col, value) && !isInBlock(row, col, value);
}

// Check if placing value at (row, col) is valid with a specific board
bool SudokuSolver::isValidWithBoard(const std::vector<int>& boardRef, int row, int col, int value) const {
    // Check row
    for (int c = 0; c < N; ++c) {
        if (boardRef[row * N + c] == value) {
            return false;
        }
    }
    
    // Check column
    for (int r = 0; r < N; ++r) {
        if (boardRef[r * N + col] == value) {
            return false;
        }
    }
    
    // Check block
    int blockRow = (row / blockSize) * blockSize;
    int blockCol = (col / blockSize) * blockSize;
    for (int r = blockRow; r < blockRow + blockSize; ++r) {
        for (int c = blockCol; c < blockCol + blockSize; ++c) {
            if (boardRef[r * N + c] == value) {
                return false;
            }
        }
    }
    
    return true;
}

// Find next empty cell
bool SudokuSolver::findNextEmptyCell(int& row, int& col) const {
    for (row = 0; row < N; ++row) {
        for (col = 0; col < N; ++col) {
            if (board[getIndex(row, col)] == 0) {
                return true;
            }
        }
    }
    return false;
}

// Get possible values for a cell
std::set<int> SudokuSolver::getPossibleValues(int row, int col) const {
    std::set<int> possibleValues;
    for (int value = 1; value <= N; ++value) {
        if (isValid(row, col, value)) {
            possibleValues.insert(value);
        }
    }
    return possibleValues;
}

// Backtracking algorithm for single thread
int SudokuSolver::backtrackSingleThread(int pos) {
    // If we've filled all cells, we found a solution
    if (pos == N * N) {
        return 1;
    }
    
    int row = pos / N;
    int col = pos % N;
    
    // Skip already filled cells
    if (board[getIndex(row, col)] != 0) {
        return backtrackSingleThread(pos + 1);
    }
    
    int count = 0;
    for (int value = 1; value <= N; ++value) {
        if (isValid(row, col, value)) {
            board[getIndex(row, col)] = value;
            count += backtrackSingleThread(pos + 1);
            board[getIndex(row, col)] = 0;  // Backtrack
        }
    }
    
    return count;
}

// Solve from a given state (used by parallel solver)
int SudokuSolver::solveFromState(const std::vector<int>& boardRef, int pos) {
    // If we've filled all cells, we found a solution
    if (pos == N * N) {
        return 1;
    }
    
    int row = pos / N;
    int col = pos % N;
    
    // Skip already filled cells
    if (boardRef[getIndex(row, col)] != 0) {
        return solveFromState(boardRef, pos + 1);
    }
    
    int count = 0;
    for (int value = 1; value <= N; ++value) {
        if (isValidWithBoard(boardRef, row, col, value)) {
            // Create a copy only when we need to modify
            // Note: This copy-per-recursion approach is necessary for thread safety
            // in the parallel solver, as each thread needs its own independent board state.
            // While memory-intensive, it ensures correctness in parallel execution.
            std::vector<int> boardCopy = boardRef;
            boardCopy[getIndex(row, col)] = value;
            count += solveFromState(boardCopy, pos + 1);
        }
    }
    
    return count;
}

// Single thread solving
void SudokuSolver::solveSingleThread() {
    auto start = std::chrono::high_resolution_clock::now();
    
    numSolutions = backtrackSingleThread(0);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    runningTime = duration.count();
}

// Parallel solving with OpenMP
void SudokuSolver::solveParallel(int numThreads) {
    auto start = std::chrono::high_resolution_clock::now();
    
    omp_set_num_threads(numThreads);
    
    // Find first empty cell
    int firstRow = -1, firstCol = -1;
    bool foundEmpty = false;
    
    for (int row = 0; row < N && !foundEmpty; ++row) {
        for (int col = 0; col < N && !foundEmpty; ++col) {
            if (board[getIndex(row, col)] == 0) {
                firstRow = row;
                firstCol = col;
                foundEmpty = true;
            }
        }
    }
    
    if (!foundEmpty) {
        // No empty cells, board is complete
        numSolutions = 1;
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        runningTime = duration.count();
        return;
    }
    
    // Get possible values for first empty cell
    std::set<int> possibleValues = getPossibleValues(firstRow, firstCol);
    std::vector<int> valuesList(possibleValues.begin(), possibleValues.end());
    
    int totalSolutions = 0;
    int numValues = static_cast<int>(valuesList.size());
    
    // Parallel loop over possible values
    #pragma omp parallel for reduction(+:totalSolutions)
    for (int i = 0; i < numValues; ++i) {
        int value = valuesList[i];
        
        // Create a copy of the board for this thread
        std::vector<int> boardCopy = board;
        boardCopy[getIndex(firstRow, firstCol)] = value;
        
        // Solve from this state
        int pos = getIndex(firstRow, firstCol) + 1;
        int solutions = solveFromState(boardCopy, pos);
        totalSolutions += solutions;
    }
    
    numSolutions = totalSolutions;
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    runningTime = duration.count();
}

// Query methods
int SudokuSolver::getNumSolutions() const {
    return numSolutions;
}

double SudokuSolver::getRunningTime() const {
    return runningTime;
}

int SudokuSolver::getSize() const {
    return N;
}

int SudokuSolver::getBlockSize() const {
    return blockSize;
}

// Print board
void SudokuSolver::printBoard() const {
    std::cout << "\n";
    for (int row = 0; row < N; ++row) {
        if (row > 0 && row % blockSize == 0) {
            for (int i = 0; i < N + blockSize - 1; ++i) {
                std::cout << "--";
            }
            std::cout << "\n";
        }
        
        for (int col = 0; col < N; ++col) {
            if (col > 0 && col % blockSize == 0) {
                std::cout << "| ";
            }
            
            int value = board[getIndex(row, col)];
            if (value == 0) {
                std::cout << ". ";
            } else {
                std::cout << value << " ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

// Optimized backtracking with bitmask for validation
int SudokuSolver::backtrackWithBitmask(std::vector<int>& boardRef, BitMaskState& state, int pos) {
    // If we've filled all cells, we found a solution
    if (pos == N * N) {
        return 1;
    }
    
    int row = pos / N;
    int col = pos % N;
    
    // Skip already filled cells
    if (boardRef[getIndex(row, col)] != 0) {
        return backtrackWithBitmask(boardRef, state, pos + 1);
    }
    
    int count = 0;
    for (int value = 1; value <= N; ++value) {
        if (state.canPlace(N, blockSize, row, col, value)) {
            boardRef[getIndex(row, col)] = value;
            state.set(N, blockSize, row, col, value);
            
            count += backtrackWithBitmask(boardRef, state, pos + 1);
            
            boardRef[getIndex(row, col)] = 0;
            state.unset(N, blockSize, row, col, value);
        }
    }
    
    return count;
}

// Solve a subproblem (used by optimized parallel solver)
int SudokuSolver::solveSubproblem(const Subproblem& subproblem) {
    // Create copies for thread safety - each thread needs independent state
    // Note: While this involves copying, it's necessary for parallel correctness
    // and only happens once per subproblem (not at every recursion level)
    std::vector<int> boardCopy = subproblem.board;
    BitMaskState stateCopy = subproblem.state;
    return backtrackWithBitmask(boardCopy, stateCopy, subproblem.startPos);
}

// Recursively generate subproblems by filling K empty cells
void SudokuSolver::generateSubproblemsRecursive(Subproblem& current, int depth, int maxDepth,
                                               std::vector<Subproblem>& results) {
    if (depth == maxDepth) {
        results.push_back(current);
        return;
    }
    
    // Find next empty cell
    int pos = current.startPos;
    while (pos < N * N && current.board[pos] != 0) {
        pos++;
    }
    
    if (pos >= N * N) {
        // No more empty cells, add current state
        results.push_back(current);
        return;
    }
    
    int row = pos / N;
    int col = pos % N;
    
    // Try all valid values for this cell
    bool foundAny = false;
    for (int value = 1; value <= N; ++value) {
        if (current.state.canPlace(N, blockSize, row, col, value)) {
            foundAny = true;
            
            // Create a new subproblem with this value
            Subproblem next = current;
            next.board[pos] = value;
            next.state.set(N, blockSize, row, col, value);
            next.startPos = pos + 1;
            
            generateSubproblemsRecursive(next, depth + 1, maxDepth, results);
        }
    }
    
    // If no valid values found, this branch is invalid
    if (!foundAny && depth < maxDepth) {
        // Dead end, don't add to results
        return;
    }
}

// Generate subproblems for parallel execution
void SudokuSolver::generateSubproblems(int partitionDepth, std::vector<Subproblem>& subproblems) {
    Subproblem initial(N);
    initial.board = board;
    initial.startPos = 0;
    
    // Initialize bitmask with existing values
    for (int row = 0; row < N; ++row) {
        for (int col = 0; col < N; ++col) {
            int value = board[getIndex(row, col)];
            if (value != 0) {
                initial.state.set(N, blockSize, row, col, value);
            }
        }
    }
    
    generateSubproblemsRecursive(initial, 0, partitionDepth, subproblems);
}

// Optimized parallel solver with configurable partition depth
void SudokuSolver::solveParallelOptimized(int numThreads, int partitionDepth) {
    auto start = std::chrono::high_resolution_clock::now();
    
    omp_set_num_threads(numThreads);
    
    // Generate subproblems
    std::vector<Subproblem> subproblems;
    generateSubproblems(partitionDepth, subproblems);
    
    if (subproblems.empty()) {
        numSolutions = 0;
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        runningTime = duration.count();
        return;
    }
    
    int totalSolutions = 0;
    int numSubproblems = static_cast<int>(subproblems.size());
    
    // Parallel loop over subproblems
    #pragma omp parallel for reduction(+:totalSolutions) schedule(dynamic)
    for (int i = 0; i < numSubproblems; ++i) {
        int solutions = solveSubproblem(subproblems[i]);
        totalSolutions += solutions;
    }
    
    numSolutions = totalSolutions;
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    runningTime = duration.count();
}
