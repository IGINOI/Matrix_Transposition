#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //
// %%%%% FUNCTIONS DECLARATION %%%%%% //
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //

// Initializes the Matrix with random values
void initializeMatrix(float **matrix, int n);
// Prints the Matrix
void printMatrix(float **matrix, int n);
// Checks if the Matrix is actually transposed
int matrixActuallyTransposed(float **matrix, float **transpose, int n);
// Transposes the Matrix using MPI Scatterv and Gatherv
void matTranspose(float *M_flat, int matrix_size, float *local_matrix, float **local_transpose, int rank, int *elements_per_process, int *scatter_displs, int *gather_displs, int *rows_per_process, float **T);


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
    // Number of rows per process
    int base_local_rows = matrix_size / size;

    if( matrix_size < size ) {
        if(rank == 0) {
            printf("Matrix size must be greater than or equal to the number of processes.\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    
    // ------------------------------------------------ //
    //  VARIABLE COMPUTATION FOR SCATTERV AND GATHERV - //
    // ------------------------------------------------ //

    int *rows_per_process = malloc(size * sizeof(int));
    int *elements_per_process = malloc(size * sizeof(int));
    int *scatter_displs = malloc(size * sizeof(int));
    int *gather_displs = malloc(size * sizeof(int));

    for (int i = 0; i < size; i++) {
        rows_per_process[i] = base_local_rows + ((i==size-1) ? matrix_size%size : 0);
        elements_per_process[i] = rows_per_process[i] * matrix_size;
        scatter_displs[i] = (i == 0) ? 0 : scatter_displs[i - 1] + rows_per_process[i - 1]*matrix_size;
        gather_displs[i] = (i == 0) ? 0 : gather_displs[i - 1] + rows_per_process[i - 1];
    }


    // ------------------------------------------------ //
    // ------------- MATRICES ALLOCATIONS ------------- //
    // ------------------------------------------------ //

    // Matrices for only rank 0
    float *M_flat = NULL;
    float **M = (float **)malloc(matrix_size * sizeof(float *));
    float **T = (float **)malloc(matrix_size * sizeof(float *));
    if (rank == 0) {
        M_flat = malloc(matrix_size * matrix_size * sizeof(float));
        for (int i = 0; i < matrix_size; i++) {
            T[i] = (float *)malloc(matrix_size * sizeof(float));
            M[i] = (float *)malloc(matrix_size * sizeof(float));
        }
    }

    // Matrices for all the ranks
    float *local_matrix = malloc(rows_per_process[rank] * matrix_size * sizeof(float));
    float **local_transpose = (float **)malloc(matrix_size * sizeof(float *));
    for (int i = 0; i < matrix_size; i++) {
        local_transpose[i] = (float *)malloc(rows_per_process[rank] * sizeof(float));
    }

    
    // ------------------------------------------------ //
    // ------------ SCATTERING THE MATRIX ------------- //
    // ------------------------------------------------ //

    //for loop to compute an average time
    double total_time = 0.0;
    int iterations = 50;

    MPI_Barrier(MPI_COMM_WORLD);

    for(int i = 0; i < iterations; i++){

        if (rank == 0) {
            initializeMatrix(M, matrix_size);
            for(int i = 0; i < matrix_size; i++) {
                for(int j = 0; j < matrix_size; j++) {
                    M_flat[i * matrix_size + j] = M[i][j];
                }
            }
        }
        
        // Process synchronization befor starting transposition
        MPI_Barrier(MPI_COMM_WORLD);
        double start_time = MPI_Wtime();

        matTranspose(M_flat, matrix_size, local_matrix, local_transpose, rank, elements_per_process, scatter_displs, gather_displs, rows_per_process, T);

        // Synchronize after each repetition
        MPI_Barrier(MPI_COMM_WORLD);
        double end_time = MPI_Wtime();

        // Compute the total time and check correctness
        double elapsed_time = end_time - start_time;
        if (rank == 0) {
            total_time += elapsed_time;

            // REMOVE THE COMMENTS BELOW TO CHECK CORRECT TRANSPOSITION
            // if (matrixActuallyTransposed(M, T, matrix_size)) {
            //     printf("Matrix transposed successfully.\n");
            // } else {
            //     printf("Matrix transposition failed.\n");
            // }
        }
    }

    // ------------------------------------------------ //
    // -------------- TIME COMPUTATION ---------------- //
    // ------------------------------------------------ //
    if (rank == 0) {
        double average_time = total_time / iterations;
        printf("Average time for %d * %d matrix transposition: %f ms\n", matrix_size, matrix_size, average_time*1000);
    }

    // ------------------------------------------------ //
    // ----------------- FREE MEMORY ------------------ //
    // ------------------------------------------------ //

    for (int i = 0; i < matrix_size; i++) {
        free(local_transpose[i]);
    }
    free(local_transpose);
    free(local_matrix);

    free(rows_per_process);
    free(scatter_displs);
    free(elements_per_process);
    free(gather_displs);

    if(rank == 0) {
        for (int i = 0; i < matrix_size; i++) {
            free(T[i]);
            free(M[i]);
        }
        free(M_flat);
    }

    free(T);
    free(M);

    MPI_Finalize();
    return 0;
}


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //
// %%%%%% FUNCTIONS DEFINITION %%%%%% //
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //

void initializeMatrix(float **matrix, int n) {
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i][j] = (float)rand() / RAND_MAX * 10.0f;
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

int matrixActuallyTransposed(float **matrix, float **transpose, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (matrix[i][j] != transpose[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

void matTranspose(float *M_flat, int matrix_size, float *local_matrix, float **local_transpose, int rank, int *elements_per_process, int *scatter_displs, int *gather_displs, int *rows_per_process, float **T) {
    MPI_Scatterv(M_flat, elements_per_process, scatter_displs, MPI_FLOAT, local_matrix, rows_per_process[rank] * matrix_size, MPI_FLOAT, 0, MPI_COMM_WORLD);


    // REMOVE THE FOLLOWING COMMENT TO SEE IF THE LOCAL MATRIX IS CORRECTLY RECEIVED
    // for (int i = 0; i < size; i++) {
    //     if (rank == i) {
    //         printf("Rank %d\n", rank);
    //         for (int i = 0; i < rows_per_process[rank]; i++) {
    //             for (int j = 0; j < matrix_size; j++) {
    //                 printf("%6.2f", local_matrix[i * matrix_size + j]);
    //             }
    //             printf("\n");
    //         }
    //     }
    //     MPI_Barrier(MPI_COMM_WORLD);
    // }


    // ------------------------------------------------ //
    // ---------- LOCAL MATRIX TRANSPOSITION ---------- //
    // ------------------------------------------------ //
    for (int i = 0; i < matrix_size; i++) {
        for (int j = 0; j < rows_per_process[rank]; j++) {
            local_transpose[i][j] = local_matrix[j * matrix_size + i];
        }
    }
    
    // REMOVE THE FOLLOWING COMMENT TO SEE IF THE LOCAL MATRIX IS CORRECTLY TRANSPOSED
    // for (int i = 0; i < size; i++) {
    //     if (rank == i) {
    //         printf("Rank %d\n", rank);
    //         for (int i = 0; i < matrix_size; i++) {
    //             for (int j = 0; j < rows_per_process[rank]; j++) {
    //                 printf("%6.2f", local_transpose[i][j]);
    //             }
    //             printf("\n");
    //         }
    //     }
    //     MPI_Barrier(MPI_COMM_WORLD);
    // }


    // ------------------------------------------------ //
    // -------- GATHERING PARTIAL TRANSPOSITION ------- //
    // ------------------------------------------------ //

    for(int i = 0; i < matrix_size; i++) {
        MPI_Gatherv(local_transpose[i], rows_per_process[rank], MPI_FLOAT, T[i], rows_per_process, gather_displs, MPI_FLOAT, 0, MPI_COMM_WORLD);
    }
}