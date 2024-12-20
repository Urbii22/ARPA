#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[])
{
    int mirango, tamano;
    int longitud;
    char nombre[32];
    int n = 8;  // Tamaño del vector. Cambiar según el número de procesos disponibles
    double* x = NULL, * y = NULL; // Vectores inicializados solo por el proceso 0
    double mi_parcial = 0.0, total = 0.0; // Suma parcial y total del producto escalar
    double start, end; // Variables para medir el tiempo

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mirango);
    MPI_Comm_size(MPI_COMM_WORLD, &tamano);
    MPI_Get_processor_name(nombre, &longitud);

    if (tamano != n)
    {
        if (mirango == 0)
        {
            printf("El número de procesos debe ser igual al tamaño del vector (%d).\n", n);
        }
        MPI_Finalize();
        return 0;
    }

    // Proceso 0 inicializa los vectores
    if (mirango == 0)
    {
        x = (double*)malloc(n * sizeof(double));
        y = (double*)malloc(n * sizeof(double));

        // Inicializar los vectores con valores aleatorios
        srand(time(NULL));
        for (int i = 0; i < n; i++)
        {
            x[i] = rand() % 10 + 1; // Valores entre 1 y 10
            y[i] = rand() % 10 + 1;
        }

        // Mostrar los vectores
        printf("Vector x: ");
        for (int i = 0; i < n; i++)
        {
            printf("%.1f ", x[i]);
        }
        printf("\n");

        printf("Vector y: ");
        for (int i = 0; i < n; i++)
        {
            printf("%.1f ", y[i]);
        }
        printf("\n");

        // Inicio del tiempo de ejecución
        start = MPI_Wtime();
    }

    // Cada proceso recibe un elemento de x y de y
    double xi, yi;
    MPI_Scatter(x, 1, MPI_DOUBLE, &xi, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(y, 1, MPI_DOUBLE, &yi, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Cada proceso calcula su parte del producto escalar
    mi_parcial = xi * yi;

    // Recolectar las sumas parciales en el proceso 0
    MPI_Reduce(&mi_parcial, &total, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    // Proceso 0 muestra el resultado
    if (mirango == 0)
    {
        // Fin del tiempo de ejecución
        end = MPI_Wtime();
        printf("Producto escalar = %.2f\n", total);
        printf("Tiempo de ejecución: %.6f segundos\n", end - start);

        // Liberar memoria
        free(x);
        free(y);
    }

    MPI_Finalize();
    return 0;
}
