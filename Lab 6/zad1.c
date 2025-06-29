#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <math.h>

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

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Użycie: %s szerokość_prostokąta maksymalna_liczba_procesów\n", argv[0]);
        return 1;
    }

    double dx = atof(argv[1]);
    int max_processes = atoi(argv[2]);
    
    if (dx <= 0 || max_processes <= 0) {
        fprintf(stderr, "Parametry muszą być dodatnie\n");
        return 1;
    }

    for (int k = 1; k <= max_processes; k++) {
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        int pipes[k][2];

        for(int i = 0; i<k; i++) {
            if(pipe(pipes[i]) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            } 
        }

        for(int i = 0; i<k; i++) {
            pid_t pid = fork();

            if(pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if(pid == 0) {
                for(int j = 0; j < k; j++) {
                    if(j != i) {
                        close(pipes[j][1]);
                    }
                    close(pipes[j][0]);
                }

                double a = (double)i / k;
                double b = (double)(i+1) / k;

                double calculate_result = calculate_integral(a, b, dx);
                write(pipes[i][1], &calculate_result, sizeof(calculate_result));
                close(pipes[i][1]);
                exit(0);
            }
        }
        
        for(int i = 0; i<k; i++) {
            wait(NULL);
        }

        double total = 0.0;
        
        for(int i = 0; i<k; i++) {
            close(pipes[i][1]);
        }

        for(int i = 0; i<k; i++) {
            double result;
            read(pipes[i][0], &result, sizeof(double));

            total = total + result;
        }
        
        clock_gettime(CLOCK_MONOTONIC, &end);
        double time_taken = (end.tv_sec - start.tv_sec) + 
                          (end.tv_nsec - start.tv_nsec) / 1e9;
        
        printf("k=%d: wynik=%.10f, czas=%.6f s\n", k, total, time_taken);
    }
    
    return 0;
}