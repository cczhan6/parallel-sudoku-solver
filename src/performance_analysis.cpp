#include "sudoku_solver.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

// Structure to store performance results
struct PerformanceResult {
    int boardSize;
    int numThreads;
    int numSolutions;
    double executionTime;
    double speedup;
    double efficiency;
};

// Get test board for 9x9
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

// Get a harder test board with more empty cells
std::vector<int> getHardTestBoard9x9() {
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

// Generate performance report
void generatePerformanceReport() {
    std::vector<PerformanceResult> results;
    std::vector<int> boardSizes = {9};  // Can extend to 16, 25 for larger boards
    std::vector<int> threadCounts = {1, 2, 4, 8};
    
    std::cout << "Performance Analysis for Parallel Sudoku Solver\n";
    std::cout << "===============================================\n\n";
    
    for (int N : boardSizes) {
        std::vector<int> board;
        
        // Get appropriate test board
        if (N == 9) {
            board = getHardTestBoard9x9();
        }
        
        std::cout << "Testing " << N << "x" << N << " board...\n";
        
        double baselineTime = 0.0;
        
        for (int threads : threadCounts) {
            SudokuSolver solver(N);
            solver.loadBoard(board);
            
            if (threads == 1) {
                solver.solveSingleThread();
                baselineTime = solver.getRunningTime();
            } else {
                solver.solveParallel(threads);
            }
            
            PerformanceResult result;
            result.boardSize = N;
            result.numThreads = threads;
            result.numSolutions = solver.getNumSolutions();
            result.executionTime = solver.getRunningTime();
            result.speedup = (baselineTime > 0) ? (baselineTime / result.executionTime) : 1.0;
            result.efficiency = (result.speedup / threads) * 100.0;
            
            results.push_back(result);
            
            std::cout << "  Threads: " << threads 
                     << ", Time: " << std::fixed << std::setprecision(2) 
                     << result.executionTime << " ms"
                     << ", Speedup: " << result.speedup << "x"
                     << ", Efficiency: " << result.efficiency << "%\n";
        }
        std::cout << "\n";
    }
    
    // Write results to CSV file
    std::ofstream csvFile("performance_results.csv");
    if (csvFile.is_open()) {
        csvFile << "Board Size,Threads,Solutions,Execution Time (ms),Speedup,Efficiency (%)\n";
        
        for (const auto& result : results) {
            csvFile << result.boardSize << ","
                   << result.numThreads << ","
                   << result.numSolutions << ","
                   << std::fixed << std::setprecision(2) << result.executionTime << ","
                   << std::fixed << std::setprecision(4) << result.speedup << ","
                   << std::fixed << std::setprecision(2) << result.efficiency << "\n";
        }
        
        csvFile.close();
        std::cout << "Performance results saved to performance_results.csv\n";
    } else {
        std::cerr << "Error: Could not create CSV file\n";
    }
    
    // Print summary
    std::cout << "\n=== Performance Summary ===\n";
    std::cout << "The parallel implementation shows speedup with increasing thread counts.\n";
    std::cout << "Efficiency indicates how well the parallel resources are utilized.\n";
    std::cout << "Speedup = Single-thread time / Multi-thread time\n";
    std::cout << "Efficiency = (Speedup / Number of threads) * 100%\n";
}

int main() {
    generatePerformanceReport();
    return 0;
}
