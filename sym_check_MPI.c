#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <unistd.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //
// %%%%% FUNCTIONS DECLARATION %%%%%% //
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //

// Function that initializes the matrix with random values
void initializeSymmetricMatrix(float **matrix, int n);
// Function that prints the original matrix
void printMatrix(float **matrix, int n);
// Functions that checks if the matrix is symmetric using MPI
void checkSym(float *M_flat, int matrix_size, int start_index_local, int stop_index_local, int *start_indexes, int *stop_indexes);


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //
// %%%%%%%%% MAIN FUNCTION %%%%%%%%%% //
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //

int main(int argc, char *argv[]) {

    // ------------------------------------------------ //
    // ---------- ENVIRONMENT INITIALIZATION ---------- //
    // ------------------------------------------------ //
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Input validation
    if (rank == 0) {
        if (argc != 2) {
            printf("Please provide a matrix size as an argument.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    int exponent = atoi(argv[1]);
    if (exponent < 4 || exponent > 12) {
        if (rank == 0) {
            printf("Matrix size exponent must be between 4 and 12 (base is 2).\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Matrix size computation
    int matrix_size = 1 << exponent;
    int base_local_rows = matrix_size / size;

    int *start_indexes = NULL;
    int *stop_indexes = NULL;
    int start_index_local;
    int stop_index_local;

    if(rank==0){
        start_indexes = malloc(size * sizeof(int));
        stop_indexes = malloc(size * sizeof(int));

        for(int i = 0; i < size; i++) {
            start_indexes[i] = i * base_local_rows;
            stop_indexes[i] = (i == size - 1) ? matrix_size-1 : (i + 1) * base_local_rows-1;
        }
    }


    // ------------------------------------------------ //
    // ------------- MATRICES ALLOCATIONS ------------- //
    // ------------------------------------------------ //

    // M is the matrix declares as duble pointer to ensure continuity with the previous codes 
    float **M = (float **)malloc(matrix_size * sizeof(float *));
    // M_flat is the matrix flattened out
    float *M_flat = malloc(matrix_size * matrix_size * sizeof(float));

    if (rank == 0) {
        for (int i = 0; i < matrix_size; i++) {
            M[i] = (float *)malloc(matrix_size * sizeof(float));
        }
    }

    
    // ------------------------------------------------ //
    // ------------ MATRIX SYMMETRY CHECK ------------- //
    // ------------------------------------------------ //

    //Set the number of interations to have an average time
    int iterations = 1;
    double total_time = 0.0;


    for(int i = 0; i < iterations; i++){

        if (rank == 0) {
            initializeSymmetricMatrix(M, matrix_size);
            for(int i = 0; i < matrix_size; i++) {
                for(int j = 0; j < matrix_size; j++) {
                    M_flat[i * matrix_size + j] = M[i][j];
                }
            }
        }

        // REMOVE COMMENTS TO CHECK IF THE MATRIX IS ACTUALLY SYMMETRIC OR NOT
        // if(rank == 0) {
        //     printf("The original matrix is:\n");
        //     printMatrix(M, matrix_size);
        // }
        
        // Synchronize processes before starting taking the time
        MPI_Barrier(MPI_COMM_WORLD);
        double start_time = MPI_Wtime();

        // Call checkSym function to check if the matrix is symmetric
        checkSym(M_flat, matrix_size, start_index_local, stop_index_local, start_indexes, stop_indexes);

        double end_time = MPI_Wtime();

        // Compute the total time
        if (rank == 0) {
            total_time += end_time - start_time;
        }

        // Synchronize before starting a new loop cycle 
        MPI_Barrier(MPI_COMM_WORLD);
    }

    // ------------------------------------------------ //
    // -------------- TIME COMPUTATION ---------------- //
    // ------------------------------------------------ //
    if (rank == 0) {
        double average_time = total_time / iterations;
        printf("Average time for symmetry check: %f seconds\n", average_time);
    }

    // ------------------------------------------------ //
    // ----------------- FREE MEMORY ------------------ //
    // ------------------------------------------------ //

    free(start_indexes);
    free(stop_indexes);

    if(rank == 0) {
        for (int i = 0; i < matrix_size; i++) {
            free(M[i]);
        }
    }
    free(M_flat);
    free(M);

    MPI_Finalize();
    return 0;
}


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //
// %%%%%% FUNCTIONS DEFINITION %%%%%% //
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //

void initializeSymmetricMatrix(float **matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            float x = (float)rand() / RAND_MAX * 10.0f;
            matrix[i][j] = x;
            matrix[j][i] = x;
            //matrix[i][j] = (float)rand() / RAND_MAX * 10.0f;
        }
    }
}

void printMatrix(float **matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%6.2f ", matrix[i][j]);
        }
        printf("\n");
    }
}

void checkSym(float *M_flat, int matrix_size, int start_index_local, int stop_index_local, int *start_indexes, int *stop_indexes) {
    
    // Broadcast of the entire matrix to all the processes
    MPI_Bcast(M_flat, matrix_size * matrix_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // Scatter the informations about initial index and final index
    MPI_Scatter(start_indexes, 1, MPI_INT, &start_index_local, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(stop_indexes, 1, MPI_INT, &stop_index_local, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // ------------------------------------------------ //
    // ---------- LOCAL MATRIX SYM CHECK -------------- //
    // ------------------------------------------------ //
    int is_symmetric_local = 1;
    for (int i = start_index_local; i <= stop_index_local; i++) { 
        for (int j = 0; j < matrix_size; j++) { 
            if(M_flat[i * matrix_size + j] != M_flat[j * matrix_size + i]){ 
                is_symmetric_local = 0;
                break;
            } 
        } 
    }

    // Reducing the local results to a global result
    int is_symmetric_global = 1;
    MPI_Reduce(&is_symmetric_local, &is_symmetric_global, 1, MPI_INT, MPI_PROD, 0, MPI_COMM_WORLD);
    if (is_symmetric_global==0){
        printf("\n\n\n THE MATRIX IS NOT SYMMETRIC \n\n\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}