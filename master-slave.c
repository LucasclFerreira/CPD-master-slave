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

    MPI_Init(NULL, NULL);
    MPI_Status status;

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    const int MAX_NUMBERS = 2000;

    // sending arr
    int *numbers;
    int number_amount;

    // receiving arr
    int *num_arr;

    if (world_rank == 0) {
        int n_tasks = generate_random(4, 40);
        printf("N_TASK = %d\n", n_tasks);

        for (int i = 1; i < world_size; i++) {
            for (int t = 0; t < n_tasks; t++) {

                number_amount = generate_random(1000, MAX_NUMBERS);
                numbers = (int*)malloc(number_amount * sizeof(int));

                for(int j = 0; j < number_amount; j++) {
                    numbers[j] = generate_random(0, 100);
                }
                
                int send_tag = generate_random(0, 4);  // escolhendo task aleatória
            
                MPI_Send(numbers, number_amount, MPI_INT, i, send_tag, MPI_COMM_WORLD);
            }

            int finalize = 10;
            MPI_Send(&finalize, 1, MPI_INT, i, 10, MPI_COMM_WORLD);

            for (int r = 0; r < n_tasks; r++) {
                MPI_Probe(i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                int tag_received = status.MPI_TAG;
                // int src = status.MPI_SOURCE;

                int val;
                MPI_Recv(&val, 1, MPI_INT, i, tag_received, MPI_COMM_WORLD, &status);
                printf("\t[Root] received from %d the value %d with tag = %d\n", i, val, status.MPI_TAG);
            }

            // MPI_Recv(&resultado, 1, MPI_INT, processor, send_tag, MPI_COMM_WORLD, &status);
            // printf("Processador %d obteve resultado = %d\n", status.MPI_SOURCE, resultado);

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