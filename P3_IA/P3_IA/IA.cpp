#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]) {
    int mirango, tamano;
    int longitud;
    char nombre[32];
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mirango);
    MPI_Comm_size(MPI_COMM_WORLD, &tamano);
    MPI_Get_processor_name(nombre, &longitud);

    int n = tamano - 1; // Size of the vectors (adjusted based on number of processes)
    int* x = NULL; // Vectors x and y
    int* y = NULL;

    start_time = MPI_Wtime();

    if (mirango == 0) {
        // Process 0 initializes the vectors
        x = new int[n];
        y = new int[n];

        // Initialize the vectors with random integers between 0 and 100
        for (int i = 0; i < n; i++) {
            x[i] = rand() % 101;
            y[i] = rand() % 101;
        }

        // Print the generated vectors
        printf("Vector x: ");
        for (int i = 0; i < n; i++) {
            printf("%d ", x[i]);
        }
        printf("\n");

        printf("Vector y: ");
        for (int i = 0; i < n; i++) {
            printf("%d ", y[i]);
        }
        printf("\n");
    }

    int local_x, local_y; // Local elements of x and y

    // Scatter the elements of x and y to each process
    MPI_Scatter(x, 1, MPI_INT, &local_x, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(y, 1, MPI_INT, &local_y, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Calculate the local product
    int local_product = local_x * local_y;

    // Gather the local products to process 0
    int* products = NULL;
    if (mirango == 0) {
        products = new int[n];
    }
    MPI_Gather(&local_product, 1, MPI_INT, products, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (mirango == 0) {
        // Process 0 calculates the dot product
        int dot_product = 0;
        for (int i = 0; i < n; i++) {
            dot_product += products[i];
        }

        // Measure the execution time
        
        end_time = MPI_Wtime();

        printf("Dot product: %d\n", dot_product);
        printf("Execution time: %f seconds\n", end_time - start_time);

        delete[] x;
        delete[] y;
        delete[] products;
    }

    MPI_Finalize();
    return 0;
}