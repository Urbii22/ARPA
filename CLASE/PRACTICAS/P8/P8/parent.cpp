// parent.cpp
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define num_hijos 3
int main(int argc, char* argv[])
{
    int mirango, tamano;
    int longitud;
    char nombre[32];
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mirango);
    MPI_Comm_size(MPI_COMM_WORLD, &tamano);
    MPI_Get_processor_name(nombre, &longitud);

    if (mirango == 0) { // Solo el proceso con rango 0 act�a como padre
        printf("[M�quina %s]> Proceso %d de %d: Iniciando proceso padre.\n", nombre, mirango, tamano);
        fflush(stdout);

        // N�mero de procesos hijos a lanzar

        // Argumentos para los hijos (pueden ser MPI_ARGV_NULL si no se necesitan)
        char** argumentos_hijos = MPI_ARGV_NULL;

        // Informaci�n sobre d�nde y c�mo lanzar los hijos (MPI_INFO_NULL por defecto)
        MPI_Info info;
        MPI_Info_create(&info);

        MPI_Comm intercom;
        int errcodes[num_hijos];

        // Definir el comando como un arreglo de char no constante
        char child_command[] = "./child";

        // Lanzar procesos hijos
        int resultado = MPI_Comm_spawn(child_command, argumentos_hijos, num_hijos, info, 0, MPI_COMM_WORLD, &intercom, errcodes);

        if (resultado != MPI_SUCCESS) {
            fprintf(stderr, "Error al lanzar los procesos hijos.\n");
            MPI_Abort(MPI_COMM_WORLD, resultado);
        }

        printf("[M�quina %s]> Proceso %d: Se han lanzado %d procesos hijos.\n", nombre, mirango, num_hijos);
        fflush(stdout);

        // Enviar mensajes de saludo a los hijos
        char saludo[] = "Hola desde el proceso padre!";
        for (int i = 0; i < num_hijos; i++) {
            MPI_Send(saludo, strlen(saludo) + 1, MPI_CHAR, i, 0, intercom);
            printf("[M�quina %s]> Proceso %d: Enviado saludo al hijo %d.\n", nombre, mirango, i);
            fflush(stdout);
        }

        // Recibir mensajes de los hijos
        for (int i = 0; i < num_hijos; i++) {
            char mensaje_recibido[100];
            MPI_Recv(mensaje_recibido, 100, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, intercom, MPI_STATUS_IGNORE);
            printf("[M�quina %s]> Proceso %d: Recibido mensaje de hijo %d: %s\n", nombre, mirango, i, mensaje_recibido);
            fflush(stdout);
        }

        MPI_Info_free(&info);
    }
    else {
        // Otros procesos (si los hay) pueden realizar otras tareas
        printf("[M�quina %s]> Proceso %d de %d: No es el proceso padre.\n", nombre, mirango, tamano);
        fflush(stdout);
    }

    MPI_Finalize();
    return 0;
}
