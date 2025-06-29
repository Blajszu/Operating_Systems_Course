#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>

volatile sig_atomic_t mode = 0;
volatile sig_atomic_t count = 0;
volatile sig_atomic_t sender_pid = 0;
volatile sig_atomic_t should_exit = false;

void handle_signal(int sig, siginfo_t *info, void *context) {
    (void)sig;
    (void)context;
    mode = info->si_value.sival_int;
    sender_pid = info->si_pid;
    count++;
    
    kill(sender_pid, SIGUSR1);
}

void handle_ctrl_c(int sig) {
    (void)sig;
    if (mode == 4) {
        printf("Wciśnięto CTRL+C\n");
    } else if (mode != 3) {
        exit(0);
    }
}

void print_numbers() {
    int num = 1;
    while (mode == 2) {
        printf("%d\n", num++);
        sleep(1);
    }
}

int main() {
    printf("Catcher PID: %d\n", getpid());

    struct sigaction sa;
    sa.sa_sigaction = handle_signal;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGINT, handle_ctrl_c) == SIG_ERR) {
        perror("signal");
        exit(EXIT_FAILURE);
    }

    while (!should_exit) {
        switch (mode) {
            case 1:
                printf("Liczba otrzymanych żądań: %d\n", count);
                mode = 0;
                break;
            case 2:
                print_numbers();
                break;
            case 3:
                signal(SIGINT, SIG_IGN);
                break;
            case 4:
                signal(SIGINT, handle_ctrl_c);
                break;
            case 5:
                should_exit = true;
                break;
            default:
                pause();
                break;
        }
    }

    printf("Catcher kończy działanie\n");
    return 0;
}