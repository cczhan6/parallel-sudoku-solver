#include "sudoku_solver.h"
#include <iostream>
#include <vector>

// Get a standard 9x9 test board with moderate difficulty
std::vector<int> getTestBoard9x9() {
    return {
        5, 3, 0, 0, 7, 0, 0, 0, 0,
        6, 0, 0, 1, 9, 5, 0, 0, 0,
        0, 9, 8, 0, 0, 0, 0, 6, 0,
        8, 0, 0, 0, 6, 0, 0, 0, 3,
        4, 0, 0, 8, 0, 3, 0, 0, 1,
        7, 0, 0, 0, 2, 0, 0, 0, 6,
        0, 6, 0, 0, 0, 0, 2, 8, 0,
        0, 0, 0, 4, 1, 9, 0, 0, 5,
        0, 0, 0, 0, 8, 0, 0, 7, 9
    };
}

// Get a simpler 9x9 test board
std::vector<int> getSimpleTestBoard9x9() {
    return {
        0, 0, 3, 0, 2, 0, 6, 0, 0,
        9, 0, 0, 3, 0, 5, 0, 0, 1,
        0, 0, 1, 8, 0, 6, 4, 0, 0,
        0, 0, 8, 1, 0, 2, 9, 0, 0,
        7, 0, 0, 0, 0, 0, 0, 0, 8,
        0, 0, 6, 7, 0, 8, 2, 0, 0,
        0, 0, 2, 6, 0, 9, 5, 0, 0,
        8, 0, 0, 2, 0, 3, 0, 0, 9,
        0, 0, 5, 0, 1, 0, 3, 0, 0
    };
}

// Run benchmark for different thread counts
void runBenchmark(int N, const std::vector<int>& board) {
    std::cout << "=== Benchmark for " << N << "x" << N << " Sudoku ===\n\n";
    
    // Test single thread
    SudokuSolver solver1(N);
    solver1.loadBoard(board);
    std::cout << "Initial board:";
    solver1.printBoard();
    
    std::cout << "Single-threaded solving...\n";
    solver1.solveSingleThread();
    std::cout << "Solutions found: " << solver1.getNumSolutions() << "\n";
    std::cout << "Time: " << solver1.getRunningTime() << " ms\n\n";
    
    double singleThreadTime = solver1.getRunningTime();
    
    // Test different thread counts
    std::vector<int> threadCounts = {2, 4, 8};
    
    for (int threads : threadCounts) {
        SudokuSolver solver(N);
        solver.loadBoard(board);
        
        std::cout << "Parallel solving with " << threads << " threads...\n";
        solver.solveParallel(threads);
        std::cout << "Solutions found: " << solver.getNumSolutions() << "\n";
        std::cout << "Time: " << solver.getRunningTime() << " ms\n";
        
        double speedup = singleThreadTime / solver.getRunningTime();
        double efficiency = (speedup / threads) * 100.0;
        
        std::cout << "Speedup: " << speedup << "x\n";
        std::cout << "Efficiency: " << efficiency << "%\n\n";
    }
}

int main() {
    std::cout << "OpenMP Parallel Sudoku Solver\n";
    std::cout << "==============================\n\n";
    
    // Run benchmark on standard test board
    std::vector<int> board9x9 = getTestBoard9x9();
    runBenchmark(9, board9x9);
    
    return 0;
}
