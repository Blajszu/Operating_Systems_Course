#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

volatile sig_atomic_t confirmation_received = 0;

void confirmation_handler() {
    confirmation_received = 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Użycie: %s <PID catchera> <tryb>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t catcher_pid = atoi(argv[1]);
    int mode = atoi(argv[2]);

    signal(SIGUSR1, confirmation_handler);

    union sigval value;
    value.sival_int = mode;

    if (sigqueue(catcher_pid, SIGUSR1, value) == -1) {
        perror("sigqueue");
        exit(EXIT_FAILURE);
    }

    while (!confirmation_received) {
        pause();
    }

    printf("Potwierdzenie otrzymane. Sender kończy działanie.\n");
    return 0;
}