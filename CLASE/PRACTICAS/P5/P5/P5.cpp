
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // Para usar strrchr
#include <windows.h>    // Para GetModuleFileName

// Función para obtener el directorio de un path
char* my_dirname(char* path) {
    char* last_slash = strrchr(path, '\\');
    if (last_slash) {
        *last_slash = '\0'; // Termina la cadena en el último '\\'
    }
    return path;
}

// Función main
int main(int argc, char* argv[])
{
    // Variables para indentificar los procesos
    int mirango, size;
    int longitud;
    char nombre[32];

    // N?mero de veces que cada proceso escribe su mensaje
    int N = 5;

    // Inicio del entorno MPI y obtenci?n de informaci?n de procesos
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mirango);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Get_processor_name(nombre, &longitud);

    // Nombre del fichero
    char filename[] = "practica_05.txt";

    // Buffer para almacenar la ruta del ejecutable
    char exePath[1024];
    // Obtener la ruta completa del ejecutable
    GetModuleFileName(NULL, exePath, sizeof(exePath));

    // Obtener el directorio donde se encuentra el ejecutable
    char* exeDir = my_dirname(exePath);  // Extraer el directorio

    // Crear una variable que contenga la ruta completa (directorio + nombre de archivo)
    char fullpath[1024];
    // Concatenar el path con el nombre del archivo
    snprintf(fullpath, sizeof(fullpath), "%s\\%s", exeDir, filename);  

    // Variable MPI que guardar? una referencia al archivo utilizado
    MPI_File fh;
    MPI_Status status;

    // Generar el mensaje
    char message[100];
    int n = mirango;
    int m = n + 1;
    snprintf(message, sizeof(message), "Soy el proceso %d de %d y escribo el dato %d\n", n, size, m);
    int msglen = strlen(message);

    // Calcular el tama?o de los datos a escribir
    long long data_size = N * msglen;

    // Calcular el desplazamiento (offset) usando MPI_Exscan
    MPI_Offset offset = 0;
    MPI_Exscan(&data_size, &offset, 1, MPI_LONG_LONG_INT, MPI_SUM, MPI_COMM_WORLD);
    if (mirango == 0)
        offset = 0;

    // Preparar el buffer para escribir
    char* write_buf = (char*)malloc(data_size);
    for (int i = 0; i < N; i++)
    {
        memcpy(write_buf + i * msglen, message, msglen);
    }

    // Abrir el fichero para escritura
    MPI_File_open(MPI_COMM_WORLD, fullpath, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);

    // Escribir los datos en el fichero en la posici?n correspondiente
    MPI_File_write_at(fh, offset, write_buf, data_size, MPI_CHAR, &status);

    // Cerrar el fichero despu?s de escribir
    MPI_File_close(&fh);

    // Sincronizar antes de leer
    MPI_Barrier(MPI_COMM_WORLD);

    // Abrir el fichero para lectura
    MPI_File_open(MPI_COMM_WORLD, fullpath, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);

    // Preparar el buffer para leer
    char* read_buf = (char*)malloc(data_size);

    // Leer los datos desde el fichero en la posici?n correspondiente
    MPI_File_read_at(fh, offset, read_buf, data_size, MPI_CHAR, &status);

    // Cerrar el fichero despu?s de leer
    MPI_File_close(&fh);

    // Mostrar los datos le?dos por cada proceso
    printf("[Maquina %s]> Proceso %d de %d: Informaci?n leida:\n%.*s", nombre, mirango, size, (int)data_size, read_buf);
    fflush(stdout);

    // Liberar memoria
    free(write_buf);
    free(read_buf);

    // Finalizar entorno MPI
    MPI_Finalize();
    return 0;
}


