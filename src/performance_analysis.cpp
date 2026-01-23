#include "sudoku_solver.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

// Structure to store performance results
struct PerformanceResult {
    int boardSize;
    std::string strategy;
    int numThreads;
    int partitionDepth;
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

// Get a very hard test board with many solutions for better parallel demonstration
std::vector<int> getVeryHardTestBoard9x9() {
    // This board has fewer hints to create more branching
    return {
        0, 0, 0, 0, 0, 0, 0, 1, 2,
        0, 0, 0, 0, 3, 5, 0, 0, 0,
        0, 0, 0, 6, 0, 0, 0, 7, 0,
        7, 0, 0, 0, 0, 0, 3, 0, 0,
        0, 0, 0, 4, 0, 0, 8, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 2, 0, 0, 0, 0,
        0, 8, 0, 0, 0, 0, 0, 4, 0,
        0, 5, 0, 0, 0, 0, 6, 0, 0
    };
}

// Generate performance report
void generatePerformanceReport() {
    std::vector<PerformanceResult> results;
    std::vector<int> boardSizes = {9};  // Can extend to 16, 25 for larger boards
    std::vector<int> threadCounts = {1, 2, 4, 8};
    std::vector<int> partitionDepths = {1, 2, 3};
    
    std::cout << "Performance Analysis for Parallel Sudoku Solver\n";
    std::cout << "===============================================\n\n";
    
    for (int N : boardSizes) {
        std::vector<int> board;
        
        // Get appropriate test board
        if (N == 9) {
            board = getVeryHardTestBoard9x9();  // Use harder test board
        } else {
            std::cerr << "Warning: No test board available for " << N << "x" << N << " board, skipping...\n";
            continue;
        }
        
        std::cout << "Testing " << N << "x" << N << " board...\n";
        
        double baselineTime = 0.0;
        
        // Test single thread baseline
        {
            SudokuSolver solver(N);
            solver.loadBoard(board);
            solver.solveSingleThread();
            baselineTime = solver.getRunningTime();
            
            PerformanceResult result;
            result.boardSize = N;
            result.strategy = "Baseline";
            result.numThreads = 1;
            result.partitionDepth = 0;
            result.numSolutions = solver.getNumSolutions();
            result.executionTime = solver.getRunningTime();
            result.speedup = 1.0;
            result.efficiency = 100.0;
            results.push_back(result);
            
            std::cout << "  [Baseline] Threads: 1, Time: " << std::fixed << std::setprecision(2) 
                     << result.executionTime << " ms\n";
        }
        
        // Test old parallel strategy
        std::cout << "\n  Testing OLD strategy (first cell only):\n";
        for (int threads : threadCounts) {
            if (threads == 1) continue;
            
            SudokuSolver solver(N);
            solver.loadBoard(board);
            solver.solveParallel(threads);
            
            PerformanceResult result;
            result.boardSize = N;
            result.strategy = "Old";
            result.numThreads = threads;
            result.partitionDepth = 1;
            result.numSolutions = solver.getNumSolutions();
            result.executionTime = solver.getRunningTime();
            result.speedup = (baselineTime > 0) ? (baselineTime / result.executionTime) : 1.0;
            result.efficiency = (result.speedup / threads) * 100.0;
            results.push_back(result);
            
            std::cout << "    Threads: " << threads 
                     << ", Time: " << std::fixed << std::setprecision(2) 
                     << result.executionTime << " ms"
                     << ", Speedup: " << std::setprecision(2) << result.speedup << "x"
                     << ", Efficiency: " << std::setprecision(1) << result.efficiency << "%\n";
        }
        
        // Test optimized parallel strategy with different partition depths
        std::cout << "\n  Testing OPTIMIZED strategy (K-level partitioning):\n";
        for (int depth : partitionDepths) {
            std::cout << "    Partition Depth = " << depth << ":\n";
            for (int threads : threadCounts) {
                if (threads == 1) continue;
                
                SudokuSolver solver(N);
                solver.loadBoard(board);
                solver.solveParallelOptimized(threads, depth);
                
                PerformanceResult result;
                result.boardSize = N;
                result.strategy = "Optimized";
                result.numThreads = threads;
                result.partitionDepth = depth;
                result.numSolutions = solver.getNumSolutions();
                result.executionTime = solver.getRunningTime();
                result.speedup = (baselineTime > 0) ? (baselineTime / result.executionTime) : 1.0;
                result.efficiency = (result.speedup / threads) * 100.0;
                results.push_back(result);
                
                std::cout << "      Threads: " << threads 
                         << ", Time: " << std::fixed << std::setprecision(2) 
                         << result.executionTime << " ms"
                         << ", Speedup: " << std::setprecision(2) << result.speedup << "x"
                         << ", Efficiency: " << std::setprecision(1) << result.efficiency << "%\n";
            }
        }
        std::cout << "\n";
    }
    
    // Write results to CSV file
    std::ofstream csvFile("performance_results.csv");
    if (csvFile.is_open()) {
        csvFile << "Board Size,Strategy,Threads,Partition Depth,Solutions,Execution Time (ms),Speedup,Efficiency (%)\n";
        
        for (const auto& result : results) {
            csvFile << result.boardSize << ","
                   << result.strategy << ","
                   << result.numThreads << ","
                   << result.partitionDepth << ","
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
    std::cout << "The optimized implementation uses K-level partitioning to create more\n";
    std::cout << "fine-grained subproblems, improving load balancing and parallel efficiency.\n";
    std::cout << "\n";
    std::cout << "Key improvements:\n";
    std::cout << "1. Bitmask-based constraint checking (faster validation)\n";
    std::cout << "2. K-level task partitioning (better parallelism)\n";
    std::cout << "3. Dynamic scheduling (better load balancing)\n";
    std::cout << "4. Reduced memory copying overhead\n";
}

int main() {
    generatePerformanceReport();
    return 0;
}
