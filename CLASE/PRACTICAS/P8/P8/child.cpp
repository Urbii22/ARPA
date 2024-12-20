// child.cpp
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
    int mirango, tamano;
    int longitud;
    char nombre[32];
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mirango);
    MPI_Comm_size(MPI_COMM_WORLD, &tamano);
    MPI_Get_processor_name(nombre, &longitud);

    // Obtener el comunicador con el padre
    MPI_Comm parent;
    MPI_Comm_get_parent(&parent);

    if (parent == MPI_COMM_NULL) {
        printf("[Máquina %s]> Proceso %d: No tengo padre. Finalizando.\n", nombre, mirango);
        fflush(stdout);
        MPI_Finalize();
        return 0;
    }

    // Enviar mensaje de saludo al ser creado
    printf("[Máquina %s]> Proceso %d: Hijo creado. ¡Hola!\n", nombre, mirango);
    fflush(stdout);

    // Recibir mensaje del padre
    char mensaje_padre[100];
    MPI_Recv(mensaje_padre, 100, MPI_CHAR, 0, MPI_ANY_TAG, parent, MPI_STATUS_IGNORE);
    printf("[Máquina %s]> Proceso %d: Recibido del padre: %s\n", nombre, mirango, mensaje_padre);
    fflush(stdout);

    // Si es el hijo de menor rango, enviar mensaje a los demás hijos
    if (mirango == 0) {
        char mensaje_hijos[] = "Mensaje del hijo de menor rango.";
        MPI_Comm intracom;
        MPI_Intercomm_merge(parent, 0, &intracom);

        // Enviar a todos los demás hijos
        for (int i = 1; i < tamano; i++) {
            MPI_Send(mensaje_hijos, strlen(mensaje_hijos) + 1, MPI_CHAR, i, 1, intracom);
            printf("[Máquina %s]> Proceso %d: Enviado mensaje a hijo %d.\n", nombre, mirango, i);
            fflush(stdout);
        }
    }

    // Recibir mensaje del hijo de menor rango
    if (mirango != 0) {
        char mensaje_hijo[100];
        MPI_Comm intracom;
        MPI_Intercomm_merge(parent, 0, &intracom);
        MPI_Recv(mensaje_hijo, 100, MPI_CHAR, 0, 1, intracom, MPI_STATUS_IGNORE);
        printf("[Máquina %s]> Proceso %d: Recibido del hijo de menor rango: %s\n", nombre, mirango, mensaje_hijo);
        fflush(stdout);
    }

    MPI_Finalize();
    return 0;
}
