#include <mpi.h>
#include <stdio.h>

// Funci�n para calcular el factorial de un n�mero
long double factorial(int n) {
    long double result = 1;
    for (int i = 1; i <= n; ++i) {
        result *= i;
    }
    return result;
}

int main(int argc, char* argv[]) {
    int rank, size;
    int length;
    char name[32];

    // Inicializar el entorno MPI
    MPI_Init(&argc, &argv);
    // Obtener el rango del proceso
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // Obtener el n�mero total de procesos
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    // Obtener el nombre del procesador
    MPI_Get_processor_name(name, &length);

    // Asegurarse de que haya al menos 2 procesos
    if (size < 2) {
        if (rank == 0) {
            printf("Se necesitan al menos 2 procesos para esta tarea.\n");
        }
        MPI_Finalize();
        return 0;
    }

    if (rank == 0) { // Proceso 0: Maneja la entrada del usuario y env�a los datos al proceso 1
        long double number;
        MPI_Request request;
        MPI_Status status;
        int calculation_done = 1; // Indicador para saber si el c�lculo anterior ha terminado

        while (1) {
            if (calculation_done) {
                // Solicitar un n�mero al usuario
                printf("Introduce un numero para calcular su factorial (0 para salir): ");
                fflush(stdout);
                scanf_s("%Lf", &number);

                // Iniciar el env�o no bloqueante al proceso 1
                MPI_Isend(&number, 1, MPI_LONG_DOUBLE, 1, 0, MPI_COMM_WORLD, &request);
                calculation_done = 0; // Establecer el indicador para indicar que el c�lculo est� en progreso

                // Si el usuario ingresa 0, terminar el programa
                if (number == 0) {
                    break;
                }
            }
            else {
                int flag = 0;
				printf("Esperando a que el proceso 55 complete el calculo...\n");
                // Probar si el env�o no bloqueante ha terminado
                MPI_Test(&request, &flag, &status);
                if (flag == 1) {
                    // Si el env�o ha terminado, establecer calculation_done a 1
                    calculation_done = 1;
                }
                else {
                    // Si el env�o no ha terminado, informar al usuario
                    printf("Esperando a que el proceso 1 complete el calculo...\n");
                    fflush(stdout);
                }
            }
        }
    }
    else if (rank == 1) { // Proceso 1: Recibe los datos y calcula el factorial
        long double number;
        MPI_Request request;
        MPI_Status status;

        while (1) {
            // Iniciar la recepci�n no bloqueante desde el proceso 0
            MPI_Irecv(&number, 1, MPI_LONG_DOUBLE, 0, 0, MPI_COMM_WORLD, &request);
            // Esperar hasta que la operaci�n de recepci�n se complete
            MPI_Wait(&request, &status);

            // Si el n�mero recibido es 0, terminar el programa
            if (number == 0) {
                break;
            }

            // Calcular el factorial del n�mero recibido
            long double result = factorial((int)number);
            // Imprimir el resultado
            printf("[Maquina %s]> Proceso %d: El factorial de %.0Lf es %.0Lf\n", name, rank, number, result);
            fflush(stdout);
        }
    }

    // Finalizar el entorno MPI
    MPI_Finalize();
    return 0;
}
