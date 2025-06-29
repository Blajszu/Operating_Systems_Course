#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(void) {
    int start;
    int end;
    double size;

    mkfifo("to_calculate", 0666);
    mkfifo("result", 0666);

    int to_calculate = open("to_calculate", O_WRONLY);
    int result = open("result", O_RDONLY);

    printf("Podaj początek przedziału: ");
    scanf("%d", &start);
    printf("Podaj koniec przedziału: ");
    scanf("%d", &end);
    printf("Podaj szerokosc prostokata: ");
    scanf("%lf", &size);

    write(to_calculate, &start, sizeof(start));
    write(to_calculate, &end, sizeof(end));
    write(to_calculate, &size, sizeof(size));

    double calculated_result;
    read(result, &calculated_result, sizeof(double));

    printf("Wynik: %f\n", calculated_result);
}