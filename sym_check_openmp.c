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
    if (exponent < 4 || exponent > 12) {
        printf("Matrix size exponent must be between 4 and 12 (recall that the base is 2).\n");
        return 1;
    }

    //Calculating the matrix size by shifting by the exponent
    int matrix_size = 1 << exponent;

    //Allocating memory for the matrices M and T
    float **M = (float **)malloc(matrix_size * sizeof(float *));
    for (int i = 0; i < matrix_size; i++) {
        M[i] = (float *)malloc(matrix_size * sizeof(float));
    }

    //Setting the number of threads
    int number_of_threads = 8;
    omp_set_num_threads(number_of_threads);

    //Set the number of iterations to get a better average time
    int total_iterations = 50;
    double total_time = 0.0;

    for(int i = 0; i < total_iterations; i++) {
        //printf("Iteration %d \n", i);
        //Initializing the symmetric matrix
        initializeSymmetricMatrix(M, matrix_size); 

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
    printf("Matrix size: %d x %d. Threads number: %d. Average time taken: %.3fms\n", matrix_size, matrix_size, number_of_threads, avg_time / 1e-3);

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
    #pragma omp parallel for reduction(&&:isSymmetric) schedule(static, 8)
    for (int i = 0; i < n; i++) {
        // #pragma omp simd
        for (int j = 0; j < n; j++) {
            if (matrix[i][j] != matrix[j][i]) {
                #pragma omp atomic write
                isSymmetric = 0;
            }
        }
    }
    return isSymmetric;
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