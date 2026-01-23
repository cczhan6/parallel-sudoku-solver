# OpenMP Parallel Sudoku Solver

A high-performance parallel sudoku solver implemented in C++ using OpenMP for parallel computation. This project demonstrates parallel programming techniques for solving constraint satisfaction problems.

## Features

- **Standard 9×9 Sudoku Support**: Solves standard sudoku puzzles
- **Generalized N×N Support**: Can solve larger sudoku variants (N must be a perfect square: 16×16, 25×25, etc.)
- **Single-threaded Solver**: Traditional backtracking algorithm
- **Two Parallel Strategies**:
  - **Original**: First-cell partitioning (for comparison)
  - **Optimized**: K-level partitioning with bitmask validation (recommended)
- **Bitmask Constraint Checking**: O(1) validation using bitwise operations
- **Configurable Partition Depth**: Adjust task granularity for optimal performance
- **Dynamic Load Balancing**: OpenMP dynamic scheduling for uneven workloads
- **Performance Analysis**: Comprehensive benchmarking tools to measure speedup and efficiency
- **Finds All Solutions**: Enumerates all possible solutions for a given puzzle

## Project Structure

```
parallel-sudoku-solver/
├── src/
│   ├── sudoku_solver.h           # SudokuSolver class definition
│   ├── sudoku_solver.cpp         # Core solver implementation
│   ├── main.cpp                  # Main program with benchmarks
│   └── performance_analysis.cpp  # Performance analysis tool
├── CMakeLists.txt                # Build configuration
├── README.md                     # This file
└── LICENSE                       # MIT License
```

## Building the Project

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.10 or higher
- OpenMP support

### Build Steps

```bash
# Clone the repository
git clone https://github.com/cczhan6/parallel-sudoku-solver.git
cd parallel-sudoku-solver

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make

# Or with optimization flags (recommended)
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

## Usage

### Running the Main Program

The main program tests both the original and optimized parallel solvers:

```bash
./sudoku_solver [boardSize] [partitionDepth] [numThreads]
```

**Parameters** (all optional):
- `boardSize`: Size of the board (default: 9)
- `partitionDepth`: K-level partition depth for optimized solver (default: 2)
- `numThreads`: Number of threads to use (default: 4)

**Example:**
```bash
# Run with defaults (9x9 board, depth=2, 4 threads)
./sudoku_solver

# Run with specific parameters
./sudoku_solver 9 3 8
```

Output includes:
- Initial board display
- Single-threaded baseline performance
- **Original parallel strategy** performance (2, 4, 8 threads)
- **Optimized parallel strategy** performance (2, 4, 8 threads) with K-level partitioning
- Speedup and efficiency metrics for comparison

### Running Performance Analysis

Generate comprehensive performance reports comparing both strategies:

```bash
./performance_analysis
```

This creates a `performance_results.csv` file with detailed performance metrics including:
- Board sizes tested
- Strategy comparison (Original vs Optimized)
- Thread counts (1, 2, 4, 8)
- Partition depths (1, 2, 3) for optimized strategy
- Execution times
- Speedup ratios
- Parallel efficiency percentages

**Sample Output:**
```
Board Size,Strategy,Threads,Partition Depth,Solutions,Execution Time (ms),Speedup,Efficiency (%)
9,Baseline,1,0,1,861.90,1.0000,100.00
9,Old,2,1,1,685.65,1.2570,62.85
9,Optimized,2,2,1,190.68,4.5202,226.01
9,Optimized,4,2,1,164.28,5.2466,131.16
9,Optimized,8,3,1,169.04,5.0988,63.73
```

## Algorithm Design

### Backtracking Algorithm

The core solving algorithm uses recursive backtracking:

```
backtrack(position):
    if position == N*N:
        return 1  // Found a complete solution
    
    row, col = get_row_col(position)
    if board[row][col] != 0:
        return backtrack(position + 1)  // Skip filled cells
    
    count = 0
    for value = 1 to N:
        if isValid(row, col, value):
            board[row][col] = value
            count += backtrack(position + 1)
            board[row][col] = 0  // Backtrack
    
    return count
```

### Constraint Checking

Three types of constraints are validated:
- **Row Constraint**: Each value 1-N appears once per row
- **Column Constraint**: Each value 1-N appears once per column
- **Block Constraint**: Each value 1-N appears once per block (√N × √N subgrid)

Methods:
- `isInRow(row, value)`: Checks if value exists in the row
- `isInCol(col, value)`: Checks if value exists in the column
- `isInBlock(row, col, value)`: Checks if value exists in the block
- `isValid(row, col, value)`: Combines all three constraints

### Parallelization Strategy

The solver now includes two parallel strategies:

#### 1. Original Strategy (solveParallel)
1. **Find First Empty Cell**: Locate the first empty position in the board
2. **Generate Possible Values**: Enumerate all valid values for this cell
3. **Parallel Task Distribution**: Use OpenMP to distribute tasks
   ```cpp
   #pragma omp parallel for reduction(+:totalSolutions)
   for (each possible value):
       Create independent board copy
       Fill the first empty cell with this value
       Solve remaining puzzle recursively
       Accumulate solutions
   ```
4. **Thread-Safe Accumulation**: Use OpenMP reduction to safely combine results

This approach provides limited parallelism as it only partitions at the first empty cell.

#### 2. Optimized Strategy (solveParallelOptimized) - **NEW**
1. **K-Level Partitioning**: Generate subproblems by exploring the first K empty cells
   - Creates many more fine-grained tasks (e.g., if K=2 and each cell has 5 options, creates 25 tasks)
   - Better load balancing across threads
   - Configurable partition depth based on board size and thread count
   
2. **Bitmask-Based Validation**: O(1) constraint checking using bitwise operations
   ```cpp
   struct BitMaskState {
       vector<uint32_t> rowMask;    // Track used values in each row
       vector<uint32_t> colMask;    // Track used values in each column
       vector<uint32_t> blockMask;  // Track used values in each block
   }
   ```
   - Replaces O(N) row/column/block scanning with O(1) bit operations
   - Significantly reduces validation overhead
   
3. **Dynamic Scheduling**: Better load balancing for uneven subproblems
   ```cpp
   #pragma omp parallel for reduction(+:totalSolutions) schedule(dynamic)
   ```
   
4. **Reduced Memory Overhead**: 
   - Bitmask state is more compact than repeated validation
   - Board copying only at partition boundaries, not at every recursion level

**Performance Comparison:**
- **Old Strategy**: ~63% efficiency at 2 threads, ~17% at 8 threads
- **Optimized Strategy**: ~226% efficiency at 2 threads (super-linear due to cache effects), ~64% at 8 threads

The optimized strategy achieves 4-5x speedup on complex puzzles with proper partition depth selection.

## Core Classes and Methods

### SudokuSolver Class

**Constructor:**
- `SudokuSolver(int N)`: Initialize N×N sudoku solver

**Board Management:**
- `loadBoard(const vector<int>& board)`: Load puzzle from flat vector (row-major order)
- `printBoard()`: Display the current board state

**Solving Methods:**
- `solveSingleThread()`: Solve using single-threaded backtracking
- `solveParallel(int numThreads)`: Solve using original parallel approach (first cell partitioning)
- `solveParallelOptimized(int numThreads, int partitionDepth)`: **NEW** - Solve using optimized K-level partitioning strategy

**Query Methods:**
- `getNumSolutions()`: Returns number of solutions found
- `getRunningTime()`: Returns execution time in milliseconds
- `getSize()`: Returns board size N
- `getBlockSize()`: Returns block size (√N)

**Helper Methods (Private):**
- `isValid(row, col, value)`: Check if placement is valid
- `findNextEmptyCell(row, col)`: Find next unfilled cell
- `getPossibleValues(row, col)`: Get valid values for a cell
- `backtrackSingleThread(pos)`: Recursive backtracking solver
- `solveFromState(boardCopy, pos)`: Solve from given state (for parallel)

## Performance Metrics

### Speedup
Measures how much faster the parallel version is compared to single-threaded:
```
Speedup = T_single / T_parallel
```

### Parallel Efficiency
Measures how effectively the parallel implementation uses available threads:
```
Efficiency = (Speedup / Number of Threads) × 100%
```

Ideal efficiency is 100%, but typically decreases as thread count increases due to:
- Synchronization overhead
- Load imbalancing
- Memory bandwidth limitations

## Example Output

```
OpenMP Parallel Sudoku Solver - Optimized Version
==================================================

=== Comprehensive Benchmark for 9x9 Sudoku ===

Initial board:
5 3 . | . 7 . | . . . 
6 . . | 1 9 5 | . . . 
. 9 8 | . . . | . 6 . 
----------------------
8 . . | . 6 . | . . 3 
4 . . | 8 . 3 | . . 1 
7 . . | . 2 . | . . 6 
----------------------
. 6 . | . . . | 2 8 . 
. . . | 4 1 9 | . . 5 
. . . | . 8 . | . 7 9 

Single-threaded solving...
Solutions found: 1
Time: 861.90 ms

--- Old Strategy (First Cell Only) ---
Threads: 2, Time: 685.65 ms, Speedup: 1.26x, Efficiency: 62.9%
Threads: 4, Time: 817.99 ms, Speedup: 1.05x, Efficiency: 26.3%
Threads: 8, Time: 621.73 ms, Speedup: 1.39x, Efficiency: 17.3%

--- Optimized Strategy (K-Level Partitioning) ---
Threads: 2, Depth: 2, Time: 190.68 ms, Speedup: 4.52x, Efficiency: 226.0%
Threads: 4, Depth: 2, Time: 164.28 ms, Speedup: 5.25x, Efficiency: 131.2%
Threads: 8, Depth: 2, Time: 181.12 ms, Speedup: 4.76x, Efficiency: 59.5%
```

**Key Observations:**
- The original strategy shows poor scaling (17-63% efficiency)
- The optimized strategy achieves 4-5x speedup with proper load balancing
- Super-linear speedup (>100% efficiency) at 2-4 threads due to improved cache locality
- At 8 threads, efficiency is still respectable at ~64%

## Testing with Different Boards

You can easily test with custom boards by modifying the board vectors in `main.cpp`:

```cpp
std::vector<int> customBoard = {
    // Your 9x9 or 16x16 board here
    // Use 0 for empty cells
};
```

## Learning Resources

- [OpenMP Tutorial](https://www.openmp.org/resources/tutorials-articles/)
- [Backtracking Algorithms](https://en.wikipedia.org/wiki/Backtracking)
- [Sudoku Solving Algorithms](https://en.wikipedia.org/wiki/Sudoku_solving_algorithms)

## Future Improvements

- Add support for more sophisticated solving techniques (naked pairs, hidden singles, etc.)
- Support for custom board input from files or command-line
- GPU acceleration using CUDA or OpenCL
- Adaptive partition depth selection based on problem characteristics
- Work-stealing queue for even better load balancing
- Support for very large boards (25×25, 36×36)
- Graphical user interface
- Hybrid solving strategies combining constraint propagation with backtracking

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Author

Yi-Zhan Cheng

## Acknowledgments

- OpenMP community for excellent parallel programming support
- Sudoku puzzle contributors for test cases
