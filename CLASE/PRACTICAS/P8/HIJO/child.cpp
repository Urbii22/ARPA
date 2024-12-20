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

    MPI_Comm parent;
    MPI_Comm_get_parent(&parent);

    MPI_Comm_rank(MPI_COMM_WORLD, &mirango);
    MPI_Comm_size(MPI_COMM_WORLD, &tamano);
    MPI_Get_processor_name(nombre, &longitud);

    if (parent == MPI_COMM_NULL) {
        printf("HIJO> Proceso %d: No tengo padre. Finalizando.\n", mirango);
        fflush(stdout);
        MPI_Finalize();
        return 0;
    }

    // Enviar mensaje de saludo al ser creado
    printf("HIJO> Proceso %d: Hijo creado. ¡Hola!\n", mirango);
    fflush(stdout);

    // Recibir mensaje del padre
    char mensaje_padre[100];
    MPI_Recv(mensaje_padre, 100, MPI_CHAR, 0, MPI_ANY_TAG, parent, MPI_STATUS_IGNORE);
    printf("HIJO> Proceso %d: Recibido del padre: %s\n", mirango, mensaje_padre);
    fflush(stdout);

    // Enviar mensaje de confirmación al padre
    char respuesta_padre[] = "Mensaje recibido por el hijo.";
    MPI_Send(respuesta_padre, strlen(respuesta_padre) + 1, MPI_CHAR, 0, 0, parent);
    printf("HIJO> Proceso %d: Enviado confirmación al padre.\n", mirango);
    fflush(stdout);

    // Comunicación entre hijos usando MPI_COMM_WORLD
    if (mirango == 0) {
        // El hijo de menor rango envía mensajes a los demás hijos
        char mensaje_hijos[] = "Mensaje del hijo de menor rango.";
        for (int i = 1; i < tamano; i++) {
            MPI_Send(mensaje_hijos, strlen(mensaje_hijos) + 1, MPI_CHAR, i, 1, MPI_COMM_WORLD);
            printf("HIJO> Proceso %d: Enviado mensaje a hijo %d.\n", mirango, i);
            fflush(stdout);
        }
    } else {
        // Los hijos que no son el de menor rango reciben mensajes del hijo 0
        char mensaje_hijo[100];
        MPI_Recv(mensaje_hijo, 100, MPI_CHAR, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("HIJO> Proceso %d: Recibido del hijo de menor rango: %s\n", mirango, mensaje_hijo);
        fflush(stdout);
    }

    MPI_Finalize();
    return 0;
}
