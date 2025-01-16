#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <unistd.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //
// %%%%% FUNCTIONS DECLARATION %%%%%% //
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //

void initializeSymmetricMatrix(float **matrix, int n);
void printMatrix(float **matrix, int n);


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //
// %%%%%%%%% MAIN FUNCTION %%%%%%%%%% //
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //

int main(int argc, char *argv[]) {

    // ------------------------------------------------ //
    // ---------- ENVIRONMENT INITIALIZATION ---------- //
    // ------------------------------------------------ //
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Input validation
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

    // Matrix size computation
    int matrix_size = 1 << exponent;
    // Number of r
    int base_local_rows = matrix_size / size;

    
    // ------------------------------------------------ //
    //  VARIABLE COMPUTATION FOR SCATTERV AND GATHERV - //
    // ------------------------------------------------ //

    int *rows_per_process = malloc(size * sizeof(int));
    int *columns_per_process = malloc(size * sizeof(int));

    int *rows_elements_per_process = malloc(size * sizeof(int));
    int *columns_elements_per_process = malloc(size * sizeof(int));
    
    int *rows_scatter_displs = malloc(size * sizeof(int));
    int *columns_scatter_displs = malloc(size * sizeof(int));

    for (int i = 0; i < size; i++) {
        // Compute displacements for rows
        rows_per_process[i] = base_local_rows + ((i == size - 1) ? matrix_size % size : 0);
        rows_elements_per_process[i] = rows_per_process[i] * matrix_size;
        rows_scatter_displs[i] = (i == 0) ? 0 : rows_scatter_displs[i - 1] +  rows_per_process[i - 1]*matrix_size;
    }


    // ---
    // METHOD 1 -> MPI_Type_vector + Scatter
    // ---
    // MPI_Datatype column_type;
    // MPI_Type_vector(8, 1, 16, MPI_FLOAT, &column_type);
    // MPI_Type_commit(&column_type);

    // ---
    // METHOD 2 -> MPI_Type_create_subarray + Scatter
    // ---
    // MPI_Datatype column_type;
    // int sizes[2] = {16, 16}; // Size of the global array (rows, columns) 
    // int subsizes[2] = {16, 8}; // Size of the subarray (rows, columns) 
    // int starts[2] = {0, rank * rows_per_process[rank]}; // Starting point of the subarray (rows, columns)  
    // MPI_Type_create_subarray(2, sizes, subsizes, starts, MPI_ORDER_C, MPI_FLOAT, &column_type);
    // MPI_Type_commit(&column_type);

    // ---
    // METHOD 3 -> vector of MPI_Type_create_subarray + Scatter
    // ---
    // MPI_Datatype column_types[size];
    // int sizes[2] = {16, 16};
    // for (int i = 0; i < size; i++) { // Define the local subarray sizes (rows, columns) for each process 
    //     int subsizes[2] = {16, 8}; // Define the starting point of each subarray (rows, columns) for each process 
    //     int starts[2] = {0, i * 8}; // Create the subarray datatype for each process 
    //     MPI_Type_create_subarray(2, sizes, subsizes, starts, MPI_ORDER_C, MPI_FLOAT, &column_types[i]); 
    //     MPI_Type_commit(&column_types[i]); 
    // }


    


    // ------------------------------------------------ //
    // ------------- MATRICES ALLOCATIONS ------------- //
    // ------------------------------------------------ //

    // Matrices for only rank 0
    float *M_flat = NULL;
    float **M = (float **)malloc(matrix_size * sizeof(float *));

    if (rank == 0) {
        M_flat = malloc(matrix_size * matrix_size * sizeof(float));
        for (int i = 0; i < matrix_size; i++) {
            M[i] = (float *)malloc(matrix_size * sizeof(float));
        }
    }

    // Matrices for all the ranks
    float *local_rows = malloc(rows_per_process[rank] * matrix_size * sizeof(float));
    float *local_columns = malloc(rows_per_process[rank] * matrix_size * sizeof(float));

    
    // ------------------------------------------------ //
    // ------------ SCATTERING THE MATRIX ------------- //
    // ------------------------------------------------ //

    //for loop to compute an average time
    double total_time = 0.0;
    int iterations = 1;

    MPI_Barrier(MPI_COMM_WORLD);

    for(int i = 0; i < iterations; i++){

        if (rank == 0) {
            initializeSymmetricMatrix(M, matrix_size);
            for(int i = 0; i < matrix_size; i++) {
                for(int j = 0; j < matrix_size; j++) {
                    M_flat[i * matrix_size + j] = M[i][j];
                }
            }
        }

        //printing the matrix
        if(rank == 0) {
            printf("The original matrix is:\n");
            printMatrix(M, matrix_size);
        }
        
        MPI_Barrier(MPI_COMM_WORLD);
        // Process synchronization befor starting transposition
        double start_time = MPI_Wtime();


        MPI_Scatterv(M_flat, rows_elements_per_process, rows_scatter_displs, MPI_FLOAT, local_rows, rows_per_process[rank] * matrix_size, MPI_FLOAT, 0, MPI_COMM_WORLD);
        
        // ---
        // Method 1 -> MPI_Type_vector + Scatter
        // ---
        // Scattering around rows
        //MPI_Scatter(M_flat, 1, column_type, local_columns, rows_per_process[rank] * matrix_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

        // ---
        // Method 2 -> MPI_Type_create_subarray + Scatter
        // ---
        //MPI_Scatter(M_flat, 1, column_type, local_columns, rows_per_process[rank] * matrix_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

        // ---
        // Method 3 -> vector of MPI_Type_create_subarray + Scatter
        // ---
        // for (int i = 0; i < size; i++) { 
        //     MPI_Scatter(M_flat, 1, column_types[i], local_columns, rows_per_process[rank] * matrix_size, MPI_FLOAT, 0, MPI_COMM_WORLD); 
        // }



        
        if (rank == 0) {
            sleep(2);
            printf("\n\n\n");
            printf("Rank %d in row-major order\n", rank);
            
            for (int i = 0; i < rows_per_process[rank]; i++) {
                for (int j = 0; j < matrix_size; j++) {
                    printf("%6.2f", local_rows[i * matrix_size + j]);
                }
                printf("\n");
            }

            // for (int i = 0; i < 128; i++) {
            //     printf("%6.2f", local_rows[i]);
            // }

            sleep(2);
            printf("\n\n\n");
            printf("Rank %d in column-major order\n", rank);
            
            for (int i = 0; i < matrix_size; i++) {
                for (int j = 0; j < rows_per_process[rank]; j++) {
                    printf("%6.2f", local_columns[i * rows_per_process[rank] + j]);
                }
                printf("\n");
            } 

            // for (int i = 0; i < 128; i++) {
            //     printf("%6.2f", local_columns[i]);
            // }
            
            sleep(2);
            printf("\n\n\n");
        }

        MPI_Barrier(MPI_COMM_WORLD);

        if (rank == 1) {
            sleep(2);
            printf("\n\n\n");
            printf("Rank %d in row-major order\n", rank);
            
            for (int i = 0; i < rows_per_process[rank]; i++) {
                for (int j = 0; j < matrix_size; j++) {
                    printf("%6.2f", local_rows[i * matrix_size + j]);
                }
                printf("\n");
            }

            // for (int i = 0; i < 128; i++) {
            //     printf("%6.2f", local_rows[i]);
            // }


            sleep(2);
            printf("\n\n\n");
            printf("Rank %d in column-major order\n", rank);
            
            for (int i = 0; i < matrix_size; i++) {
                for (int j = 0; j < rows_per_process[rank]; j++) {
                    printf("%6.2f", local_columns[i * rows_per_process[rank] + j]);
                }
                printf("\n");
            } 

            // for (int i = 0; i < 128; i++) {
            //     printf("%6.2f", local_columns[i]);
            // }
            
            sleep(2);
            printf("\n\n\n");
        }

        MPI_Barrier(MPI_COMM_WORLD);




        // ------------------------------------------------ //
        // ---------- LOCAL MATRIX SYM CHECK -------------- //
        // ------------------------------------------------ //
        for (int i = 0; i < rows_per_process[rank]; i++) { 
            for (int j = 0; j < matrix_size; j++) { 
                if(local_rows[i * matrix_size + j] != local_columns[i * matrix_size + j]){ 
                    printf("From rank %d I got that local_rows_elements is: %6.2f and local_column element is: %6.2f \n", rank, local_rows[i * matrix_size + j], local_columns[j * columns_per_process[rank] + i]);
                    MPI_Abort(MPI_COMM_WORLD, 1); 
                } 
            } 
        }
        
        // Printing local transposed matrices to see if they are correctly transposed
        // for (int i = 0; i < size; i++) {
        //     if (rank == i) {
        //         printf("Rank %d\n", rank);
        //         for (int i = 0; i < matrix_size; i++) {
        //             for (int j = 0; j < rows_per_process[rank]; j++) {
        //                 printf("%6.2f", local_transpose[i][j]);
        //             }
        //             printf("\n");
        //         }
        //     }
        //     MPI_Barrier(MPI_COMM_WORLD);
        // }


        // ------------------------------------------------ //
        // -------- GATHERING PARTIAL TRANSPOSITION ------- //
        // ------------------------------------------------ //

        for(int i = 0; i < matrix_size; i++) {
            // Reducing the result with AND operation
        }

        // Synchronize after each repetition
        MPI_Barrier(MPI_COMM_WORLD);
        double end_time = MPI_Wtime();

        // Compute the total time and check correctness
        double elapsed_time = end_time - start_time;
        if (rank == 0) {
            total_time += elapsed_time;

            // Check for correct transposition
            // if (matrixActuallyTransposed(M, T, matrix_size)) {
            //     printf("Matrix transposed successfully.\n");
            // } else {
            //     printf("Matrix transposition failed.\n");
            // }
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }

    // ------------------------------------------------ //
    // -------------- TIME COMPUTATION ---------------- //
    // ------------------------------------------------ //
    if (rank == 0) {
        double average_time = total_time / iterations;
        printf("Average time for symmetry check: %f seconds\n", average_time);
    }

    // ------------------------------------------------ //
    // ----------------- FREE MEMORY ------------------ //
    // ------------------------------------------------ //

    free(rows_per_process);
    free(columns_per_process);
    free(rows_scatter_displs);
    free(columns_scatter_displs);
    free(rows_elements_per_process);
    free(columns_elements_per_process);

    free(local_rows);
    free(local_columns);

    if(rank == 0) {
        for (int i = 0; i < matrix_size; i++) {
            free(M[i]);
        }
        free(M_flat);
    }

    free(M);

    // ---
    // METHOD 1 -> MPI_Type_vector + Scatter
    // ---
    // MPI_Type_free(&column_type);

    // ---
    // METHOD 2 -> MPI_Type_create_subarray + Scatter
    // ---
    // MPI_Type_free(&column_type);

    // ---
    // METHOD 3 -> vector of MPI_Type_create_subarray + Scatter
    // ---
    // for (int i = 0; i < size; i++) {
    //     MPI_Type_free(&column_types[i]);
    // }

    MPI_Finalize();
    return 0;
}


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //
// %%%%%% FUNCTIONS DEFINITION %%%%%% //
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% //

void initializeSymmetricMatrix(float **matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            float x = (float)rand() / RAND_MAX * 10.0f;
            matrix[i][j] = x;
            matrix[j][i] = x;
            // matrix[i][j] = (float)rand() / RAND_MAX * 10.0f;
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