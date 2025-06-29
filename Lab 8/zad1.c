#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>

#define MAX_QUEUE 10
#define JOB_LEN 10

#define SEM_MUTEX 0
#define SEM_FULL 1
#define SEM_EMPTY 2

typedef struct {
    char jobs[MAX_QUEUE][JOB_LEN];
    int head, tail;
} Queue;

void wait_semaphore(int semid, int semnum) {
    struct sembuf op = { semnum, -1, 0 };
    semop(semid, &op, 1);
}

void signal_semaphore(int semid, int semnum) {
    struct sembuf op = { semnum, +1, 0 };
    semop(semid, &op, 1);
}

void generate_job(char *job) {
    for (int i = 0; i < JOB_LEN; i++) {
        job[i] = 'A' + rand() % 26;
    }
}

void get_job(char *job) {
    for (int i = 0; i < JOB_LEN; i++) {
        job[i] = 'A' + rand() % 26;
    }
}

void user(Queue *queue, int semid, int id) {
    srand(time(NULL) ^ getpid());
    while (1) {
        char job[JOB_LEN];
        generate_job(job);

        wait_semaphore(semid, SEM_EMPTY);
        wait_semaphore(semid, SEM_MUTEX);

        for (int i = 0; i < JOB_LEN; i++) {
            queue->jobs[queue->tail][i] = job[i];
        }

        queue->tail = (queue->tail + 1) % MAX_QUEUE;

        signal_semaphore(semid, SEM_MUTEX);
        signal_semaphore(semid, SEM_FULL);
        printf("User %d added job: %s\n", id, job);
        sleep(rand() % 3 + 1);
    }
}

void printer(Queue *queue, int semid, int id) {
    srand(time(NULL) ^ getpid());
    while (1) {

        wait_semaphore(semid, SEM_FULL);
        wait_semaphore(semid, SEM_MUTEX);        

        char job[JOB_LEN];
        get_job(job);

        queue->head = (queue->head + 1) % MAX_QUEUE;

        signal_semaphore(semid, SEM_MUTEX);
        signal_semaphore(semid, SEM_EMPTY);

        printf("Printer %d got job: %s\n", id, job);

        for (int i = 0; i < JOB_LEN; i++) {
            printf("Printer %d processing job: %c\n", id, job[i]);
            sleep(1);
        }
    }
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <number_of_users> <number_of_printers>\n", argv[0]);
        return 1;
    }

    key_t shmkey = ftok("./file", 'A');
    key_t semkey = ftok("./file", 'B');

    int shmid = shmget(shmkey, sizeof(Queue), IPC_CREAT | 0666);
    Queue *queue = (Queue *)shmat(shmid, NULL, 0);

    int semid = semget(semkey, 3, IPC_CREAT | 0666);
    semctl(semid, SEM_MUTEX, SETVAL, 1);
    semctl(semid, SEM_FULL, SETVAL, 0);
    semctl(semid, SEM_EMPTY, SETVAL, MAX_QUEUE);

    int users_number = atoi(argv[1]);
    int printers_number = atoi(argv[2]);

    for (int i = 0; i < users_number; i++)
        if (fork() == 0) user(queue, semid, i);

    for (int i = 0; i < printers_number; i++)
        if (fork() == 0) printer(queue, semid, i);

    while (1) pause();
    return 0;
}
