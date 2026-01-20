# OpenMP Parallel Sudoku Solver

A high-performance parallel sudoku solver implemented in C++ using OpenMP for parallel computation. This project demonstrates parallel programming techniques for solving constraint satisfaction problems.

## Features

- **Standard 9×9 Sudoku Support**: Solves standard sudoku puzzles
- **Generalized N×N Support**: Can solve larger sudoku variants (N must be a perfect square: 16×16, 25×25, etc.)
- **Single-threaded Solver**: Traditional backtracking algorithm
- **Parallel Solver**: OpenMP-based parallel implementation with task distribution
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

The main program tests the solver with standard 9×9 sudoku boards:

```bash
./sudoku_solver
```

Output includes:
- Initial board display
- Single-threaded solving time and solution count
- Multi-threaded performance with 2, 4, and 8 threads
- Speedup and efficiency metrics

### Running Performance Analysis

Generate comprehensive performance reports:

```bash
./performance_analysis
```

This creates a `performance_results.csv` file with detailed performance metrics including:
- Board sizes tested
- Thread counts (1, 2, 4, 8)
- Execution times
- Speedup ratios
- Parallel efficiency percentages

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

The parallel solver uses OpenMP task distribution:

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

This approach provides good load balancing when the first empty cell has multiple possible values, creating independent subproblems that can be solved in parallel.

## Core Classes and Methods

### SudokuSolver Class

**Constructor:**
- `SudokuSolver(int N)`: Initialize N×N sudoku solver

**Board Management:**
- `loadBoard(const vector<int>& board)`: Load puzzle from flat vector (row-major order)
- `printBoard()`: Display the current board state

**Solving Methods:**
- `solveSingleThread()`: Solve using single-threaded backtracking
- `solveParallel(int numThreads)`: Solve using parallel approach with specified thread count

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
OpenMP Parallel Sudoku Solver
==============================

=== Benchmark for 9x9 Sudoku ===

Initial board:
5 3 . | . 7 . | . . .
6 . . | 1 9 5 | . . .
. 9 8 | . . . | . 6 .
------+-------+------
8 . . | . 6 . | . . 3
4 . . | 8 . 3 | . . 1
7 . . | . 2 . | . . 6
------+-------+------
. 6 . | . . . | 2 8 .
. . . | 4 1 9 | . . 5
. . . | . 8 . | . 7 9

Single-threaded solving...
Solutions found: 1
Time: 45.23 ms

Parallel solving with 2 threads...
Solutions found: 1
Time: 24.56 ms
Speedup: 1.84x
Efficiency: 92.00%

Parallel solving with 4 threads...
Solutions found: 1
Time: 13.78 ms
Speedup: 3.28x
Efficiency: 82.00%

Parallel solving with 8 threads...
Solutions found: 1
Time: 8.91 ms
Speedup: 5.08x
Efficiency: 63.50%
```

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
- Implement iterative deepening for better load balancing
- Add GPU acceleration using CUDA or OpenCL
- Support for custom board input from files
- Graphical user interface
- More intelligent parallelization strategies (work stealing, dynamic scheduling)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Author

Yi-Zhan Cheng

## Acknowledgments

- OpenMP community for excellent parallel programming support
- Sudoku puzzle contributors for test cases
