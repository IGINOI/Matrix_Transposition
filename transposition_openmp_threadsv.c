#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif
#include <time.h>
#include <omp.h>


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //
// %%%%% FUNCTIONS DECLARATION %%%%%% //
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //

//initializes the matrix with random values
void initializeMatrix(float **matrix, int n);
//transposes the matrix
void matTranspose(float **matrix, float **transpose, int n);
//prints the matrix
void printMatrix(float **matrix, int n);
//checks if the matrix is actually transposed
int matrix_actually_transposed(float **matrix, float **transpose, int n);


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //
// %%%%%%%%% MAIN FUNCTION %%%%%%%%%% //
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //

int main(int argc, char *argv[]) {
    //Checking the number of arguments
    if (argc != 2) {
        printf("Please add a matrix size as an argument.\n");
        return 1;
    }

    //Checking the matrix size
    int exponent = atoi(argv[1]);
    if (exponent < 2 || exponent > 12) {
        printf("Matrix size exponent must be between 2 and 12 (recall that the base is 2).\n");
        return 1;
    }

    //Calculating the matrix size by shifting by the exponent
    int matrix_size = 1 << exponent;

    printf("Matrix size: %d\n", matrix_size);

    //Allocating memory for the matrices M and T
    float **M = (float **)malloc(matrix_size * sizeof(float *));
    float **T = (float **)malloc(matrix_size * sizeof(float *));
    for (int i = 0; i < matrix_size; i++) {
        M[i] = (float *)malloc(matrix_size * sizeof(float));
        T[i] = (float *)malloc(matrix_size * sizeof(float));
    }

    
    
    for(int n = 1; n <= 8; n++) {
        //Setting for the number of threads
        omp_set_num_threads(n);

        //Set the number of iterations to get a better average time
        int total_iterations = 50;
        double total_time = 0.0;

        for(int i = 0; i < total_iterations; i++) {
            // Initializing the completely casual matrix
            initializeMatrix(M, matrix_size);

            // Structure to store the time
            struct timeval start, end;
            long seconds, microseconds;
            double time_taken;

            // Transposing the matrix
            #ifdef _WIN32
                mingw_gettimeofday(&start, NULL);
            #else
                gettimeofday(&start, NULL);
            #endif

            matTranspose(M, T, matrix_size);
            
            #ifdef _WIN32
                mingw_gettimeofday(&end, NULL);
            #else
                gettimeofday(&end, NULL);
            #endif

            //Time elapsed calculation
            seconds = end.tv_sec - start.tv_sec;
            microseconds = end.tv_usec - start.tv_usec;
            time_taken = seconds + microseconds * 1e-6;
            total_time += time_taken;

            //CHECK SECTION - Uncomment to check the matrices
            // Check whether the matrix is actually transposed
            // printf("Matrix's actually transposed: %s\n", matrix_actually_transposed(M, T, matrix_size) ? "YES" : "NO");

            // printf("Original Matrix:\n");
            // printMatrix(M, matrix_size);

            // printf("Transposed Matrix:\n");
            // printMatrix(T, matrix_size);
        }

        double avg_time = total_time / total_iterations;
        printf("Matrix size: %d. Number of threads: %d. Average time taken: %.3fms\n", matrix_size, n, avg_time / 1e-3);
    }

    //Freeing memory
    for (int i = 0; i < matrix_size; i++) {
        free(M[i]);
        free(T[i]);
    }

    free(M);
    free(T);
    
    return 0;
}


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //
// %%%%%% FUNCTIONS DEFINITION %%%%%% //
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //

void initializeMatrix(float **matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i][j] = (float)rand();
        }
    }
}

void matTranspose(float **matrix, float **transpose, int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            transpose[j][i] = matrix[i][j];
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

int matrix_actually_transposed(float **matrix, float **transpose, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (matrix[i][j] != transpose[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
