#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

void handler() {
    printf("Otrzymano sygnał SIGUSR1\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Użycie: %s [none|ignore|handler|mask]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "none") == 0) {
        printf("Ustawiono domyślną reakcję na SIGUSR1\n");
    } 
    else if (strcmp(argv[1], "ignore") == 0) {
        signal(SIGUSR1, SIG_IGN);
        printf("Ustawiono ignorowanie SIGUSR1\n");
    } 
    else if (strcmp(argv[1], "handler") == 0) {
        signal(SIGUSR1, handler);
        printf("Zainstalowano handler dla SIGUSR1\n");
    } 
    else if (strcmp(argv[1], "mask") == 0) {
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &mask, NULL);
        printf("Zamaskowano SIGUSR1\n");
    } 
    else {
        printf("Nieznana opcja: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    printf("Wysyłam SIGUSR1 do siebie...\n");
    raise(SIGUSR1);

    if (strcmp(argv[1], "mask") == 0) {
        sigset_t pending;
        sigpending(&pending);
        if (sigismember(&pending, SIGUSR1)) {
            printf("Sygnał SIGUSR1 jest oczekujący\n");
        } else {
            printf("Sygnał SIGUSR1 nie jest oczekujący\n");
        }
    }

    printf("Kontynuacja programu...\n");
    sleep(1);
    printf("Zakończenie programu\n");

    return 0;
}