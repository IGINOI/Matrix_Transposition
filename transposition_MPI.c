#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

void initializeMatrix(float **matrix, int n);
void printMatrix(float **matrix, int n);
int matrixActuallyTransposed(float **matrix, float **transpose, int n);

int main(int argc, char *argv[]) {

    ////////////////////////////////
    // ENVIRONMENT INITIALIZATION //
    ////////////////////////////////
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
    if (exponent < 2 || exponent > 12) {
        if (rank == 0) {
            printf("Matrix size exponent must be between 4 and 12 (base is 2).\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Matrix size computation
    int matrix_size = 1 << exponent;
    // Number of rows per process
    int base_local_rows = matrix_size / size;

    // Print information about the environment
    if (rank == 0) {
        printf("The number of processes is %d, the number of rows per process is %d, and the matrix size is %d.\n", size, base_local_rows, matrix_size);
    }

    
    //////////////////////
    // ROWS COMPUTATION //
    //////////////////////

    int *rows_per_process = malloc(size * sizeof(int));
    int *displs = malloc(size * sizeof(int));
    int *gather_displs = malloc(size * sizeof(int));
    int *elements_per_process = malloc(size * sizeof(int));

    for (int i = 0; i < size; i++) {
        rows_per_process[i] = base_local_rows + ((i==size-1) ? matrix_size%size : 0);
        displs[i] = (i == 0) ? 0 : displs[i - 1] + rows_per_process[i - 1]*matrix_size;
        gather_displs[i] = (i == 0) ? 0 : gather_displs[i - 1] + rows_per_process[i - 1];
        elements_per_process[i] = rows_per_process[i] * matrix_size;
    }

    if (rank == 0) {
        for (int i = 0; i < size; i++) {
            printf("Process %d handles %d rows.\n", i, rows_per_process[i]);
            printf("Process %d starts at row %d.\n", i, displs[i]);
        }
    }


    //////////////////////////
    // MATRICES ALLOCATIONS //
    //////////////////////////

    // Only rank 0 needs this matrix
    // Initial matrix
    float *M_flat = NULL;
    float **M = (float **)malloc(matrix_size * sizeof(float *));
    float **T = (float **)malloc(matrix_size * sizeof(float *));

    if (rank == 0) {
        M_flat = malloc(matrix_size * matrix_size * sizeof(float));
        for (int i = 0; i < matrix_size; i++) {
            T[i] = (float *)malloc(matrix_size * sizeof(float));
            M[i] = (float *)malloc(matrix_size * sizeof(float));
        }
        initializeMatrix(M, matrix_size);
        for(int i=0; i<matrix_size; i++) {
            for(int j=0; j<matrix_size; j++) {
                M_flat[i*matrix_size+j] = M[i][j];
            }
        }
    }

    //Every process need a local matrix, local transpose and the gathered matrix
    float *local_matrix = malloc(rows_per_process[rank] * matrix_size * sizeof(float));

    float **local_transpose = (float **)malloc(matrix_size * sizeof(float *));
    for (int i = 0; i < matrix_size; i++) {
        local_transpose[i] = (float *)malloc(rows_per_process[rank] * sizeof(float));
    }

    ///////////////////////////
    // SCATTERING THE MATRIX //
    ///////////////////////////

    printf("Rank %d reached barrier 1\n", rank);

    MPI_Scatterv(M_flat, elements_per_process, displs, MPI_FLOAT, local_matrix, rows_per_process[rank] * matrix_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

    printf("Rank %d reached barrier 2\n", rank);

    MPI_Barrier(MPI_COMM_WORLD);

    //Printing local matrices to see if they are correctly distributed
    for (int i = 0; i < size; i++) {
        if (rank == i) {
            printf("Rank %d\n", rank);
            for (int i = 0; i < rows_per_process[rank]; i++) {
                for (int j = 0; j < matrix_size; j++) {
                    printf("%6.2f", local_matrix[i * matrix_size + j]);
                }
                printf("\n");
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }


    ////////////////////////////////
    // LOCAL MATRIX TRANSPOSITION //
    ////////////////////////////////
    for (int i = 0; i < matrix_size; i++) {
        for (int j = 0; j < rows_per_process[rank]; j++) {
            local_transpose[i][j] = local_matrix[j * matrix_size + i];
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    
    //Printing local transposed matrices to see if they are correctly transposed
    for (int i = 0; i < size; i++) {
        if (rank == i) {
            printf("Rank %d\n", rank);
            for (int i = 0; i < matrix_size; i++) {
                for (int j = 0; j < rows_per_process[rank]; j++) {
                    printf("%6.2f", local_transpose[i][j]);
                }
                printf("\n");
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }


    /////////////////////////////////////
    // GATHERING PARTIAL TRANSPOSITION //
    /////////////////////////////////////

    for(int i = 0; i < matrix_size; i++) {
        MPI_Gatherv(local_transpose[i], rows_per_process[rank], MPI_FLOAT, T[i], rows_per_process, gather_displs, MPI_FLOAT, 0, MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);


    ///////////////////
    // CHECK PROCESS //
    ///////////////////
    
    if (rank == 0) {
        printf("RANK 0 REACHED CHECKING PART \n", rank);

        printf("Original Matrix:\n");
        printMatrix(M, matrix_size);
        printf("Transposed Matrix:\n");
        printMatrix(T, matrix_size);

        if (matrixActuallyTransposed(M, T, matrix_size)) {
            printf("Matrix transposed successfully.\n");
        } else {
            printf("Matrix transposition failed.\n");
        }
    }
    
    MPI_Barrier(MPI_COMM_WORLD);

    //////////////////
    // FREE PROCESS //
    //////////////////

    for (int i = 0; i < matrix_size; i++) {
        free(local_transpose[i]);
    }
    free(local_transpose);
    free(local_matrix);

    free(rows_per_process);
    free(displs);
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

    //////////////////
    // MPI FINILIZE //
    //////////////////

    MPI_Finalize();
    return 0;
}

void initializeMatrix(float **matrix, int n) {
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