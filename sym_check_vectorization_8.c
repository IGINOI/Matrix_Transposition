#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif
#include <time.h>
#include <immintrin.h>

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //
// %%%%% FUNCTIONS DECLARATION %%%%%% //
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //

//initializes the matrix with random values and makes it symmetric
void initializeSymmetricMatrix(float **matrix, int n);
//checks if the matrix is symmetric
int checkSym(float **matrix, int n);
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
    for (int i = 0; i < matrix_size; i++) {
        M[i] = (float *)malloc(matrix_size * sizeof(float));
    }

    //Set the number of iterations to get a better average time
    int total_iterations = 50;
    double total_time = 0.0;

    for(int i = 0; i < total_iterations; i++) {
        //printf("Iteration %d \n", i);
        //Initializing the symmetric matrix
        initializeSymmetricMatrix(M, matrix_size); 

        // Check if the matrix is stored in row-major order
        // if (isRowMajor(M, matrix_size)) {
        //     printf("Matrix is stored in row-major order.\n");
        // } else {
        //     printf("Matrix is not stored in row-major order.\n");
        //     return 1;
        // }

        // Structure to store the time
        struct timeval start, end;
        long seconds, microseconds;
        double time_taken;

        //Checking matrix symmetry
        #ifdef _WIN32
            mingw_gettimeofday(&start, NULL);
        #else
            gettimeofday(&start, NULL);
        #endif
        
        int isSymmetric = checkSym(M, matrix_size);
        
        #ifdef _WIN32
            mingw_gettimeofday(&end, NULL);
        #else
            gettimeofday(&end, NULL);
        #endif

        //printf("Matrix is %s\n", isSymmetric ? "symmetric" : "not symmetric");

        //Time elapsed calculation
        seconds = end.tv_sec - start.tv_sec;
        microseconds = end.tv_usec - start.tv_usec;
        time_taken = seconds + microseconds / 1e6;
        total_time += time_taken;
    }
    
    double avg_time = total_time / total_iterations;
    printf("Average symmetry check time: %.3fms\n", avg_time * 1e3);

    // printf("Original Matrix:\n");
    // printMatrix(M, matrix_size);

    //Freeing memory
    for (int i = 0; i < matrix_size; i++) {
        free(M[i]);
    }
    free(M);
    
    return 0;
}


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //
// %%%%%% FUNCTIONS DEFINITION %%%%%% //
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //

int checkSym(float **matrix, int n) {
    int isSymmetric = 1;
    // Assuming matrix is row-major and a power of 2 for simplicity
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j += 8) { // Process 8 elements at a time
            // Load 8 elements from row i and column j
            __m256 row_vec = _mm256_loadu_ps(&matrix[i][j]);

            // Load 8 elements from column i and row j
            __m256 col_vec = _mm256_set_ps(matrix[j + 7][i], matrix[j + 6][i], matrix[j + 5][i], matrix[j + 4][i], matrix[j + 3][i], matrix[j + 2][i], matrix[j + 1][i], matrix[j][i]);

            // Compare row_vec and col_vec
            __m256 cmp = _mm256_cmp_ps(row_vec, col_vec, _CMP_NEQ_OQ);

            // Move the comparison result to an integer
            isSymmetric = _mm256_movemask_ps(cmp);

            // If any element mismatches, set isSymmetric = 0
            if (isSymmetric != 0) {
                return 0;
            }
        }
    }
    return 1;
}

void initializeSymmetricMatrix(float **matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j <= i; j++) {
            float value = (float)rand();
            matrix[i][j] = value;
            matrix[j][i] = value;
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

int isRowMajor(float **matrix, int n) {
    for (int i = 0; i < n; i++) {
        if (&matrix[i][1] != &matrix[i][0] + 1) { //checking whether memory addresses are contiguous
            return 0;
        }
    }
    return 1;
}