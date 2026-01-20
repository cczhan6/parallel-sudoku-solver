#ifndef SUDOKU_SOLVER_H
#define SUDOKU_SOLVER_H

#include <vector>
#include <chrono>
#include <set>

class SudokuSolver {
private:
    int N;              // Size of the board (N x N)
    int blockSize;      // Size of each block (sqrt(N))
    std::vector<int> board;  // Flattened board representation
    int numSolutions;   // Number of solutions found
    double runningTime; // Time taken to solve (in milliseconds)

    // Helper methods
    int getIndex(int row, int col) const;
    bool isInRow(int row, int value) const;
    bool isInCol(int col, int value) const;
    bool isInBlock(int row, int col, int value) const;
    bool isValid(int row, int col, int value) const;
    bool findNextEmptyCell(int& row, int& col) const;
    std::set<int> getPossibleValues(int row, int col) const;
    int backtrackSingleThread(int pos);
    int solveFromState(std::vector<int> boardCopy, int pos);

public:
    // Constructor
    SudokuSolver(int N);

    // Load board from vector
    void loadBoard(const std::vector<int>& boardData);

    // Solving methods
    void solveSingleThread();
    void solveParallel(int numThreads);

    // Query methods
    int getNumSolutions() const;
    double getRunningTime() const;
    int getSize() const;
    int getBlockSize() const;

    // Display method
    void printBoard() const;
};

#endif // SUDOKU_SOLVER_H
