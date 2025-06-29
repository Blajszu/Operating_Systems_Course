#include <stdio.h>
#include <stdlib.h>

#ifdef DYNAMIC_LOAD
#include <dlfcn.h>
#endif

#ifndef DYNAMIC_LOAD
#include "collatz.h"
#endif

int main(void) {
    int input;
    int max_iter;

    printf("Podaj liczbę początkową: ");
    scanf("%d", &input);

    printf("Podaj maksymalną liczbę iteracji: ");
    scanf("%d", &max_iter);

    int steps[max_iter];
    int result;

#ifdef DYNAMIC_LOAD
    void* handle = dlopen("./libcollatz.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        return 1;
    }

    int (*collatz_conjecture)(int) = dlsym(handle, "collatz_conjecture");
    int (*test_collatz_convergence)(int, int, int*) = dlsym(handle, "test_collatz_convergence");

    if (!collatz_conjecture || !test_collatz_convergence) {
        fprintf(stderr, "%s\n", dlerror());
        dlclose(handle);
        return 1;
    }

    result = test_collatz_convergence(input, max_iter, steps);

    dlclose(handle);
#else
    result = test_collatz_convergence(input, max_iter, steps);
#endif

    if(result != 0) {
        printf("Osiągnięto 1 przy %d iteracjach.\n", result);

        printf("Kolejne kroki:\n");

        for(int i = 0; i < max_iter; i++) {
            printf("%d\n", steps[i]);
            if(steps[i] == 1)
                break;
        }
    } else {
        printf("Nie udało się osiągnąć 1 przy %d iteracjach.\n", max_iter);
    }

    return 0;
}