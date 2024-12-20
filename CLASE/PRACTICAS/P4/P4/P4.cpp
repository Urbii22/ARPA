#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

// Definir dimensiones de las matrices
#define M 4 // Número de filas
#define N 3 // Número de columnas

int main(int argc, char* argv[]) {
    int rank, size;
    int dims[2], periods[2] = { 0, 0 }, coords[2];
    MPI_Comm cart_comm;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;

    // Matrices y resultado
    int A[M][N];
    int B[M][N];
    int C[M][N];

    // Inicializar MPI
    MPI_Init(&argc, &argv);
    double start_time = MPI_Wtime(); // Iniciar temporizador

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Get_processor_name(processor_name, &name_len);

    // Verificar que el número de procesos coincida con M*N
    if (size != M * N) {
        if (rank == 0) {
            printf("Este programa requiere exactamente %d procesos.\n", M * N);
        }
        MPI_Finalize();
        exit(1);
    }

    // Crear una topología cartesiana 2D
    dims[0] = M; // Filas
    dims[1] = N; // Columnas
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &cart_comm);
    MPI_Cart_coords(cart_comm, rank, 2, coords);
    int fila = coords[0];
    int columna = coords[1];

    // Inicializar matrices A y B (solo el proceso raíz lo hace y luego lo distribuye)
    if (rank == 0) {
        printf("Inicializando matrices A y B...\n");
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                A[i][j] = i + j; // Por ejemplo, A[i][j] = i + j
                B[i][j] = i - j; // Por ejemplo, B[i][j] = i - j
            }
        }
    }

    // Distribuir las matrices A y B a todos los procesos
    // Usamos MPI_Scatter para distribuir filas
    // Sin embargo, cada proceso necesita un único elemento, por lo que es más eficiente usar MPI_Bcast y luego cada proceso selecciona su elemento
    MPI_Bcast(A, M * N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(B, M * N, MPI_INT, 0, MPI_COMM_WORLD);

    // Cada proceso suma su elemento correspondiente
    int elemento_A = A[fila][columna];
    int elemento_B = B[fila][columna];
    int elemento_C = elemento_A + elemento_B;

    // Recolectar los resultados en la matriz C en el proceso raíz
    C[fila][columna] = elemento_C;

    // Utilizar MPI_Gather para recolectar todos los elementos
    // Primero, reestructuramos la matriz C en un arreglo lineal
    int* C_linear = NULL;
    if (rank == 0) {
        C_linear = (int*)malloc(M * N * sizeof(int));
    }

    MPI_Gather(&elemento_C, 1, MPI_INT, C_linear, 1, MPI_INT, 0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime(); // Finalizar temporizador

    if (rank == 0) {
        // Reconstruir la matriz C desde el arreglo lineal
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                C[i][j] = C_linear[i * N + j];
            }
        }

        // Mostrar la matriz resultante
        printf("Matriz A:\n");
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                printf("%4d ", A[i][j]);
            }
            printf("\n");
        }

        printf("\nMatriz B:\n");
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                printf("%4d ", B[i][j]);
            }
            printf("\n");
        }

        printf("\nMatriz C (A + B):\n");
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                printf("%4d ", C[i][j]);
            }
            printf("\n");
        }

        printf("\nTiempo empleado: %f segundos\n", end_time - start_time);
        free(C_linear);
    }

    MPI_Finalize();
    return 0;
}
