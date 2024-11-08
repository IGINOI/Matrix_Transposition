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
    const int blockSize = 8;  // Block size for AVX (4x4 block)

    for (int i = 0; i < n; i += blockSize) {
        for (int j = 0; j < n; j += blockSize) {

            // %%%%%%%%%%%% to compile with flag -mavx2 %%%%%%%%%%%%

            // loads 4 floats at time into row0, row1, row2, row3 --m256 registers
            __m256 row0 = _mm256_loadu_ps(&matrix[i][j]);  //row0 = (matrix[i][j], matrix[i][j+1], matrix[i][j+2], matrix[i][j+3], matrix[i][j+4], matrix[i][j+5], matrix[i][j+6], matrix[i][j+7])
            __m256 row1 = _mm256_loadu_ps(&matrix[i + 1][j]);
            __m256 row2 = _mm256_loadu_ps(&matrix[i + 2][j]);
            __m256 row3 = _mm256_loadu_ps(&matrix[i + 3][j]);
            __m256 row4 = _mm256_loadu_ps(&matrix[i + 4][j]);
            __m256 row5 = _mm256_loadu_ps(&matrix[i + 5][j]);
            __m256 row6 = _mm256_loadu_ps(&matrix[i + 6][j]);
            __m256 row7 = _mm256_loadu_ps(&matrix[i + 7][j]);

            // Rearrange the rows, by interleaving the inputs lower and higher parts
            __m256 tmp0 = _mm256_unpacklo_ps(row0, row1);  //tmp0 will contain the lower part of row0 and row1 interleaved
            __m256 tmp1 = _mm256_unpackhi_ps(row0, row1);  //tmp1 will contain the higher part of row0 and row1 interleaved
            __m256 tmp2 = _mm256_unpacklo_ps(row2, row3);
            __m256 tmp3 = _mm256_unpackhi_ps(row2, row3);
            __m256 tmp4 = _mm256_unpacklo_ps(row4, row5);
            __m256 tmp5 = _mm256_unpackhi_ps(row4, row5);
            __m256 tmp6 = _mm256_unpacklo_ps(row6, row7);
            __m256 tmp7 = _mm256_unpackhi_ps(row6, row7);



            // Rearrange the tmp register by interleaving the inputs lower and higher parts
            __m256 col0 = _mm256_permute2f128_ps(tmp0, tmp2, 0x20);  //col0 will contain, in order, the lower part of tmp0 and lower part of tmp2
            __m256 col1 = _mm256_permute2f128_ps(tmp2, tmp0, 0x20);  //col1 will contain, in order, the higher part of tmp0 and the higher part of tmp2
            __m256 col2 = _mm256_permute2f128_ps(tmp1, tmp3, 0x31);
            __m256 col3 = _mm256_permute2f128_ps(tmp3, tmp1, 0x31);
            __m256 col4 = _mm256_permute2f128_ps(tmp4, tmp6, 0x20);
            __m256 col5 = _mm256_permute2f128_ps(tmp6, tmp4, 0x20);
            __m256 col6 = _mm256_permute2f128_ps(tmp5, tmp7, 0x31);
            __m256 col7 = _mm256_permute2f128_ps(tmp7, tmp5, 0x31);


            // Store transposed block in the output matrix
            _mm256_storeu_ps(&transpose[j][i], col0);      // Store col0 in transpose[j][i]
            _mm256_storeu_ps(&transpose[j + 1][i], col1);
            _mm256_storeu_ps(&transpose[j + 2][i], col2);
            _mm256_storeu_ps(&transpose[j + 3][i], col3);
            _mm256_storeu_ps(&transpose[j + 4][i], col4);
            _mm256_storeu_ps(&transpose[j + 5][i], col5);
            _mm256_storeu_ps(&transpose[j + 6][i], col6);
            _mm256_storeu_ps(&transpose[j + 7][i], col7);
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