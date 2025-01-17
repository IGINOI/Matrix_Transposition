# Exploring Implicit, Explicit (OPENMP) and MPI Parallelization with Matrix Transposition

## Table of Contents

- [Replication](#replication)
    - [Windows Machine](#windows-machine)
    - [Unitn Cluster](#unitn-cluster)
- [Files Explanation](#files-explanation)
- [Contact](#contact)

## Repository Structure and Description
The repository is the result of the merge of two parts of a project. The first part is about trying to using OPENMP to optimize a matrix transposition sequential code, while the second part is about using MPI to do reach the same goal.\
During the first part of the project the codes where debugged and tested on a windows machine and the UNITN cluster was eventually used to collect final data. On the other hand, the second part was brough entirely on on the cluster, nevertheless, below, it is possible to find environmental specifications for both the machines.

### Windows Machine
* Hardware:
    * CPU: Intel(R) Core(TM) i7-1165G7, 4 cores, Base speer: 2.8 GHz
    * RAM: 16 GB
    * Storage: 512 GB SSD
* Operating System:
    * Name and Version: Windows 11, version 23H2
    * Architecture: 64-bit
* Software Environment
    * Compiler: GCC 13.2.0 via MinGW-w64 (Built by MSYS2 project)

### UNITN Cluster
* Hardware:
    * CPU: 96 Intel(R) Xeon(R) Gold 6252N CPU @ 2.30GHz
    * Thread(s) per core: 1
    * Core(s) per socket: 24
    * Socket(s): 4
    * L1d cache: 32K
    * L1i cache: 32K
    * L2 cache: 1024K
    * L3 cache: 36608K
    * RAM: 1.1TB
* Operating System:
    * Name and Version: CentOS Linux, version 7
    * Architecture: 64-bit
* Software Environment
    * Compiler: GCC 9.1

## Files Explanation & Code Usage Intructions for Replication
In order to compile and run the files you have two different choices. If you are in the cluster you can use the MainScript.pbs, alternatively you can run the file that you are interested in separately.
* PBS files for Cluster
    * [OpenMP.pbs](OpenMP.pbs)
        * This is the file that you have to use to compile and run all the codes that are used to analyze the Implicit and Explicit Optimization on the cluster in one shot. It is widely commented so it should be easy to use it.
        * Utilization: qsub -q short_cpuQ OpenMP.pbs
    * [MPI.pbs](MPI.pbs)
        * This is the file that you have to use to compile and run all the codes that are used to analyze MPI directives and characteristics on the cluster in one shot.\
        To be clear, it is divided into 2 parts, the first analyzing matrix transposition and the second analyzing matrix symmetry check. Each of these two sections is further divided into 4 additional ones: the first one used explore the effect of using different matrix sizes with different amounts of processes, the second one used to run the sequential baseline and the OPENMP code to have a comparison, the third one used for the strong scaling and the last one for the weak scaling.\
        Moreover at the start, all the information about the cluster architectures are printed out.
        * Utilization: qsub -q short_cpuQ MPI.pbs
* Matrix Transposition files
    * [transposition_seq.c](transposition_seq.c):
        * description: this file contains the sequential code for the matrix transposition.
        * compilation: gcc seq_matrix_transposition.c -O0.
        * run: .\a.exe 4 -> .\a.exe 12 or ./a.out 4 -> ./a.out 12.
    * [transposition_unroll.c](transposition_unroll.c): 
        * description: this file contains the explicit optimization using loop unrolling. To change the level of unrolling there is the need to uncomment the lines in the SymCheck function. I decided to use this method beacause I tought that it was the most intuitive.
        * compilation: gcc par_matrix_transposition_unroll.c -O0.
        * run: .\a.exe 4 -> .\a.exe 12 or ./a.out 4 -> ./a.out 12.
    * [transposition_vectorization_4.c](transposition_vectorization_4.c): 
        * description: this file contains the explicit parallelization using vectorization of blocks 4*4.
        * compilation: gcc par_matrix_transposition_vectorization_4.c -O0.
        * run: .\a.exe 4 -> .\a.exe 12 or ./a.out 4 -> ./a.out 12.
    * [transposition_vectorization_8.c](transposition_vectorization_8.c): 
        * description: this file uses explicit parallilazion using vectorization of blocks 8*8.
        * compilation: gcc par_matrix_transposition_vectorization_8.c -O0 -mavx2.
        * run: .\a.exe 4 -> .\a.exe 12 or ./a.out 4 -> ./a.out 12.
    * [transposition_openmp.c](transposition_openmp.c)
        * description: this file contains implicit parallelization through openMP. To see the differece that is possible to get with all the conmbinations of directives that I tried there is the need to uncomment them in the code.
        * compilation: gcc par_matrix_transposition_openmp.c -fopenmp.
        * run: .\a.exe 4 -> .\a.exe 12 or ./a.out 4 -> ./a.out 12.
    * [transposition_openmp_threadsv.c](transposition_openmp_threadsv.c)
        * description: this file contains the code to look how different numbers of thread influence on the execution time.
        * compilation: gcc transposition_openmp_threadsv.c -fopenmp.
        * run: .\a.exe 4 -> .\a.exe 12 or ./a.out 4 -> ./a.out 12.
    * [transposition_MPI_blocks.c](transposition_MPI_blocks.c)
        * description: this file contains MPI solution to the problem. The technique used is by rows distribution of the matrix.
        * compilation: mpicc -o transposition_MPI_blocks transposition_MPI_blocks.c.
        * run: mpirun -np 4 ./transposition_MPI_blocks 12.
* Matrix Symmetry Check files
    * [sym_check_seq.c](sym_check_seq.c): 
        * description: this file contains the sequential code for the matrix symmetry check.
        * compilation: gcc sym_check_seq.c -O0.
        * run: .\a.exe 4 -> .\a.exe 12 or ./a.out 4 -> ./a.out 12.
    * [sym_check_unroll.c](sym_check_unroll.c):
        * description: this file contains the explicit optimization using loop unrolling. To change the level of unrolling there is the need to uncomment the lines in the SymCheck function. I decided to use this method beacause I tought that it was the most intuitive.
        * compilation: gcc sym_check_unroll.c -O0.
        * run: .\a.exe 4 -> .\a.exe 12 or ./a.out 4 -> ./a.out 12.
    * [sym_check_vectorization_4.c](sym_check_vectorization_4.c):
        * description: this file contains the explicit parallelization using vectorization of array of 4 items.
        * compilation: gcc sym_check_vectorization_4.c -O0.
        * run: .\a.exe 4 -> .\a.exe 12 or ./a.out 4 -> ./a.out 12.
    * [sym_check_vectorization_8.c](sym_check_vectorization_8.c):
        * description: this file contains the explicit parallelization using vectorization of array of 8 items.
        * compilation: gcc sym_check_vectorization_8.c -O0 -mavx2.
        * run: .\a.exe 4 -> .\a.exe 12 or ./a.out 4 -> ./a.out 12.
    * [sym_check_openmp.c](sym_check_openmp.c):
        * description: this file contains implicit parallelization through openMP.To see the differece that is possible to get with all the conmbinations of directives that I tried there is the need to uncomment them in the code.
        * compilation: gcc sym_check_openmp.c -fopenmp.
        * run: .\a.exe 4 -> .\a.exe 12 or ./a.out 4 -> ./a.out 12.
    * [sym_check_openmp_threadsv.c](sym_check_openmp_threadsv.c):
        * description: this file contains the code to look how different numbers of thread influence on the execution time.
        * compilation: gcc sym_check_openmp_threadsv.c -fopenmp.
        * run: .\a.exe 4 -> .\a.exe 12 or ./a.out 4 -> ./a.out 12.
    * [sym_check_MPI.c](sym_check_MPI.c):
        * description: this file contains MPI solution to the problem by means of a MPI_Bcast directive.
        * compilation: mpicc  sym_check_MPI.c.
        * run: mpirun -np 4 ./a.out 12.
    * [sym_check_MPI_blocks.c](sym_check_MPI_blocks.c):
        * description: this file contains an aptempt to use MPI to solve the problem by scattering around first the rows and then the columns of the matrix.
        * compilation: mpicc  sym_check_MPI_blocks.c.
        * run: mpirun -np 4 ./a.out 12.
        * note: this file is the only file that does not work, its porpuse is to show the idea of the technique I explored in trying to make it work.

 
## Contact
email: davide.facchinelli@studenti.unitn.it