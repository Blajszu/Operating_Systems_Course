#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Podaj liczbę procesów jako argument\n");
        return 1;
    }

    int num_processes = atoi(argv[1]);
    pid_t child_pid;

    for(int i = 1; i <= (int)num_processes; i++) {
        
        child_pid = fork();
        
        if(child_pid == 0) {
            printf("%d. Proces dziecka o pid: %d, proces macierzysty pid: %d\n", i, (int)getpid(), (int)getppid());
            return 0;
        }
        else {
            waitpid(child_pid, NULL, 0);
        } 
    }
    
    printf("PID glownego programu: %d\n", (int)getpid());

    return 0;
}
