#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int generate_random(int min, int max) {
    // return min + (int)(((double)rand() / RAND_MAX) * (max - min));
    return min + rand() % (max - min);
}

int sum_array(int *arr, int arr_sz) {
    int sum = 0;
    for (int i = 0; i < arr_sz; i++) {
        sum += arr[i];
    }
    return sum;
}

int max_val(int *arr, int arr_sz) {
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
    qsort(arr, arr_sz, sizeof(int), cmpfunc);
    
    if (arr_sz % 2 == 0) {
        return (arr[(arr_sz / 2) - 1] + arr[arr_sz / 2]) / 2;
    } else {
        return arr[(arr_sz / 2)];
    }
}

int main(int argc, char** argv) {
    int seed = time(NULL);
    srand(seed);

    MPI_Status status;
    MPI_Init(NULL, NULL);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    const int MAX_NUMBERS = 2000;
    // sending
    int *numbers;
    int number_amount;

    // receiving
    int *num_arr;

    if (world_rank == 0) {
        for(int p = 1; p < world_size; p++) {
            number_amount = generate_random(1000, MAX_NUMBERS);
            numbers = malloc(number_amount * sizeof(int));
            for(int i = 0; i < number_amount; i++) {
                numbers[i] = generate_random(0, 100);
                // printf("arr[%d]: %d\n", i, numbers[i]);
            }
            
            int tag = generate_random(0, 4);
            // int tag = 0;
            MPI_Send(numbers, number_amount, MPI_INT, p, tag, MPI_COMM_WORLD);
            printf("MASTER sent %d numbers to %d. TAG: %d\n", number_amount, p, tag);
            
        }

        for(int p = 1; p < world_size; p++){
            // MPI_Probe(p, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Probe(p, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int tag = status.MPI_TAG;

            int resultado;
            MPI_Recv(&resultado, 1, MPI_INT, p, tag, MPI_COMM_WORLD, &status);

            printf("\nA tarefa foi %d e aqui está o resultado: %d\n", tag, resultado);
        }
    } else {
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int tag = status.MPI_TAG;

        int size;
        MPI_Get_count(&status, MPI_INT, &size);

        num_arr = malloc(size * sizeof(int));

        printf("PROBE recebeu: size=%d; tag=%d\n", size, tag);

        switch (tag) {
            case 0: {
                MPI_Recv(num_arr, size, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

                int sum = sum_array(num_arr, size);
                MPI_Send(&sum, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                break;
            }
            case 1: {
                MPI_Recv(num_arr, size, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
                
                int avg = sum_array(num_arr, size) / size;
                MPI_Send(&avg, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
                break;
            }     
            case 2: {
                MPI_Recv(num_arr, size, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);

                int max = max_val(num_arr, size);
                MPI_Send(&max, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);       
                break;
            }
            case 3: {
                MPI_Recv(num_arr, MAX_NUMBERS, MPI_INT, 0, 3, MPI_COMM_WORLD, &status);

                int med = median_val(num_arr, size);
                MPI_Send(&med, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);   
                break;
            }
            default: {
                printf("Tag of %d does nothing.\n", tag);
                break;
            }
        }
    }

    free(num_arr);
    free(numbers);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

    // for (int k = 0; k < 10; k++) {
    //     int sz = generate_random(5, 10);
    //     int *arr = malloc(sz * sizeof(int));

    //     for (int i = 0; i < sz; i++) {
    //         arr[i] = generate_random(0, 100);
    //         printf("%d ", arr[i]);
    //     }
    //     int tag = generate_random(0, 4);
    //     int result;
    //     switch (tag) {
    //         case 0:
    //             result = sum_array(arr, sz);
    //             break;
    //         case 1:
    //             result = max_val(arr, sz);
    //             break;
    //         case 2:
    //             result = median_val(arr, sz);
    //             break;
    //         case 3:
    //             result = sum_array(arr, sz) / sz;
    //             break;
    //         default:
    //             break;
    //     }
    //     printf("[%d] results in: %d\n\n", tag, result);
    // }

    // srand(time(NULL));
    // int rng = generate_random(1000, 2000);
    // printf("random number: %d\n", rng);


    return 0;
}