#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Función para imprimir una matriz
void imprimir_matriz(const char* nombre, int* matriz, int N) {
    printf("%s:\n", nombre);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d ", matriz[i * N + j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    int rango, tamano;
    int longitud;
    char nombre[MPI_MAX_PROCESSOR_NAME];

    // Inicialización de MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rango);
    MPI_Comm_size(MPI_COMM_WORLD, &tamano);
    MPI_Get_processor_name(nombre, &longitud);

    // Verifica que haya al menos 2 procesos (1 master + 1 trabajador)
    if (tamano < 2) {
        if (rango == 0) {
            printf("Error: Se requieren al menos 2 procesos (1 master + 1 trabajador).\n");
        }
        MPI_Finalize();
        return 0;
    }

    int N = tamano - 1; // Tamaño de las matrices

    // Proceso master genera las matrices A y B
    int* A = NULL;
    int* B = NULL;
    int* C = NULL;

    if (rango == 0) {
        A = (int*)malloc(N * N * sizeof(int));
        B = (int*)malloc(N * N * sizeof(int));
        C = (int*)malloc(N * N * sizeof(int));

        // Inicializa las matrices A y B con valores aleatorios
        srand(time(NULL));
        for (int i = 0; i < N * N; i++) {
            A[i] = rand() % 10; // Valores entre 0 y 9
            B[i] = rand() % 10;
        }

        // Imprime las matrices A y B
        printf("[Master] Generando matrices A y B...\n");
        imprimir_matriz("Matriz A", A, N);
        imprimir_matriz("Matriz B", B, N);
    }

    // Sincroniza todos los procesos
    MPI_Barrier(MPI_COMM_WORLD);

    double inicio, fin;

    if (rango == 0) {
        // Inicia el temporizador
        inicio = MPI_Wtime();
    }

    // Preparar para MPI_Scatterv y MPI_Gatherv
    // Definir sendcounts y displs para Scatterv
    int* sendcounts_A = NULL;
    int* displs_A = NULL;
    int* sendcounts_B = NULL;
    int* displs_B = NULL;

    if (rango == 0) {
        sendcounts_A = (int*)malloc(tamano * sizeof(int));
        displs_A = (int*)malloc(tamano * sizeof(int));
        sendcounts_B = (int*)malloc(tamano * sizeof(int));
        displs_B = (int*)malloc(tamano * sizeof(int));

        // Proceso 0 no enviará datos a sí mismo, solo a los trabajadores
        for (int i = 0; i < tamano; i++) {
            if (i == 0) {
                sendcounts_A[i] = 0;
                sendcounts_B[i] = 0;
            }
            else {
                sendcounts_A[i] = N; // Una fila por trabajador
                sendcounts_B[i] = N;
            }
        }

        // Definir desplazamientos
        displs_A[0] = 0;
        displs_B[0] = 0;
        for (int i = 1; i < tamano; i++) {
            displs_A[i] = displs_A[i - 1] + sendcounts_A[i - 1];
            displs_B[i] = displs_B[i - 1] + sendcounts_B[i - 1];
        }
    }

    // Cada trabajador necesita una fila de A y una fila de B
    int* filaA = (int*)malloc(N * sizeof(int));
    int* filaB = (int*)malloc(N * sizeof(int));
    int* filaC = (int*)malloc(N * sizeof(int));

    // Distribuye las filas de A
    MPI_Scatterv(A, sendcounts_A, displs_A, MPI_INT, filaA, N, MPI_INT, 0, MPI_COMM_WORLD );

    // Distribuye las filas de B
    MPI_Scatterv(B, sendcounts_B, displs_B, MPI_INT, filaB, N, MPI_INT, 0, MPI_COMM_WORLD);

    // Cada trabajador suma su fila
    if (rango != 0) {
        printf("[Proceso %d en %s] Recibió fila de A: ", rango, nombre);
        for (int j = 0; j < N; j++) {
            printf("%d ", filaA[j]);
        }
        printf("\n");

        printf("[Proceso %d en %s] Recibió fila de B: ", rango, nombre);
        for (int j = 0; j < N; j++) {
            printf("%d ", filaB[j]);
        }
        printf("\n");

        // Calcula la suma de las filas
        for (int j = 0; j < N; j++) {
            filaC[j] = filaA[j] + filaB[j];
        }

        // Imprime la fila resultante de la suma
        printf("[Proceso %d en %s] Calculó fila de C: ", rango, nombre);
        for (int j = 0; j < N; j++) {
            printf("%d ", filaC[j]);
        }
        printf("\n");
    }

    // Recopila las filas sumadas en la matriz C
    MPI_Gatherv((rango != 0) ? filaC : NULL, (rango != 0) ? N : 0, MPI_INT, C, sendcounts_A, displs_A,MPI_INT, 0, MPI_COMM_WORLD );

    if (rango == 0) {
        // Finaliza el temporizador
        fin = MPI_Wtime();

        // Imprime la matriz resultante C
        printf("Matriz C (A + B):\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                printf("%d ", C[i * N + j]);
            }
            printf("\n");
        }
        printf("\n");

        // Imprime el tiempo de ejecución
        printf("Tiempo de ejecución: %f segundos\n", fin - inicio);

        // Libera la memoria asignada a A, B, C, sendcounts y displs
        free(A);
        free(B);
        free(C);
        free(sendcounts_A);
        free(displs_A);
        free(sendcounts_B);
        free(displs_B);
    }

    // Libera la memoria asignada a las filas
    free(filaA);
    free(filaB);
    free(filaC);

    // Finaliza MPI
    MPI_Finalize();
    return 0;
}
