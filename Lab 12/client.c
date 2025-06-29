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
struct sockaddr_in serv_addr;

void handle_signal(int sig) {
    if (sock > 0) {
        sendto(sock, "STOP", 4, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        close(sock);
    }
    running = false;
}

void *receiver(void *arg) {
    char buffer[BUFFER_SIZE] = {0};
    
    while (running) {
        ssize_t read_size = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0, NULL, NULL);
        if (read_size <= 0) {
            if (running) {
                printf("Server stopped responding or error occurred.\n");
            }
            running = false;
            break;
        }
        
        buffer[read_size] = '\0';
        
        if (strncmp(buffer, "PING", 4) == 0) {
            if (sendto(sock, "PONG", 4, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                perror("pong send failed");
                running = false;
                break;
            }
        }
        else if (strncmp(buffer, "MSGALL|", 7) == 0) {
            char *sender = strtok(buffer + 7, "|");
            char *timestamp = strtok(NULL, "|");
            char *message = strtok(NULL, "");
            printf("\n[%s] %s to all: %s\n> ", timestamp, sender, message);
            fflush(stdout);
        }
        else if (strncmp(buffer, "MSGONE|", 7) == 0) {
            char *sender = strtok(buffer + 7, "|");
            char *timestamp = strtok(NULL, "|");
            char *message = strtok(NULL, "");
            printf("\n[%s] %s (private): %s\n> ", timestamp, sender, message);
            fflush(stdout);
        }
        else if (strncmp(buffer, "LIST|", 5) == 0) {
            printf("\nActive clients: %s\n> ", buffer + 5);
            fflush(stdout);
        }
        else if (strncmp(buffer, "ERROR|", 6) == 0) {
            printf("\nError from server: %s\n> ", buffer + 6);
            fflush(stdout);
        }
        else {
            printf("\nServer: %s\n> ", buffer);
            fflush(stdout);
        }
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <id> <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE] = {0};
    strncpy(client_id, argv[1], ID_LENGTH - 1);
    client_id[ID_LENGTH - 1] = '\0';
    int port = atoi(argv[3]);

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, argv[2], &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }
    
    snprintf(buffer, BUFFER_SIZE, "REGI|%s", client_id);
    if (sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("sendto failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    ssize_t read_size = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0, NULL, NULL);
    if (read_size <= 0) {
        printf("Registration failed: No response from server.\n");
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    tv.tv_sec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    buffer[read_size] = '\0';
    if (strncmp(buffer, "OK", 2) != 0) {
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
        printf("> ");
        if (!fgets(buffer, BUFFER_SIZE, stdin)) {
            running = false;
            break;
        }
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strlen(buffer) == 0) continue;

        if (strncmp(buffer, "LIST", 4) == 0) {
            if (sendto(sock, "LIST", 4, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                perror("sendto failed"); break;
            }
        } else if (strncmp(buffer, "2ALL ", 5) == 0) {
            char msg[BUFFER_SIZE];
            snprintf(msg, BUFFER_SIZE, "2ALL|%s", buffer + 5);
            if (sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                perror("sendto failed"); break;
            }
        } else if (strncmp(buffer, "2ONE ", 5) == 0) {
            char *recipient = strtok(buffer + 5, " ");
            char *message = strtok(NULL, "");
            if (recipient && message) {
                char msg[BUFFER_SIZE];
                snprintf(msg, BUFFER_SIZE, "2ONE|%s|%s", recipient, message);
                if (sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                    perror("sendto failed"); break;
                }
            } else {
                printf("Usage: 2ONE <recipient_id> <message>\n");
            }
        } else if (strncmp(buffer, "STOP", 4) == 0) {
            if (sendto(sock, "STOP", 4, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                perror("sendto failed");
            }
            running = false;
        } else {
            printf("Unknown command\n");
        }
    }

    if (isatty(STDIN_FILENO)) {
        pthread_cancel(recv_thread);
    }
    pthread_join(recv_thread, NULL);
    close(sock);
    printf("\nClient stopped\n");
    return 0;
}