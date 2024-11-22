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

    //Set the number of iterations to get a better average time
    int total_iterations = 50;
    double total_time = 0.0;

    for(int i = 0; i < total_iterations; i++) {
        // Initializing the completely casual matrix
        initializeMatrix(M, matrix_size);

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
    const int blockSize = 8;
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

            //Subdivide each row in rows of 4 floats
            __m128 low_tmp0 = _mm256_castps256_ps128(tmp0);        // [0, 8, 1, 9]
            __m128 high_tmp0 = _mm256_extractf128_ps(tmp0, 1);     // [4, 12, 5, 13]
            __m128 low_tmp1 = _mm256_castps256_ps128(tmp1);        // [2, 10, 3, 11]
            __m128 high_tmp1 = _mm256_extractf128_ps(tmp1, 1);     // [6, 14, 7, 15]
            __m128 low_tmp2 = _mm256_castps256_ps128(tmp2);        // [16, 24, 17, 25]
            __m128 high_tmp2 = _mm256_extractf128_ps(tmp2, 1);     // [20, 28, 21, 29]
            __m128 low_tmp3 = _mm256_castps256_ps128(tmp3);        // [18, 26, 19, 27]
            __m128 high_tmp3 = _mm256_extractf128_ps(tmp3, 1);     // [22, 30, 23, 31]
            __m128 low_tmp4 = _mm256_castps256_ps128(tmp4);        // [32, 40, 33, 41]
            __m128 high_tmp4 = _mm256_extractf128_ps(tmp4, 1);     // [36, 44, 37, 45]
            __m128 low_tmp5 = _mm256_castps256_ps128(tmp5);        // [34, 42, 35, 43]
            __m128 high_tmp5 = _mm256_extractf128_ps(tmp5, 1);     // [38, 46, 39, 47]
            __m128 low_tmp6 = _mm256_castps256_ps128(tmp6);        // [48, 56, 49, 57]
            __m128 high_tmp6 = _mm256_extractf128_ps(tmp6, 1);     // [52, 60, 53, 61]
            __m128 low_tmp7 = _mm256_castps256_ps128(tmp7);        // [50, 58, 51, 59]
            __m128 high_tmp7 = _mm256_extractf128_ps(tmp7, 1);     // [54, 62, 55, 63]

            // Rearrange the tmp register by interleaving the inputs lower and higher parts
            __m128 col0low = _mm_movelh_ps(low_tmp0, low_tmp2);
            __m128 col0high = _mm_movelh_ps(low_tmp4, low_tmp6);
            __m128 col1low = _mm_movehl_ps(low_tmp2, low_tmp0);
            __m128 col1high = _mm_movehl_ps(low_tmp6, low_tmp4);
            __m128 col2low = _mm_movelh_ps(low_tmp1, low_tmp3);
            __m128 col2high = _mm_movelh_ps(low_tmp5, low_tmp7);
            __m128 col3low = _mm_movehl_ps(low_tmp3, low_tmp1);
            __m128 col3high = _mm_movehl_ps(low_tmp7, low_tmp5);
            __m128 col4low = _mm_movelh_ps(high_tmp0, high_tmp2);
            __m128 col4high = _mm_movelh_ps(high_tmp4, high_tmp6);
            __m128 col5low = _mm_movehl_ps(high_tmp2, high_tmp0);
            __m128 col5high = _mm_movehl_ps(high_tmp6, high_tmp4);
            __m128 col6low = _mm_movelh_ps(high_tmp1, high_tmp3);
            __m128 col6high = _mm_movelh_ps(high_tmp5, high_tmp7);
            __m128 col7low = _mm_movehl_ps(high_tmp3, high_tmp1);
            __m128 col7high = _mm_movehl_ps(high_tmp7, high_tmp5);
            
            // Merge together again the lower and higher parts of the columns
            __m256 col0 = _mm256_set_m128(col0high, col0low);
            __m256 col1 = _mm256_set_m128(col1high, col1low);   
            __m256 col2 = _mm256_set_m128(col2high, col2low);
            __m256 col3 = _mm256_set_m128(col3high, col3low);   
            __m256 col4 = _mm256_set_m128(col4high, col4low);
            __m256 col5 = _mm256_set_m128(col5high, col5low);
            __m256 col6 = _mm256_set_m128(col6high, col6low);
            __m256 col7 = _mm256_set_m128(col7high, col7low);

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

int isRowMajor(float **matrix, int n) {
    for (int i = 0; i < n; i++) {
        if (&matrix[i][1] != &matrix[i][0] + 1) { //checking whether memory addresses are contiguous
            return 0;
        }
    }
    return 1;
}