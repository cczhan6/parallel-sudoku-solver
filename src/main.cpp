#include "sudoku_solver.h"
#include <iostream>
#include <vector>
#include <iomanip>

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

// Get a 16x16 test board
std::vector<int> getTestBoard16x16() {
    return {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
}

// Run benchmark comparing old and new parallel strategies
void runComprehensiveBenchmark(int N, const std::vector<int>& board) {
    std::cout << "=== Comprehensive Benchmark for " << N << "x" << N << " Sudoku ===\n\n";
    
    // Test single thread
    SudokuSolver solver1(N);
    solver1.loadBoard(board);
    std::cout << "Initial board:";
    solver1.printBoard();
    
    std::cout << "Single-threaded solving...\n";
    solver1.solveSingleThread();
    std::cout << "Solutions found: " << solver1.getNumSolutions() << "\n";
    std::cout << "Time: " << std::fixed << std::setprecision(2) << solver1.getRunningTime() << " ms\n\n";
    
    double singleThreadTime = solver1.getRunningTime();
    
    // Test different thread counts with old strategy
    std::cout << "--- Old Strategy (First Cell Only) ---\n";
    std::vector<int> threadCounts = {2, 4, 8};
    
    for (int threads : threadCounts) {
        SudokuSolver solver(N);
        solver.loadBoard(board);
        
        solver.solveParallel(threads);
        
        double speedup = singleThreadTime / solver.getRunningTime();
        double efficiency = (speedup / threads) * 100.0;
        
        std::cout << "Threads: " << threads 
                  << ", Time: " << std::fixed << std::setprecision(2) << solver.getRunningTime() << " ms"
                  << ", Speedup: " << std::setprecision(2) << speedup << "x"
                  << ", Efficiency: " << std::setprecision(1) << efficiency << "%\n";
    }
    std::cout << "\n";
    
    // Test different thread counts with optimized strategy
    std::cout << "--- Optimized Strategy (K-Level Partitioning) ---\n";
    
    // Automatically choose partition depth based on board size
    int partitionDepth = (N == 9) ? 2 : (N == 16) ? 3 : 2;
    
    for (int threads : threadCounts) {
        SudokuSolver solver(N);
        solver.loadBoard(board);
        
        solver.solveParallelOptimized(threads, partitionDepth);
        
        double speedup = singleThreadTime / solver.getRunningTime();
        double efficiency = (speedup / threads) * 100.0;
        
        std::cout << "Threads: " << threads 
                  << ", Depth: " << partitionDepth
                  << ", Time: " << std::fixed << std::setprecision(2) << solver.getRunningTime() << " ms"
                  << ", Speedup: " << std::setprecision(2) << speedup << "x"
                  << ", Efficiency: " << std::setprecision(1) << efficiency << "%\n";
    }
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
    std::cout << "OpenMP Parallel Sudoku Solver - Optimized Version\n";
    std::cout << "==================================================\n\n";
    
    // Parse command line arguments
    int boardSize = 9;
    int partitionDepth = 2;
    int numThreads = 4;
    
    if (argc > 1) {
        boardSize = std::atoi(argv[1]);
    }
    if (argc > 2) {
        partitionDepth = std::atoi(argv[2]);
    }
    if (argc > 3) {
        numThreads = std::atoi(argv[3]);
    }
    
    // Get appropriate board
    std::vector<int> board;
    if (boardSize == 9) {
        board = getTestBoard9x9();
        runComprehensiveBenchmark(9, board);
    } else if (boardSize == 16) {
        // For demo purposes, using empty 16x16 board would take too long
        // In real scenario, you'd have a partially filled 16x16 board
        std::cout << "Note: Full 16x16 benchmark would take too long for demo.\n";
        std::cout << "Using 9x9 board for demonstration instead.\n\n";
        board = getTestBoard9x9();
        runComprehensiveBenchmark(9, board);
    } else {
        std::cout << "Using standard 9x9 board.\n\n";
        board = getTestBoard9x9();
        runComprehensiveBenchmark(9, board);
    }
    
    return 0;
}
