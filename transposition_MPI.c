#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

void initializeMatrix(float *matrix, int n);
void printFlatMatrix(float *matrix, int n);
void printNormalMatrix(float **matrix, int n);
int matrix_actually_transposed(float *matrix, float **transpose, int n);

int main(int argc, char *argv[]) {

    /////////////////////////////////
    // INITIALIZATION OF VARIABLES //
    /////////////////////////////////
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        if (argc != 2) {
            printf("Please provide a matrix size as an argument.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        if (atoi(argv[1])%size != 0) {
            printf("Matrix size must be divisible by the number of processes.\n");
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
    int local_rows = matrix_size / size;

    if (rank == 0) {
        printf("The number of processes is %d, the number of rows per process is %d, and the matrix size is %d.\n", size, local_rows, matrix_size);
    }

    
    //////////////////////////
    // MATRICES ALLOCATIONS //
    //////////////////////////

    // Only rank 0 needs this matrix
    // Initial matrix
    float *M = NULL;
    float **T = (float **)malloc(matrix_size * sizeof(float *));

    if (rank == 0) {
        M = malloc(matrix_size * matrix_size * sizeof(float));
        for (int i = 0; i < matrix_size; i++) {
            T[i] = (float *)malloc(matrix_size * sizeof(float));
        }
        initializeMatrix(M, matrix_size);
    }

    //Every process need a local matrix, local transpose and the gathered matrix
    float *local_matrix = malloc(local_rows * matrix_size * sizeof(float));
    float **local_transpose = (float **)malloc(matrix_size * sizeof(float *));

    for (int i = 0; i < matrix_size; i++) {
        local_transpose[i] = (float *)malloc(local_rows * sizeof(float));
    }

    ///////////////////////////
    // SCATTERING THE MATRIX //
    ///////////////////////////
    MPI_Scatter(M, local_rows * matrix_size, MPI_FLOAT, local_matrix, local_rows * matrix_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);

    //Printing local matrices to see if they are correctly distributed
    for (int i = 0; i < size; i++) {
        if (rank == i) {
            printf("Rank %d\n", rank);
            for (int i = 0; i < local_rows; i++) {
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
        for (int j = 0; j < local_rows; j++) {
            local_transpose[i][j] = local_matrix[j * matrix_size + i];
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    
    //Printing local transposed matrices to see if they are correctly transposed
    for (int i = 0; i < size; i++) {
        if (rank == i) {
            printf("Rank %d\n", rank);
            for (int i = 0; i < matrix_size; i++) {
                for (int j = 0; j < local_rows; j++) {
                    printf("%6.2f", local_transpose[i][j]);
                }
                printf("\n");
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    printf("RANK %d PASSED BARRIER 1 \n", rank);

    /////////////////////////////////////
    // GATHERING PARTIAL TRANSPOSITION //
    /////////////////////////////////////
    for(int i = 0; i < matrix_size; i++) {
        MPI_Gather(local_transpose[i], local_rows , MPI_FLOAT, T[i], local_rows, MPI_FLOAT, 0, MPI_COMM_WORLD);
    }

    printf("RANK %d PASSED BARRIER 2 \n", rank);
    MPI_Barrier(MPI_COMM_WORLD);

    ///////////////////
    // CHECK PROCESS //
    ///////////////////
    
    if (rank == 0) {
        printf("RANK 0 REACHED CHECKING PART \n", rank);

        printf("Original Matrix:\n");
        printFlatMatrix(M, matrix_size);
        printf("Transposed Matrix:\n");
        printNormalMatrix(T, matrix_size);

        if (matrix_actually_transposed(M, T, matrix_size)) {
            printf("Matrix transposed successfully.\n");
        } else {
            printf("Matrix transposition failed.\n");
        }
    }
    
    MPI_Barrier(MPI_COMM_WORLD);

    //////////////////
    // FREE PROCESS //
    //////////////////
    printf("RANK %d REACED FREEING MEMORY PART \n", rank);

    for (int i = 0; i < matrix_size; i++) {
        free(local_transpose[i]);
    }
    free(local_transpose);
    free(local_matrix);


    if(rank == 0) {
        for (int i = 0; i < matrix_size; i++) {
            free(T[i]);
        }
        free(M);
    }

    free(T);

    //////////////////
    // MPI FINILIZE //
    //////////////////

    MPI_Finalize();
    return 0;
}

void initializeMatrix(float *matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i+j*n] = (float)rand() / RAND_MAX * 10.0f;
        }
    }
}

void printFlatMatrix(float *matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%6.2f ", matrix[i*n+j]);
        }
        printf("\n");
    }
}

void printNormalMatrix(float **matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%6.2f ", matrix[i][j]);
        }
        printf("\n");
    }
}

int matrix_actually_transposed(float *matrix, float **transpose, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (matrix[i*n+j] != transpose[j][i]) {
                return 0;
                printf("The value [%d][%d] in the original matrix is not equal to the value [%d][%d] in the transposed matrix.", i, j, j, i);
            }
        }
    }
    return 1;
}