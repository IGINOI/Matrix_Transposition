#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif
#include <time.h>


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //
// %%%%% FUNCTIONS DECLARATION %%%%%% //
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //

//initializes the matrix with random values
void initializeMatrix(float **matrix, int n);
//transposes the matrix
void matTranspose(float **matrix, float **transpose, int n);
//prints the matrix
void printMatrix(float **matrix, int n);


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

        seconds = end.tv_sec - start.tv_sec;
        microseconds = end.tv_usec - start.tv_usec;
        time_taken = seconds + microseconds * 1e-6;
        //printf("Matrix transposition time: %.3fms\n", time_taken / 1e-3);

        total_time += time_taken;
    }

    double avg_time = total_time / total_iterations;
    printf("Average matrix transposition time: %.3fms\n", avg_time / 1e-3);

    //printf("Original Matrix:\n");
    //printMatrix(M, matrix_size);

    //printf("Transposed Matrix:\n");
    //printMatrix(T, matrix_size);


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
    const int blockSize = 8; // Block size for cache efficiency
    
    //Iterating over the blocks of elements. 
    //For each iteration we work over a chunk of rows and columns
    for (int i = 0; i < n; i += 1) {
        int j;
        for (j = 0; j < n - blockSize + 1; j += blockSize) {
            transpose[j][i] = matrix[i][j];
            transpose[j + 1][i] = matrix[i][j + 1];
            transpose[j + 2][i] = matrix[i][j + 2];
            transpose[j + 3][i] = matrix[i][j + 3];
            transpose[j + 4][i] = matrix[i][j + 4];
            transpose[j + 5][i] = matrix[i][j + 5];
            transpose[j + 6][i] = matrix[i][j + 6];
            transpose[j + 7][i] = matrix[i][j + 7];
        }
        for (; j < n; j++) {
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