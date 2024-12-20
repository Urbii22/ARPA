#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TAG_DATA 0

// ZONA DE DECLARACIÓN DE FUNCIONES
// Funciones de impresion
void calcular_rango_filas(int rank, int filas_por_proceso, int extra, int* start_p, int* end_p, int* num_filas_p);


int main(int argc, char* argv[]) {
    int rank, size;
    int N; // Tamanyo de las matrices

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Lee el tamaño de matriz por línea de comandos
    N = atoi(argv[1]);

    // Reservar memoria para matriz B en todos los procesos
    float** B = (float**)malloc(N * sizeof(float*));
    float* B_f = (float*)malloc(N * N * sizeof(float));
    for (int i = 0; i < N; i++) {
        B[i] = B_f + i * N;
    }

    // Reserva y asignacion para A y C en proceso 0
    float** A = NULL, ** C = NULL;
    float* A_f = NULL, * C_f = NULL;

    if (rank == 0) {
        // Reservar memoria para A
        A = (float**)malloc(N * sizeof(float*));
        A_f = (float*)malloc(N * N * sizeof(float));
        for (int i = 0; i < N; i++) {
            A[i] = A_f + i * N;
        }

        // Reservar memoria para C
        C = (float**)malloc(N * sizeof(float*));
        C_f = (float*)malloc(N * N * sizeof(float));
        for (int i = 0; i < N; i++) {
            C[i] = C_f + i * N;
            for (int j = 0; j < N; j++) {
                C[i][j] = 0.0;
            }
        }

        // Inicializar A y B con valores aleatorios
        srand(time(NULL));
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                A[i][j] = ((float)rand()) / RAND_MAX;
                B[i][j] = ((float)rand()) / RAND_MAX;
            }
        }

    }

    // Broadcast de la matriz B a todos los procesos
    MPI_Bcast(&(B[0][0]), N * N, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // Cada proceso necesita tener su propia copia local de A
    int filas_por_proceso = N / size;
    int extra = N % size;
    int inicio, fin, num_filas;

    calcular_rango_filas(rank, filas_por_proceso, extra, &inicio, &fin, &num_filas);

    // Reserva para A_local
    float** A_local = (float**)malloc(num_filas * sizeof(float*));
    float* A_local_f = (float*)malloc(num_filas * N * sizeof(float));
    for (int i = 0; i < num_filas; i++) {
        A_local[i] = A_local_f + i * N;
    }

    // Reserva para C_local
    float** C_local = (float**)malloc(num_filas * sizeof(float*));
    float* C_local_f = (float*)malloc(num_filas * N * sizeof(float));
    for (int i = 0; i < num_filas; i++) {
        C_local[i] = C_local_f + i * N;
        for (int j = 0; j < N; j++) {
            C_local[i][j] = 0.0;
        }
    }

    // Enviar las filas de A a cada proceso
    if (rank == 0) {
        for (int p = 1; p < size; p++) {
            int start_p, end_p, num_filas_p;
            calcular_rango_filas(p, filas_por_proceso, extra, &start_p, &end_p, &num_filas_p);
            MPI_Send(&(A[start_p][0]), num_filas_p * N, MPI_FLOAT, p, TAG_DATA, MPI_COMM_WORLD);
        }
        // Copiar las filas para el proceso 0
        for (int i = 0; i < num_filas; i++) {
            for (int j = 0; j < N; j++) {
                A_local[i][j] = A[i + inicio][j];
            }
        }
    }
    else {
        // Recibir las filas de A
        MPI_Recv(&(A_local[0][0]), num_filas * N, MPI_FLOAT, 0, TAG_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    //Mido el tiempo antes de empezar la multiplicación
    double start_time = MPI_Wtime();

    // Multiplicacion de matrices
    for (int i = 0; i < num_filas; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {
                C_local[i][j] += A_local[i][k] * B[k][j];
            }
        }
    }

    //Mido el tiempo despúes de empezar la multiplicación
    double end_time = MPI_Wtime();

    // Enviar resultados de C_local al proceso 0
    if (rank != 0) {
        MPI_Send(&(C_local[0][0]), num_filas * N, MPI_FLOAT, 0, TAG_DATA, MPI_COMM_WORLD);
    }
    else {
        // Copiar las filas calculadas por el proceso 0
        for (int i = 0; i < num_filas; i++) {
            for (int j = 0; j < N; j++) {
                C[i + inicio][j] = C_local[i][j];
            }
        }

        // Recibir resultados de otros procesos
        for (int p = 1; p < size; p++) {
            int start_p, end_p, num_filas_p;
            calcular_rango_filas(p, filas_por_proceso, extra, &start_p, &end_p, &num_filas_p);
            MPI_Status status;
            //MPI_Recv(&(C[start_p][0]), num_filas_p * N, MPI_FLOAT, MPI_ANY_SOURCE, TAG_DATA, MPI_COMM_WORLD, &status);
            MPI_Recv(&(C[start_p][0]), num_filas_p * N, MPI_FLOAT, p, TAG_DATA, MPI_COMM_WORLD, &status);
        }

        // Imprimo la duración de la multiplicación
        printf("Número de procesos %d\n", size);
        printf("Tiempo de ejecución: %f segundos\n", end_time - start_time);
    }

    // Liberar memoria
    if (rank == 0) {
        free(A_f);
        free(A);
        free(C_f);
        free(C);
    }

    free(A_local_f);
    free(A_local);
    free(C_local_f);
    free(C_local);
    free(B_f);
    free(B);

    MPI_Finalize();
    return 0;
}

// ZONA DE DEFINICIÓN DE FUNCIOES
// Implementación de la función de utilidad para calcular el rango de filas
void calcular_rango_filas(int rank, int filas_por_proceso, int extra, int* start_p, int* end_p, int* num_filas_p) {

    if (rank < extra) {
        *start_p = rank * (filas_por_proceso + 1);
        *end_p = *start_p + filas_por_proceso + 1;
    }
    else {
        *start_p = rank * filas_por_proceso + extra;
        *end_p = *start_p + filas_por_proceso;
    }
    *num_filas_p = *end_p - *start_p;
}