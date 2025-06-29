#include <stdio.h>

int collatz_conjecture(int input) {
    if(input % 2 == 0) {
        return input / 2;
    } else {
        return (input * 3) + 1;
    }
}

int test_collatz_convergence(int input, int max_iter, int *steps) {
    
    int i = 0;
    steps[i] = input;


    while(steps[i] != 1 && i < max_iter - 1) {
        steps[i + 1] = collatz_conjecture(steps[i]);
        i++;
    } 

    if (steps[i] != 1) {
        return 0;
    }

    return i + 1;
}