#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

void initializeMatrix(float *matrix, int n);
void printMatrix(float *matrix, int n);
int matrix_actually_transposed(float *matrix, float *transpose, int n);

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

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

    int matrix_size = 1 << exponent;
    int local_rows = matrix_size / size;

    

    float *M = NULL;
    float *T = NULL;
    float *TT = NULL;

    if (rank == 0) {
        printf("The number of processes is %d, the number of rows per process is %d, and the matrix size is %d.\n", size, local_rows, matrix_size);
        M = malloc(matrix_size * matrix_size * sizeof(float));
        T = malloc(matrix_size * matrix_size * sizeof(float));
        TT = malloc(matrix_size * matrix_size * sizeof(float));
        initializeMatrix(M, matrix_size);
    }

    float *local_matrix = malloc(local_rows * matrix_size * sizeof(float));
    float *local_transpose = malloc(local_rows * matrix_size * sizeof(float));

    MPI_Scatter(M, local_rows * matrix_size, MPI_FLOAT, local_matrix, local_rows * matrix_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);

    //Printing local matrices to see if they are correctly distributed
    for (int i = 0; i < size; i++) {
        if (rank == i) {
            printf("Rank %d\n", rank);
            for (int i = 0; i < local_rows; i++) {
                for (int j = 0; j < matrix_size; j++) {
                    printf("%6.2f", local_matrix[i*matrix_size+j]);
                }
                printf("\n");
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    // Transpose locally
    for (int i = 0; i < local_rows; i++) {
        for (int j = 0; j < matrix_size; j++) {
            local_transpose[j * local_rows + i] = local_matrix[i * matrix_size + j];
        }
    }

    //Printing local transposed matrices to see if they are correctly transposed
    for (int i = 0; i < size; i++) {
        if (rank == i) {
            printf("Rank %d\n", rank);
            for (int i = 0; i < matrix_size; i++) {
                for (int j = 0; j < local_rows; j++) {
                    printf("%6.2f", local_transpose[j+local_rows*i]);
                }
                printf("\n");
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    printf("Rank %d is at step 1\n", rank);

    MPI_Gather(local_transpose, local_rows * matrix_size, MPI_FLOAT, T, local_rows * matrix_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

    printf("Rank %d is at step 2\n", rank);

    if (rank == 0) {
        printf("Original Matrix:\n");
        printMatrix(M, matrix_size);
        printf("Unordered Matrix:\n");
        printMatrix(T, matrix_size);
        printf("Transposed Matrix:\n");
        printMatrix(TT, matrix_size);

        for (int i = 0; i < matrix_size; ++i) {
            for (int j = 0; j < matrix_size; ++j) {
                TT[i * matrix_size + j] = T[j * matrix_size + i];
            }
        }

        if (matrix_actually_transposed(M, TT, matrix_size)) {
            printf("Matrix transposed successfully.\n");
        } else {
            printf("Matrix transposition failed.\n");
        }
        free(M);
        free(T);
        free(TT);
    }

    free(local_matrix);
    free(local_transpose);

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

void printMatrix(float *matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%6.2f ", matrix[i*n+j]);
        }
        printf("\n");
    }
}

int matrix_actually_transposed(float *matrix, float *transpose, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (matrix[i*n+j] != transpose[j*n+i]) {
                return 0;
            }
        }
    }
    return 1;
}