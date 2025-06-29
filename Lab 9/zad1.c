#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

typedef struct {
    double a;
    double b;
    double dx;
    double *result;
    int *ready;
} ThreadData;

double f(double x) {
    return 4.0 / (x*x + 1);
}

double calculate_integral(double a, double b, double dx) {
    double sum = 0.0;
    double x;
    
    for (x = a; x < b; x += dx) {
        sum += f(x) * dx;
    }
    
    return sum;
}

void* thread_function(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    *(data->result) = calculate_integral(data->a, data->b, data->dx);
    *(data->ready) = 1;
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Użycie: %s szerokość_prostokąta liczba_wątków\n", argv[0]);
        return 1;
    }

    double dx = atof(argv[1]);
    int num_threads = atoi(argv[2]);
    
    if (dx <= 0 || num_threads <= 0) {
        fprintf(stderr, "Parametry muszą być dodatnie\n");
        return 1;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    double results[num_threads];
    int ready[num_threads];
    for(int i = 0; i < num_threads; i++) {
        ready[i] = 0;
    }

    for(int i = 0; i < num_threads; i++) {
        double a = i * (1.0 / num_threads);
        double b = (i + 1) * (1.0 / num_threads);
        thread_data[i].a = a;
        thread_data[i].b = b;
        thread_data[i].dx = dx;
        thread_data[i].result = &results[i];
        thread_data[i].ready = &ready[i];

        if (pthread_create(&threads[i], NULL, thread_function, &thread_data[i]) != 0) {
            perror("Nie można utworzyć wątku");
            return 1;
        }
    }

    while(1) {
        int all_ready = 1;
        for(int i = 0; i < num_threads; i++) {
            if (!ready[i]) {
                all_ready = 0;
                break;
            }
        }
        if (all_ready) break;
        
        struct timespec delay = {0, 100000000L};
        nanosleep(&delay, NULL);
    }

    double total = 0.0;
    for(int i = 0; i<num_threads; i++) {
        total += results[i];
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) + 
                        (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("Liczba wątków=%d: wynik=%.10f, czas=%.6f s\n", num_threads, total, time_taken);
    
    return 0;
}