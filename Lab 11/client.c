#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>
#include <pthread.h>

#define BUFFER_SIZE 1024
#define ID_LENGTH 32

int sock = 0;
volatile bool running = true;
char client_id[ID_LENGTH];

void handle_signal(int sig) {
    if (sock > 0) {
        send(sock, "STOP", 4, 0);
        close(sock);
    }
    running = false;
}

void *receiver(void *arg) {
    char buffer[BUFFER_SIZE] = {0};
    
    while (running) {
        ssize_t read_size = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (read_size <= 0) {
            printf("Disconnected from server\n");
            running = false;
            break;
        }
        
        buffer[read_size] = '\0';
        
        if (strncmp(buffer, "PING", 4) == 0) {
            if (send(sock, "PONG", 4, 0) < 0) {
                perror("pong send failed");
                running = false;
                break;
            }
        }
        else if (strncmp(buffer, "MSGALL|", 7) == 0) {
            char *sender = strtok(buffer + 7, "|");
            char *timestamp = strtok(NULL, "|");
            char *message = strtok(NULL, "");
            printf("[%s] %s to all: %s\n", timestamp, sender, message);
        }
        else if (strncmp(buffer, "MSGONE|", 7) == 0) {
            char *sender = strtok(buffer + 7, "|");
            char *timestamp = strtok(NULL, "|");
            char *message = strtok(NULL, "");
            printf("[%s] %s (private): %s\n", timestamp, sender, message);
        }
        else if (strncmp(buffer, "LIST|", 5) == 0) {
            printf("Active clients: %s\n", buffer + 5);
        }
        else if (strncmp(buffer, "ERROR|", 6) == 0) {
            printf("Error: %s\n", buffer + 6);
        }
        else {
            printf("Server: %s\n", buffer);
        }
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <id> <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    strncpy(client_id, argv[1], ID_LENGTH - 1);
    client_id[ID_LENGTH - 1] = '\0';
    int port = atoi(argv[3]);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, argv[2], &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    snprintf(buffer, BUFFER_SIZE, "REGI|%s", client_id);
    if (send(sock, buffer, strlen(buffer), 0) < 0) {
        perror("send failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    ssize_t read_size = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (read_size <= 0 || strncmp(buffer, "OK", 2) != 0) {
        printf("Registration failed: %s\n", buffer);
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server as %s\n", client_id);

    signal(SIGINT, handle_signal);

    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receiver, NULL) != 0) {
        perror("pthread_create failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    while (running) {
        printf("Enter command (LIST, 2ALL <msg>, 2ONE <id> <msg>, STOP): ");
        if (!fgets(buffer, BUFFER_SIZE, stdin)) {
            break;
        }
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strlen(buffer) == 0) continue;

        if (strncmp(buffer, "LIST", 4) == 0) {
            if (send(sock, "LIST", 4, 0) < 0) {
                perror("send failed");
                break;
            }
        }
        else if (strncmp(buffer, "2ALL ", 5) == 0) {
            char msg[BUFFER_SIZE];
            snprintf(msg, BUFFER_SIZE, "2ALL|%s", buffer + 5);
            if (send(sock, msg, strlen(msg), 0) < 0) {
                perror("send failed");
                break;
            }
        }
        else if (strncmp(buffer, "2ONE ", 5) == 0) {
            char *recipient = strtok(buffer + 5, " ");
            char *message = strtok(NULL, "");
            
            if (recipient && message) {
                char msg[BUFFER_SIZE];
                snprintf(msg, BUFFER_SIZE, "2ONE|%s|%s", recipient, message);
                if (send(sock, msg, strlen(msg), 0) < 0) {
                    perror("send failed");
                    break;
                }
            } else {
                printf("Usage: 2ONE <recipient_id> <message>\n");
            }
        }
        else if (strncmp(buffer, "STOP", 4) == 0) {
            if (send(sock, "STOP", 4, 0) < 0) {
                perror("send failed");
            }
            running = false;
        }
        else {
            printf("Unknown command\n");
        }
    }

    pthread_join(recv_thread, NULL);
    close(sock);
    printf("Client stopped\n");
    return 0;
}