#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void initializeMatrix(float **matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i][j] = (float)rand();
        }
    }
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

void matTranspose(float **matrix, float **transpose, int n) {
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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Please add a matrix size as an argument.\n");
        return 1;
    }

    int n = atoi(argv[1]);
    if (n <= 0) {
        printf("Matrix size must be a positive integer.\n");
        return 1;
    }

    printf("Matrix size: %d\n", n);

    float **matrix = (float **)malloc(n * sizeof(float *));
    float **transpose = (float **)malloc(n * sizeof(float *));
    for (int i = 0; i < n; i++) {
        matrix[i] = (float *)malloc(n * sizeof(float));
        transpose[i] = (float *)malloc(n * sizeof(float));
    }

    srand(time(NULL));
    initializeMatrix(matrix, n);

    clock_t start, end;
    double cpu_time_used;

    start = clock();
    int isSymmetric = checkSym(matrix, n);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Symmetry check time: %f seconds\n", cpu_time_used);
    printf("Matrix is %ssymmetric\n", isSymmetric ? "" : "not ");

    start = clock();
    matTranspose(matrix, transpose, n);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Matrix transposition time: %f seconds\n", cpu_time_used);

    for (int i = 0; i < n; i++) {
        free(matrix[i]);
        free(transpose[i]);
    }
    free(matrix);
    free(transpose);
    printf("Original Matrix:\n");
    //printMatrix(matrix, n);

    printf("Transposed Matrix:\n");
    //printMatrix(transpose, n);
    return 0;
}