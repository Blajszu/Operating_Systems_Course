#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>
#include <signal.h>

#define SERVER_QUEUE_NAME "/server_queue"
#define CLIENT_QUEUE_NAME "/client_%d"
#define MAX_MSG_SIZE 512
#define MAX_USERNAME_SIZE 32
#define INIT_MSG "INIT"

int client_id;
mqd_t client_queue;

void receiver_thread(void) {
    char buffer[MAX_MSG_SIZE + 1];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        
        ssize_t bytes_read = mq_receive(client_queue, buffer, MAX_MSG_SIZE, NULL);
        if (bytes_read == -1) {
            perror("mq_receive (client)");
            continue;
        }
        
        buffer[bytes_read] = '\0';
        printf("%s\n", buffer);
    }
}

int main() {
    char username[MAX_USERNAME_SIZE];
    printf("Podaj nazwe: ");
    scanf("%31s", username);
    
    char client_queue_name[32];
    sprintf(client_queue_name, CLIENT_QUEUE_NAME, getpid());
    
    mq_unlink(client_queue_name);
    
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;
    
    client_queue = mq_open(
        client_queue_name,
        O_CREAT | O_RDONLY,
        0666,
        &attr
    );
    
    if (client_queue == (mqd_t)-1) {
        perror("mq_open (client)");
        exit(1);
    }
    
    mqd_t server_queue = mq_open(SERVER_QUEUE_NAME, O_WRONLY);
    if (server_queue == (mqd_t)-1) {
        perror("mq_open (server from client)");
        exit(1);
    }
    
    char init_msg[MAX_MSG_SIZE];
    snprintf(init_msg, sizeof(init_msg), "%s %s", INIT_MSG, client_queue_name);
    
    if(mq_send(server_queue, init_msg, strlen(init_msg) + 1, 1) == -1) {
        perror("mq_send (INIT)");
        exit(1);
    }
    
    char id_buffer[MAX_MSG_SIZE];
    memset(id_buffer, 0, sizeof(id_buffer));
    
    if (mq_receive(client_queue, id_buffer, sizeof(id_buffer), NULL) == -1) {
        perror("mq_receive (ID)");
        exit(1);
    }
    
    client_id = atoi(id_buffer);
    printf("Moje ID: %d\n", client_id);
    
    pid_t child_pid = fork();
    if (child_pid == 0) {
        receiver_thread();
        exit(0);
    }
    
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    char message[MAX_MSG_SIZE];
    char input[MAX_MSG_SIZE-MAX_USERNAME_SIZE-5];
    
    while (fgets(input, sizeof(input), stdin)) {
        input[strcspn(input, "\n")] = '\0';
        
        snprintf(message, sizeof(message), "%d|%s: %s", client_id, username, input);
        
        if (mq_send(server_queue, message, strlen(message) + 1, 0) == -1) {
            perror("mq_send");
        }
    }
    
    kill(child_pid, SIGTERM);
    
    mq_close(client_queue);
    mq_unlink(client_queue_name);
    mq_close(server_queue);
    
    return 0;
}