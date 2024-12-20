
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TAG_DATA 0
#define PRINT_LIMIT 15
#define DEBUGGING 1

// ZONA DE DECLARACI?N DE FUNCIONES
// Funciones de impresion
void imprimir_matriz(float** matriz, int filas, int columnas, const char* nombre);
void imprimir_matriz_parcial(float** matriz, int filas, int columnas, int inicio, const char* nombre);
void calcular_rango_filas(int rank, int filas_por_proceso, int extra, int* start_p, int* end_p, int* num_filas_p);
void debugger(int N, int rank, float** matriz, int num_filas, int inicio, const char* nombre1, const char* nombre2);

int main(int argc, char* argv[]) {
    int rank, size;
    int N; // Tamanyo de las matrices

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Definir tamanyo de matrices
    if (rank == 0) {
        printf("Ingrese el tama?o de las matrices (N): ");
        fflush(stdout);
        scanf_s("%d", &N);
    }
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);


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

        // Opcional: Imprimir matrices A y B si N es pequenyo
        if (N <= PRINT_LIMIT) { // Ajusta este limite segun tus necesidades
            imprimir_matriz(A, N, N, "A");
            imprimir_matriz(B, N, N, "B");
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

    // **Depuracion**: Imprimir las porciones de A_local en cada proceso
    if (DEBUGGING == 1) {
        debugger(N, rank, A_local, num_filas, inicio, "A_local", "A_local (Proceso 0)");
    }

    // Inicio de temporizacion
    double start_time = 0.0, end_time;
    if (rank == 0) {
        start_time = MPI_Wtime();
    }

    // Multiplicacion de matrices
    for (int i = 0; i < num_filas; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {
                C_local[i][j] += A_local[i][k] * B[k][j];
            }
        }
    }

    // **Depuracion**: Imprimir las porciones de C_local en cada proceso
    if (DEBUGGING == 1) {
        debugger(N, rank, C_local, num_filas, inicio, "C_local", "C_local (Proceso 0)");
    }

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
            printf("Recibido de proceso %d con etiqueta %d\n", status.MPI_SOURCE, status.MPI_TAG);

            // **Depuracion**: Imprimir las filas recibidas
            if (DEBUGGING == 1) {
                if (N <= PRINT_LIMIT) { // Solo para matrices pequenyas
                    printf("Filas %d a %d recibidas de proceso %d:\n", start_p, end_p - 1, status.MPI_SOURCE);
                    for (int i = start_p; i < end_p; i++) {
                        for (int j = 0; j < N; j++) {
                            printf("%0.2f\t", C[i][j]);
                        }
                        printf("\n");
                    }
                    printf("\n");
                }
            }
        }

        // Fin de temporizacion
        end_time = MPI_Wtime();
        printf("Tiempo de ejecuci?n: %f segundos\n", end_time - start_time);

        // Opcional: Imprimir la matriz resultante C si N es pequenyo
        if (N <= PRINT_LIMIT) {
            imprimir_matriz(C, N, N, "C");
        }
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

// ZONA DE DEFINICI?N DE FUNCIOES
// Implementacion de las funciones de impresion
void imprimir_matriz(float** matriz, int filas, int columnas, const char* nombre) {
    printf("Matriz %s:\n", nombre);
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            printf("%0.2f\t", matriz[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void imprimir_matriz_parcial(float** matriz, int filas, int columnas, int inicio, const char* nombre) {
    printf("Matriz %s (Filas %d a %d):\n", nombre, inicio, inicio + filas - 1);
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            printf("%0.2f\t", matriz[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Implementaci?n de la funci?n de utilidad para calcular el rango de filas
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

// Funci?n que muestra el debugging del proceso
void debugger(int N, int rank, float** matriz, int num_filas, int inicio, const char* nombre1, const char* nombre2) {
    // **Depuracion**: Imprimir las porciones de la matrices en cada proceso
    if (N <= PRINT_LIMIT) { // Solo para matrices pequenyas
        MPI_Barrier(MPI_COMM_WORLD); // Sincronizar procesos
        if (rank != 0) {
            imprimir_matriz_parcial(matriz, num_filas, N, inicio, nombre1);
        }
        else {
            imprimir_matriz_parcial(matriz, num_filas, N, inicio, nombre2);
        }
    }
}