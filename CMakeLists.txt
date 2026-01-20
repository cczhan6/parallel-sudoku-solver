cmake_minimum_required(VERSION 3.10)
project(parallel-sudoku-solver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenMP
find_package(OpenMP REQUIRED)

# Add main program
add_executable(sudoku_solver 
    src/sudoku_solver.cpp
    src/main.cpp
)
target_link_libraries(sudoku_solver PUBLIC OpenMP::OpenMP_CXX)
target_include_directories(sudoku_solver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(MSVC)
    target_compile_options(sudoku_solver PRIVATE /O2)
else()
    target_compile_options(sudoku_solver PRIVATE -O2)
endif()

# Add performance analysis tool
add_executable(performance_analysis
    src/sudoku_solver.cpp
    src/performance_analysis.cpp
)
target_link_libraries(performance_analysis PUBLIC OpenMP::OpenMP_CXX)
target_include_directories(performance_analysis PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(MSVC)
    target_compile_options(performance_analysis PRIVATE /O2)
else()
    target_compile_options(performance_analysis PRIVATE -O2)
endif()

enable_testing()
