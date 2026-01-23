#ifndef SUDOKU_SOLVER_H
#define SUDOKU_SOLVER_H

#include <vector>
#include <chrono>
#include <set>
#include <cstdint>

// Structure to hold bitmask state for faster validation
struct BitMaskState {
    std::vector<uint32_t> rowMask;
    std::vector<uint32_t> colMask;
    std::vector<uint32_t> blockMask;
    
    BitMaskState(int N);
    void set(int N, int blockSize, int row, int col, int value);
    void unset(int N, int blockSize, int row, int col, int value);
    bool canPlace(int N, int blockSize, int row, int col, int value) const;
};

// Structure representing a subproblem for parallel execution
struct Subproblem {
    std::vector<int> board;
    BitMaskState state;
    int startPos;
    
    Subproblem(int N) : board(), state(N), startPos(0) {}
};

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
    bool isValidWithBoard(const std::vector<int>& boardRef, int row, int col, int value) const;
    bool findNextEmptyCell(int& row, int& col) const;
    std::set<int> getPossibleValues(int row, int col) const;
    int backtrackSingleThread(int pos);
    int solveFromState(const std::vector<int>& boardRef, int pos);
    
    // Optimized methods with bitmask
    int backtrackWithBitmask(std::vector<int>& boardRef, BitMaskState& state, int pos);
    int solveSubproblem(const Subproblem& subproblem);
    void generateSubproblems(int partitionDepth, std::vector<Subproblem>& subproblems);
    void generateSubproblemsRecursive(Subproblem& current, int depth, int maxDepth, 
                                     std::vector<Subproblem>& results);

public:
    // Constructor
    SudokuSolver(int N);

    // Load board from vector
    void loadBoard(const std::vector<int>& boardData);

    // Solving methods
    void solveSingleThread();
    void solveParallel(int numThreads);
    void solveParallelOptimized(int numThreads, int partitionDepth);

    // Query methods
    int getNumSolutions() const;
    double getRunningTime() const;
    int getSize() const;
    int getBlockSize() const;

    // Display method
    void printBoard() const;
};

#endif // SUDOKU_SOLVER_H
