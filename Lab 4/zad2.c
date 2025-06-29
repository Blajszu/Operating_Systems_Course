#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int global = 1;

int main(int argc, char *argv[]) {


    if (argc < 2) {
        printf("Podaj katalog jako argument\n");
        return 1;
    }

    const char *directory = argv[1]; 
    int local = 1;

    pid_t child_pid = fork();

    if(child_pid == 0) {
        printf("Child Process\n");

        global++; local++;

        printf("child pid = %d, parent pid = %d\n", (int)getpid(), (int)getppid());
        printf("child's local = %d, child's global = %d\n", local, global);

        return execl("/bin/ls", "ls", "-l", directory, NULL);
    }
    else {
        printf("Parent Process\n");

        int child_status;
        waitpid(child_pid, &child_status, 0);

        printf("parent pid = %d, child pid = %d\n", (int)getpid(), child_pid);
        printf("Child exit code: %d\n", WEXITSTATUS(child_status));
        printf("Parent's local = %d, parent's global = %d\n", local, global);

        return WEXITSTATUS(child_status);
    }

    return 0;
}
