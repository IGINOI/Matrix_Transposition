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

    //Initializing the symmetric matrix
    initializeSymmetricMatrix(M, matrix_size); 

    //Structure to store the time
    struct timeval start, end;
    long seconds, useconds;
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

    seconds = end.tv_sec - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    time_taken = ((seconds) * 1000000 + useconds);
    printf("Symmetry check time: %f microseconds\n", time_taken);
    printf("Matrix is %s symmetric\n", isSymmetric ? "" : "not ");

    //printf("Original Matrix:\n");
    //printMatrix(matrix, matrix_size);


    //Freeing memory
    for (int i = 0; i < matrix_size; i++) {
        free(M[i]);
    }
    free(M);
    
    return 0;
}






int checkSym(float **matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (matrix[i][j] != matrix[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

void initializeSymmetricMatrix(float **matrix, int n) {
    for (int i = 0; i < n/2; i++) {
        for (int j = 0; j < n/2; j++) {
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