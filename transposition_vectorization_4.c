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

//initializes the matrix with random values
void initializeMatrix(float **matrix, int n);
//transposes the matrix
void matTranspose(float **matrix, float **transpose, int n);
//prints the matrix
void printMatrix(float **matrix, int n);
//checks if the matrix is stored in row-major order
int isRowMajor(float **matrix, int n);
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
    if (exponent < 4 || exponent > 12) {
        printf("Matrix size exponent must be between 4 and 12 (recall that the base is 2).\n");
        return 1;
    }

    //Calculating the matrix size by shifting by the exponent
    int matrix_size = 1 << exponent;

    //Allocating memory for the matrices M and T
    float **M = (float **)malloc(matrix_size * sizeof(float *));
    float **T = (float **)malloc(matrix_size * sizeof(float *));
    for (int i = 0; i < matrix_size; i++) {
        M[i] = (float *)malloc(matrix_size * sizeof(float));
        T[i] = (float *)malloc(matrix_size * sizeof(float));
    }

    //Set the number of iterations to get a better average time
    int total_iterations = 50;
    double total_time = 0.0;

    for(int i = 0; i < total_iterations; i++) {
        // Initializing the completely casual matrix
        initializeMatrix(M, matrix_size);

        // Check if the matrix is stored in row-major order
        if (!isRowMajor(M, matrix_size)) {
            printf("Matrix is not stored in row-major order.\n");
            return 1;
        }

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
    }

    double avg_time = total_time / total_iterations;
    printf("Matrix size: %d x %d. Average time taken: %.3fms\n", matrix_size, matrix_size, avg_time / 1e-3);

    // printf("Original Matrix:\n");
    // printMatrix(M, matrix_size);

    // printf("Transposed Matrix:\n");
    // printMatrix(T, matrix_size);


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

void matTranspose(float** matrix, float** transpose, int n) {
    const int blockSize = 4;
    for (int i = 0; i < n; i += blockSize) {
        for (int j = 0; j < n; j += blockSize) {
            // loads 4 floats at time into row0, row1, row2, row3 --m128 registers
            __m128 row0 = _mm_loadu_ps(&matrix[i][j]);  //row0 = (matrix[i][j], matrix[i][j+1], matrix[i][j+2], matrix[i][j+3])
            __m128 row1 = _mm_loadu_ps(&matrix[i + 1][j]);
            __m128 row2 = _mm_loadu_ps(&matrix[i + 2][j]);
            __m128 row3 = _mm_loadu_ps(&matrix[i + 3][j]);

            // Rearrange the rows, by interleaving the inputs lower and higher parts
            __m128 tmp0 = _mm_unpacklo_ps(row0, row1);  //tmp0 will contain the lower part of row0 and row1 interleaved
            __m128 tmp1 = _mm_unpackhi_ps(row0, row1);  //tmp1 will contain the higher part of row0 and row1 interleaved
            __m128 tmp2 = _mm_unpacklo_ps(row2, row3);
            __m128 tmp3 = _mm_unpackhi_ps(row2, row3);


            // Rearrange the tmp register by interleaving the inputs lower and higher parts
            __m128 col0 = _mm_movelh_ps(tmp0, tmp2);  //col0 will contain, in order, the lower part of tmp0 and lower part of tmp2
            __m128 col1 = _mm_movehl_ps(tmp2, tmp0);  //col1 will contain, in order, the higher part of tmp0 and the higher part of tmp2
            __m128 col2 = _mm_movelh_ps(tmp1, tmp3);
            __m128 col3 = _mm_movehl_ps(tmp3, tmp1);

            // Store transposed block in the output matrix
            _mm_storeu_ps(&transpose[j][i], col0);      // Store col0 in transpose[j][i]
            _mm_storeu_ps(&transpose[j + 1][i], col1);
            _mm_storeu_ps(&transpose[j + 2][i], col2);
            _mm_storeu_ps(&transpose[j + 3][i], col3);
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
