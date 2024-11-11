# Exploring Implicit and Explicit Parallelization with Matrix Transposition

## Table of Contents

- [Replication](#replication)
- [FileExplaination](#fileexplaination)
- [Contact](#contact)

## Replication
I worked in two different enviroments. The idea was also to look at the difference between a controlled envinroment (UNITN Cluster) and a more realistic one (my windows machine).

### Windows Machine
#### Machine Specification
* Hardware:
    * CPU: Intel Core i7-9700K, 8 cores, 3.6 GHz.
    * RAM: 16 GB DDR4.
    * Storage: 512 GB SSD with 200 GB free.
    * GPU: NVIDIA GeForce GTX 1660, Driver version 472.12.
* Operating System:
    * Name and Version: Windows 10 Pro, version 22H2.
    * Architecture: 64-bit.
* Software Environment
    * Compiler/IDE:
        * Compiler: GCC 11.2.0 via MinGW-w64.
        * IDE: Code::Blocks 20.03, with default compiler settings.
    * Libraries and Dependencies:
        * Boost 1.75.0, installed via vcpkg.
        * OpenCV 4.5.3, installed via vcpkg.
    * Build System:
        * Using CMake 3.21.2 for building the project.
        * CMake command: cmake -S . -B build and cmake --build build.
* Environment Variables and Configuration
    * Environment Variables:
        * PATH includes C:\Program Files\CMake\bin;C:\Program Files\mingw-w64\bin.
* Data Input and Output
    * Input Data:
        * Input data file format: CSV, with columns [A, B, C].
        * Example file: data/input.csv located in the project root.
    * Output Data:
        * Output file format: JSON, results are stored in results/output.json.

### UNITN Cluster
The version of gcc used to compile here is 9.1

## File Explaination
* Matrix Transposition files
    * seq_matrix_transposition.c: this file contains the sequential code for the matrix transposition
    * par_matrix_transposition_unroll.c:
    * par_matrix_transposition_vectorization_4.c:
    * par_matrix_transposition_vectorization_8.c: this file uses explicit parallilazion using Instrinsics functions 
    * par_matrix_transposition_openmp.c: this file uses implicit parallelization through OPENMP
* Matrix Symmetry Check files
    * seq_matrix_sym_check.c: 

## Contact
email: davide.facchinelli@studenti.unitn.it