#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]) {
    int rank, size;
    int nombre_longitud;
    char nombre_host[32];

    // Inicializar el entorno MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Obtener el rango del proceso
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Obtener el número total de procesos
    MPI_Get_processor_name(nombre_host, &nombre_longitud); // Obtener el nombre del host

    // Verificar que haya al menos 2 procesos (1 maestro y al menos 1 esclavo)
    if (size < 2) {
        if (rank == 0) {
            printf("Error: Se requieren al menos 2 procesos (1 maestro y 1 esclavo).\n");
        }
        MPI_Finalize();
        return 1;
    }

    // Determinar el tamaño de los vectores como n-1
    int tamaño_vectores = size - 1;

    // Variables para el producto escalar
    double* x = NULL; // Vector x (solo en el maestro)
    double* y = NULL; // Vector y (solo en el maestro)
    double recv_x = 0.0; // Elemento de x recibido (esclavos)
    double recv_y = 0.0; // Elemento de y recibido (esclavos)
    double producto_parcial = 0.0; // Producto parcial calculado por el esclavo
    double producto_total = 0.0; // Producto escalar total (maestro)
    double inicio = 0.0, fin = 0.0; // Variables para medir el tiempo

    // Proceso 0 (Maestro) inicializa los vectores y configura Scatterv
    if (rank == 0) {
        // Asignar memoria para los vectores x e y
        x = (double*)malloc(sizeof(double) * tamaño_vectores);
        y = (double*)malloc(sizeof(double) * tamaño_vectores);

        // Inicializar los vectores con valores aleatorios
        srand(time(NULL));
        printf("Proceso %d: Inicializando vectores de tamaño %d...\n", rank, tamaño_vectores);
        for (int i = 0; i < tamaño_vectores; i++) {
            x[i] = rand() % 100; // Valores entre 0 y 99
            y[i] = rand() % 100;
        }

        // Imprimir los vectores completos para verificación
        printf("Proceso %d: Vector x: [", rank);
        for (int i = 0; i < tamaño_vectores; i++) {
            printf("%.2lf", x[i]);
            if (i < tamaño_vectores - 1) printf(", ");
        }
        printf("]\n");

        printf("Proceso %d: Vector y: [", rank);
        for (int i = 0; i < tamaño_vectores; i++) {
            printf("%.2lf", y[i]);
            if (i < tamaño_vectores - 1) printf(", ");
        }
        printf("]\n");

        // Iniciar el temporizador justo antes de distribuir los datos
        inicio = MPI_Wtime();
    }

    // Preparar los arreglos de sendcounts y displs para Scatterv
    int* sendcounts_x = (int*)malloc(sizeof(int) * size);
    int* displs_x = (int*)malloc(sizeof(int) * size);
    int* sendcounts_y = (int*)malloc(sizeof(int) * size);
    int* displs_y = (int*)malloc(sizeof(int) * size);

    for (int i = 0; i < size; i++) {
        if (i == 0) {
            sendcounts_x[i] = 0; // Proceso 0 no recibe datos de x
            sendcounts_y[i] = 0; // Proceso 0 no recibe datos de y
            displs_x[i] = 0;
            displs_y[i] = 0;
        }
        else {
            sendcounts_x[i] = 1; // Cada esclavo recibe un elemento de x
            sendcounts_y[i] = 1; // Cada esclavo recibe un elemento de y
            displs_x[i] = i - 1; // Desplazamiento para x
            displs_y[i] = i - 1; // Desplazamiento para y
        }
    }

    // Distribuir los elementos de x a los esclavos
    MPI_Scatterv(x, sendcounts_x, displs_x, MPI_DOUBLE, &recv_x, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    // Distribuir los elementos de y a los esclavos
    MPI_Scatterv(y, sendcounts_y, displs_y, MPI_DOUBLE, &recv_y, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Cada proceso esclavo (rank != 0) realiza el cálculo parcial
    if (rank != 0) {
        printf("Proceso %d en host %s: Recibió x = %.2lf y y = %.2lf\n", rank, nombre_host, recv_x, recv_y);
        producto_parcial = recv_x * recv_y;
        printf("Proceso %d en host %s: Calculó producto parcial = %.2lf\n", rank, nombre_host, producto_parcial);
    }

    // Reducir todos los productos parciales sumándolos en el proceso 0
    MPI_Reduce(&producto_parcial, &producto_total, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    // Proceso 0 finaliza el temporizador y muestra el resultado
    if (rank == 0) {
        fin = MPI_Wtime();
        printf("Proceso %d: Producto escalar total = %.2lf\n", rank, producto_total);
        printf("Proceso %d: Tiempo empleado = %.6lf segundos\n", rank, fin - inicio);

        // Liberar memoria
        free(x);
        free(y);
    }

    // Liberar memoria de arreglos de Scatterv
    free(sendcounts_x);
    free(displs_x);
    free(sendcounts_y);
    free(displs_y);

    // Finalizar el entorno MPI
    MPI_Finalize();
    return 0;
}
