#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int N = 20;
    // Si se quiere, se puede pasar N por línea de comandos:
    if (argc > 1) N = atoi(argv[1]);

    int* matrix = NULL;

    if (rank == 0) {
        // Proceso 0 inicializa la matriz
        matrix = (int*)malloc(N * N * sizeof(int));
        srand((unsigned int)time(NULL));
        for (int i = 0; i < N * N; i++) {
            matrix[i] = rand() % 101; // valores aleatorios entre 0 y 100
        }
    }

    // Difundir el valor de N a todos los procesos
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Suponemos que N es divisible entre size
    int rows_per_proc = N / size;
    int elements_per_proc = rows_per_proc * N;

    // Alojar espacio para el bloque local de cada proceso
    int* local_block = (int*)malloc(elements_per_proc * sizeof(int));

    // Repartir la matriz a todos los procesos
    MPI_Scatter(matrix, elements_per_proc, MPI_INT,
        local_block, elements_per_proc, MPI_INT,
        0, MPI_COMM_WORLD);

    // Calcular la suma parcial en cada proceso
    long local_sum = 0;
    for (int i = 0; i < elements_per_proc; i++) {
        local_sum += local_block[i];
    }

    // Cada proceso informa lo que ha hecho
    printf("Proceso %d: He sumado %d elementos. Suma parcial: %ld\n", rank, elements_per_proc, local_sum);
    fflush(stdout);

    // Sincronización para que los mensajes salgan ordenados (opcional)
    MPI_Barrier(MPI_COMM_WORLD);

    // Reducir las sumas parciales al proceso 0
    long global_sum = 0;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    // El total de elementos es N*N
    int global_count = N * N;

    // El proceso 0 calcula el promedio y muestra resultados
    if (rank == 0) {
        double promedio = (double)global_sum / global_count;

        // Imprimir la matriz
        printf("\nMatriz generada (%dx%d):\n", N, N);
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                printf("%3d ", matrix[i * N + j]);
            }
            printf("\n");
        }

        printf("\nEl promedio de todos los elementos es: %f\n", promedio);

        free(matrix);
    }
	// Liberamos la memoria local de cada proceso
    free(local_block);

	// Finalizar MPI
    MPI_Finalize();
    return 0;
}
