#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>

#define SERVER_QUEUE_NAME "/server_queue"
#define MAX_CLIENTS 10
#define MAX_MSG_SIZE 512
#define INIT_MSG "INIT"

typedef struct {
    mqd_t client_queue;
    int client_id;
} ClientInfo;

ClientInfo clients[MAX_CLIENTS];
int num_clients = 0;

int main() {
    mq_unlink(SERVER_QUEUE_NAME);
    
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;
    
    mqd_t server_queue = mq_open(
        SERVER_QUEUE_NAME,
        O_CREAT | O_RDONLY,
        0666,
        &attr
    );
    
    if (server_queue == (mqd_t)-1) {
        perror("mq_open (server)");
        exit(1);
    }
    
    printf("Serwer uruchomiony. Oczekiwanie na połączenia...\n");
    
    while (1) {
        char buffer[MAX_MSG_SIZE + 1];
        memset(buffer, 0, sizeof(buffer));
        
        unsigned int priority;
        ssize_t bytes_read = mq_receive(server_queue, buffer, MAX_MSG_SIZE, &priority);
        
        if (bytes_read == -1) {
            perror("mq_receive");
            continue;
        }
        
        if (strncmp(buffer, INIT_MSG, strlen(INIT_MSG)) == 0) {
            char *token = strtok(buffer, " ");
            token = strtok(NULL, " ");
            
            if (token == NULL) {
                fprintf(stderr, "Nieprawidłowy format wiadomości inicjującej\n");
                continue;
            }
            
            char *client_queue_name = token;
            mqd_t client_queue = mq_open(client_queue_name, O_WRONLY);
            
            if (client_queue == (mqd_t)-1) {
                perror("mq_open (client)");
                continue;
            }
            
            if (num_clients < MAX_CLIENTS) {
                clients[num_clients].client_queue = client_queue;
                clients[num_clients].client_id = num_clients;
                
                char client_id[10];
                snprintf(client_id, sizeof(client_id), "%d", num_clients);
                
                if (mq_send(client_queue, client_id, sizeof(client_id), 1) == -1) {
                    perror("mq_send (ID)");
                }
                
                printf("Klient %d połączony: %s\n", num_clients, client_queue_name);
                num_clients++;
            } else {
                fprintf(stderr, "Osiągnięto maksymalną liczbę klientów\n");
                mq_close(client_queue);
            }
        } 
        else {
            char *token = strtok(buffer, "|");
            if (token == NULL) {
                fprintf(stderr, "Nieprawidłowy format wiadomości (brak ID nadawcy)\n");
                continue;
            }
            
            int sender_id = atoi(token);
            char *message = strtok(NULL, "");
            
            if (message != NULL) {
                printf("Odebrano od klienta %d: %s\n", sender_id, message);
                
                for (int i = 0; i < num_clients; i++) {
                    if (clients[i].client_id != sender_id) {
                        if (mq_send(clients[i].client_queue, message, strlen(message) + 1, 1) == -1) {
                            perror("mq_send (message)");
                        }
                    }
                }
            } else {
                fprintf(stderr, "Nieprawidłowy format wiadomości (brak treści)\n");
            }
        }
    }
    
    for (int i = 0; i < num_clients; i++) {
        mq_close(clients[i].client_queue);
    }
    
    mq_close(server_queue);
    mq_unlink(SERVER_QUEUE_NAME);
    
    return 0;
}