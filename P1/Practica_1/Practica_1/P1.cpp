#include <mpi.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    int mirango, tamano;
    int longitud;
    char nombre[32];
    int num;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mirango);
    MPI_Comm_size(MPI_COMM_WORLD, &tamano);
    MPI_Get_processor_name(nombre, &longitud);

    printf("[Maquina %s]> Proceso %d de %d: Hola Mundo!\n", nombre, mirango, tamano);
    fflush(stdout);

    if (mirango == 0) {
        printf("Proceso %d: Introduzca un número entero: ", mirango);
        fflush(stdout);
        scanf_s("%d", &num);
    }

    // Difundir el número a todos los procesos
    MPI_Bcast(&num, 1, MPI_INT, 0, MPI_COMM_WORLD);

    printf("Proceso %d: He recibido el número %d.\n", mirango, num);
    fflush(stdout);

    MPI_Finalize();
    return 0;
}
