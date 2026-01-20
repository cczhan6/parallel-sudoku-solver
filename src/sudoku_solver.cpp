#include "sudoku_solver.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <omp.h>

// Constructor
SudokuSolver::SudokuSolver(int N) : N(N), numSolutions(0), runningTime(0.0) {
    blockSize = static_cast<int>(sqrt(N));
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
    if (pos == static_cast<int>(boardRef.size())) {
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
    
    // Parallel loop over possible values
    #pragma omp parallel for reduction(+:totalSolutions)
    for (size_t i = 0; i < valuesList.size(); ++i) {
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
