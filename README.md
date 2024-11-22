# Exploring Implicit and Explicit Parallelization with Matrix Transposition

## Table of Contents

- [Replication](#replication)
    - [Windows Machine](#windows-machine)
    - [Unitn Cluster](#unitn-cluster)
- [Files Explanation](#files-explanation)
- [Contact](#contact)

## Replication
I worked in two different enviroments. The idea was also to look at the difference between a controlled envinroment (UNITN Cluster) and a more realistic one (my windows machine).

### Windows Machine
#### Machine Specification
* Hardware:
    * CPU: Intel(R) Core(TM) i7-1165G7, 4 cores, Base speer: 2.8 GHz .
    * RAM: 16 GB.
    * Storage: 512 GB SSD with 200 GB free.
* Operating System:
    * Name and Version: Windows 11, version 23H2.
    * Architecture: 64-bit.
* Software Environment
    * Compiler: GCC 13.2.0 via MinGW-w64 (Built by MSYS2 project).

### UNITN Cluster
* Software Environment
    * Compiler: GCC 9.1

## Files Explanation
* Matrix Transposition files
    * [transposition_seq.c](transposition_seq.c):
        * description: this file contains the sequential code for the matrix transposition
        * compilation: gcc seq_matrix_transposition.c -O0
        * run: .\a.exe 2 -> .\a.exe 12 or ./a.out 2 -> ./a.out 12
    * [transposition_unroll.c](transposition_unroll.c): 
        * description: this file contains the explicit optimization using loop unrolling
        * compilation: gcc par_matrix_transposition_unroll.c -O0
        * run: .\a.exe 2 -> .\a.exe 12 or ./a.out 2 -> ./a.out 12
    * [transposition_vectorization_4.c](transposition_vectorization_4.c): 
        * description: this file contains the explicit parallelization using vectorization of blocks 4*4
        * compilation: gcc par_matrix_transposition_vectorization_4.c -O0
        * run: .\a.exe 2 -> .\a.exe 12 or ./a.out 2 -> ./a.out 12
    * [transposition_vectorization_8.c](transposition_vectorization_8.c): 
        * description: this file uses explicit parallilazion using vectorization of blocks 8*8 
        * compilation: gcc par_matrix_transposition_vectorization_8.c -O0 -mavx2
        * run: .\a.exe 2 -> .\a.exe 12 or ./a.out 2 -> ./a.out 12
    * [transposition_openmp.c](transposition_openmp.c)
        * description: this file contains implicit parallelization through openMP
        * compilation: gcc par_matrix_transposition_openmp.c -fopenmp
        * run: .\a.exe 2 -> .\a.exe 12 or ./a.out 2 -> ./a.out 12
    * [transposition_openmp_threadsv.c](transposition_openmp_threadsv.c)
        * description: this file contains the code to look how different numbers of thread influence on the execution time
        * compilation: gcc transposition_openmp_threadsv.c -fopenmp
        * run: .\a.exe 2 -> .\a.exe 12 or ./a.out 2 -> ./a.out 12
* Matrix Symmetry Check files
    * [sym_check_seq.c](sym_check_seq.c): 
        * description: this file contains the sequential code for the matrix symmetry check
        * compilation: gcc sym_check_seq.c -O0
        * run: .\a.exe 2 -> .\a.exe 12 or ./a.out 2 -> ./a.out 12
    * [sym_check_unroll.c](sym_check_unroll.c):
        * description: this file contains the explicit optimization using loop unrolling
        * compilation: gcc sym_check_unroll.c -O0
        * run: .\a.exe 2 -> .\a.exe 12 or ./a.out 2 -> ./a.out 12
    * [sym_check_vectorization_4.c](sym_check_vectorization_4.c):
        * description: this file contains the explicit parallelization using vectorization of array of 4 items
        * compilation: gcc sym_check_vectorization_4.c -O0
        * run: .\a.exe 2 -> .\a.exe 12 or ./a.out 2 -> ./a.out 12
    * [sym_check_vectorization_8.c](sym_check_vectorization_8.c):
        * description: this file contains the explicit parallelization using vectorization of array of 8 items
        * compilation: gcc sym_check_vectorization_8.c -O0 -mavx2
        * run: .\a.exe 2 -> .\a.exe 12 or ./a.out 2 -> ./a.out 12
    * [sym_check_openmp.c](sym_check_openmp.c):
        * description: this file contains implicit parallelization through openMP
        * compilation: gcc sym_check_openmp.c -O0
        * run: .\a.exe 2 -> .\a.exe 12 or ./a.out 2 -> ./a.out 12
    * [sym_check_openmp_threadsv.c](sym_check_openmp_threadsv.c):
        * description: this file contains the code to look how different numbers of thread influence on the execution time
        * compilation: gcc sym_check_openmp_threadsv.c -O0
        * run: .\a.exe 2 -> .\a.exe 12 or ./a.out 2 -> ./a.out 12

## Contact
email: davide.facchinelli@studenti.unitn.it