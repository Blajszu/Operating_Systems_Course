#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>

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

int main(void) {
    int start;
    int end;
    double size;

    int to_calculate = open("to_calculate", O_RDONLY);
    int result = open("result", O_WRONLY);

    read(to_calculate, &start, sizeof(int));
    read(to_calculate, &end, sizeof(int));
    read(to_calculate, &size, sizeof(double));

    double calculated_result = calculate_integral(start, end, size);
    write(result, &calculated_result, sizeof(calculated_result));
}