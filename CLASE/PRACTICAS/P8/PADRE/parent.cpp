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

    if (mirango == 0) { // Solo el proceso con rango 0 actúa como padre
        printf("PADRE> Proceso %d de %d: Iniciando proceso padre.\n", mirango, tamano);
        fflush(stdout);


        // Argumentos para los hijos (pueden ser MPI_ARGV_NULL si no se necesitan)
        char** argumentos_hijos = MPI_ARGV_NULL;

        // Información sobre dónde y cómo lanzar los hijos (MPI_INFO_NULL por defecto)
        MPI_Info info;
        MPI_Info_create(&info);

        MPI_Comm intercom;
        int errcodes[num_hijos];

        // Definir el comando como un arreglo de char no constante
        char child_command[] = "C:\\Users\\urban\\DIEGO\\UBU 4º\\1ºSemestre\\Arquitecturas Paralelas\\PRACTICAS\\P8\\x64\\Debug\\HIJO.exe"; // Ruta absoluta

        // Lanzar procesos hijos
        int resultado = MPI_Comm_spawn(child_command, argumentos_hijos, num_hijos, info, 0, MPI_COMM_WORLD, &intercom, errcodes);

        if (resultado != MPI_SUCCESS) {
            fprintf(stderr, "Error al lanzar los procesos hijos.\n");
            MPI_Abort(MPI_COMM_WORLD, resultado);
        }

        printf("PADRE> Proceso %d: Se han lanzado %d procesos hijos.\n", mirango, num_hijos);
        fflush(stdout);

        // Enviar mensajes de saludo a los hijos
        char saludo[] = "Hola desde el proceso padre!";
        for (int i = 0; i < num_hijos; i++) {
            MPI_Send(saludo, strlen(saludo) + 1, MPI_CHAR, i, 0, intercom);
            printf("PADRE> Proceso %d: Enviado saludo al hijo %d.\n", mirango, i);
            fflush(stdout);
        }

        // Recibir mensajes de confirmación de los hijos
        for (int i = 0; i < num_hijos; i++) {
            char mensaje_recibido[100];
            MPI_Recv(mensaje_recibido, 100, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, intercom, MPI_STATUS_IGNORE);
            printf("PADRE> Proceso %d: Recibido mensaje de hijo: %s\n", mirango, mensaje_recibido);
            fflush(stdout);
        }

        MPI_Info_free(&info);
    }
    else {
        // Otros procesos (si los hay) pueden realizar otras tareas
        printf("PADRE> Proceso %d de %d: No es el proceso padre.\n", mirango, tamano);
        fflush(stdout);
    }

    MPI_Finalize();
    return 0;
}
