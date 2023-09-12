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
        return (arr[(arr_sz / 2) - 1] + arr[arr_sz / 2]);
    } else {
        return arr[(arr_sz / 2) - 1];
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
    int numbers[MAX_NUMBERS];
    int number_amount;

    if (world_rank == 0) {

        for(int p = 1; p < world_size; p++) {
            number_amount = generate_random(1000, 2000);
            printf("n_amount: %d; ", number_amount);
            for(int i = 0; i < number_amount; i++) {
                numbers[i] = generate_random(0, 100);
                // printf("arr[%d]: %d\n", i, numbers[i]);
            }
            
            int tag = generate_random(0, 3);
            // int tag = 0;
            MPI_Send(numbers, number_amount, MPI_INT, p, tag, MPI_COMM_WORLD);
            printf("MASTER sent %d numbers to %d. TAG: %d\n", number_amount, p, tag);
        }

        for(int p = 1; p < world_size; p++){
            MPI_Probe(p, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int resultado;
            MPI_Probe(p, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int tag = status.MPI_TAG;
            MPI_Recv(&resultado, 1, MPI_INT, p, tag, MPI_COMM_WORLD, &status);
            printf("A tarefa foi %d e aqui está o resultado: %d\n", tag, resultado);
        }

    } else {
            int size;
            int *num_arr;
            MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int tag = status.MPI_TAG;
            switch (tag) {
                case 0:
                    // DRY
                    MPI_Get_count(&status, MPI_INT, &size);
                    num_arr = malloc(size * sizeof(int));

                    MPI_Recv(num_arr, MAX_NUMBERS, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

                    int sum = sum_array(num_arr, size);
                    MPI_Send(&sum, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                    break;
                case 1:
                    // num_arr = malloc(size * sizeof(int));

                    MPI_Recv(num_arr, MAX_NUMBERS, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
                    MPI_Get_count(&status, MPI_INT, &size);
                    
                    int avg = sum_array(num_arr, size) / size;
                    MPI_Send(&avg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                    break;
                case 2:
                    // num_arr = malloc(size * sizeof(int));

                    MPI_Recv(num_arr, MAX_NUMBERS, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
                    MPI_Get_count(&status, MPI_INT, &size);

                    int max = max_val(num_arr, size);
                    MPI_Send(&max, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);       
                    break;
                case 3:
                    // num_arr = malloc(size * sizeof(int));

                    MPI_Recv(num_arr, MAX_NUMBERS, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
                    MPI_Get_count(&status, MPI_INT, &size);

                    int med = median_val(num_arr, size);
                    MPI_Send(&med, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);   
                    break;
                default:
                    break;
            }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

    // for (int k = 0; k < 10; k++) {
    //     int sz = generate_random(1000, 2000);
    //     int *arr = malloc(sz * sizeof(int));

    //     for (int i = 0; i < sz; i++) {
    //         arr[i] = generate_random(0, 100);
    //         printf("%d ", arr[i]);
    //     }
    //     printf("\n\n");
    // }

    // srand(time(NULL));
    // int rng = generate_random(1000, 2000);
    // printf("random number: %d\n", rng);


    return 0;
}