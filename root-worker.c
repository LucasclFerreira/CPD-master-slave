#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int generate_random(int min, int max) {
    // return min + (int)(((double)rand() / RAND_MAX) * (max - min));
    return min + rand() % (max - min);
}

int sum_array(int *arr, int arr_sz) {
    sleep(2);
    int sum = 0;
    for (int i = 0; i < arr_sz; i++) {
        sum += arr[i];
    }
    return sum;
}

int max_val(int *arr, int arr_sz) {
    sleep(1);
    int max = 0;
    for (int i = 0; i < arr_sz; i++) {
        if (arr[i] > max) {
            max = arr[i];
        }
    }
    return max;
}

int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

int median_val(int *arr, int arr_sz) {
    sleep(3);
    qsort(arr, arr_sz, sizeof(int), cmpfunc);
    
    if (arr_sz % 2 == 0) {
        return (arr[(arr_sz / 2) - 1] + arr[arr_sz / 2]) / 2;
    } else {
        return arr[(arr_sz / 2)];
    }
}

int *random_array(int *arr, int arr_sz) {
    for (int j = 0; j < arr_sz; j++) {
        arr[j] = generate_random(0, 100);
    }
    return arr;
}

int main(int argc, char** argv) {
    int seed = time(NULL);
    srand(seed);

    MPI_Init(NULL, NULL);
    MPI_Status status;

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    const int MAX_NUMBERS = 2000;

    // buffer para o vetor a ser enviado
    int *numbers;
    int number_amount;

    // buffer para o vetor a ser recebido
    int *num_arr;

    if (world_rank == 0) {
        int n_tasks = generate_random(10, 100);

        // enviando uma tarefa para cada trabalhador
        for (int i = 1; i < world_size; i++) {
            printf("{  Task number %d  }\n", n_tasks);

            // gerando e populando o vetor aleatório de inteiros
            number_amount = generate_random(1000, MAX_NUMBERS);
            numbers = (int*)malloc(number_amount * sizeof(int));
            numbers = random_array(numbers, number_amount);

            // gerando a tag que seleciona a tarefa a ser realizada
            int send_tag = generate_random(0, 4);

            MPI_Send(numbers, number_amount, MPI_INT, i, send_tag, MPI_COMM_WORLD);
            n_tasks--;
        }

        // o processo que terminar primeiro recebe uma nova tarefa até não ter mais tarefas
        while (n_tasks > 0) {
            printf("{  Task number %d  }\n", n_tasks);
            int result;
            MPI_Recv(&result, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            printf("\t[Root] received from %d the value %d with tag = %d\n", status.MPI_SOURCE, result, status.MPI_TAG);

            number_amount = generate_random(1000, MAX_NUMBERS);
            numbers = (int*)malloc(number_amount * sizeof(int));
            numbers = random_array(numbers, number_amount);

            int send_tag = generate_random(0, 4);

            MPI_Send(numbers, number_amount, MPI_INT, status.MPI_SOURCE, send_tag, MPI_COMM_WORLD);
            n_tasks--;
        }

        // recebendo últimos resultados e enviando tag 10 para o trabalhadores finalizarem a operação
        for (int j = 1; j < world_size; j++) {
            int result;
            MPI_Recv(&result, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            printf("\t[Root] received from %d the value %d with tag = %d\n", status.MPI_SOURCE, result, status.MPI_TAG);

            // buffer abaixo não representa nada, pois o importante do send abaixo é a tag 10!
            int finish_computing = 42;
            MPI_Send(&finish_computing, 1, MPI_INT, j, 10, MPI_COMM_WORLD);
        }        
    } else {
        while (1) {
            MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int recv_tag = status.MPI_TAG;

            if (recv_tag == 10) {
                printf("Tag 10 received - finalizing process...\n");
                break;
            }

            int size;
            MPI_Get_count(&status, MPI_INT, &size);

            num_arr = (int*)malloc(size * sizeof(int));
            printf("\t\t[Worker] received %d numbers and tag = %d\n", size, recv_tag);

            int value;
            MPI_Recv(num_arr, size, MPI_INT, 0, recv_tag, MPI_COMM_WORLD, &status);
            switch (recv_tag) {
                case 0: {
                    value = sum_array(num_arr, size);
                    // printf("\tsum: %d\n", value);
                    break;
                }
                case 1: {
                    value = sum_array(num_arr, size) / size;
                    // printf("\tavg: %d\n", value);
                    break;
                }     
                case 2: {
                    value = max_val(num_arr, size);  
                    // printf("\tmax: %d\n", value);
                    break;
                }
                case 3: {
                    value = median_val(num_arr, size);
                    // printf("\tmed: %d\n", value);
                    break;
                }
                default: {
                    printf("Tag of %d does nothing.\n", status.MPI_TAG);
                    break;
                }    
            }
            MPI_Send(&value, 1, MPI_INT, 0, recv_tag, MPI_COMM_WORLD);
        }
    }
    free(num_arr);
    free(numbers);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

    return 0;
}