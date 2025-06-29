#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <signal.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define ID_LENGTH 32
#define PING_INTERVAL 30
#define PING_TIMEOUT 60

typedef struct {
    char id[ID_LENGTH];
    struct sockaddr_in addr;
    socklen_t addr_len;
    time_t last_active;
    int active;
} Client;

Client clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int server_running = 1;
int server_fd;

int find_client_by_id(const char *id) {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].active && strcmp(clients[i].id, id) == 0) {
            return i;
        }
    }
    return -1;
}

int find_client_by_addr(const struct sockaddr_in *addr) {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].active &&
            clients[i].addr.sin_addr.s_addr == addr->sin_addr.s_addr &&
            clients[i].addr.sin_port == addr->sin_port) {
            return i;
        }
    }
    return -1;
}

void broadcast_message(const char *sender_id, const char *message) {
    time_t now = time(NULL);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    char msg[BUFFER_SIZE];
    snprintf(msg, sizeof(msg), "MSGALL|%s|%s|%s", sender_id, timestamp, message);

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].active && strcmp(clients[i].id, sender_id) != 0) {
            sendto(server_fd, msg, strlen(msg), 0,
                   (struct sockaddr *)&clients[i].addr, clients[i].addr_len);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_private_message(const char *sender_id, const char *recipient_id, const char *message) {
    time_t now = time(NULL);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    char msg[BUFFER_SIZE];
    snprintf(msg, sizeof(msg), "MSGONE|%s|%s|%s", sender_id, timestamp, message);

    pthread_mutex_lock(&clients_mutex);
    int idx = find_client_by_id(recipient_id);
    if (idx != -1) {
        sendto(server_fd, msg, strlen(msg), 0,
               (struct sockaddr *)&clients[idx].addr, clients[idx].addr_len);
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *ping_thread(void *arg) {
    while (server_running) {
        sleep(PING_INTERVAL);
        time_t now = time(NULL);

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i].active) {
                if (now - clients[i].last_active > PING_TIMEOUT) {
                    printf("Client %s timed out\n", clients[i].id);
                    clients[i].active = 0;
                } else {
                    sendto(server_fd, "PING", 4, 0,
                           (struct sockaddr *)&clients[i].addr, clients[i].addr_len);
                }
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
    return NULL;
}

void handle_client_message(char *buffer, struct sockaddr_in *client_addr, socklen_t addr_len) {
    pthread_mutex_lock(&clients_mutex);
    int idx = find_client_by_addr(client_addr);
    pthread_mutex_unlock(&clients_mutex);
    
    char *client_ip = inet_ntoa(client_addr->sin_addr);
    int client_port = ntohs(client_addr->sin_port);

    if (strncmp(buffer, "REGI|", 5) == 0) {
        char *id = buffer + 5;
        pthread_mutex_lock(&clients_mutex);
        if (find_client_by_id(id) != -1) {
            sendto(server_fd, "ERROR|ID already in use", 23, 0, (struct sockaddr *)client_addr, addr_len);
        } else {
            int new_idx = -1;
            for (int i = 0; i < MAX_CLIENTS; ++i) {
                if (!clients[i].active) {
                    new_idx = i;
                    break;
                }
            }
            
            if (new_idx != -1) {
                strncpy(clients[new_idx].id, id, ID_LENGTH - 1);
                clients[new_idx].addr = *client_addr;
                clients[new_idx].addr_len = addr_len;
                clients[new_idx].last_active = time(NULL);
                clients[new_idx].active = 1;
                sendto(server_fd, "OK", 2, 0, (struct sockaddr *)client_addr, addr_len);
                printf("Registered client: %s from %s:%d\n", id, client_ip, client_port);
            } else {
                 sendto(server_fd, "ERROR|Server is full", 19, 0, (struct sockaddr *)client_addr, addr_len);
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    } else if (idx == -1) {
        sendto(server_fd, "ERROR|Not registered", 20, 0, (struct sockaddr *)client_addr, addr_len);
    } else {
        pthread_mutex_lock(&clients_mutex);
        clients[idx].last_active = time(NULL);
        pthread_mutex_unlock(&clients_mutex);

        if (strcmp(buffer, "LIST") == 0) {
            char response[BUFFER_SIZE] = "LIST|";
            pthread_mutex_lock(&clients_mutex);
            int first = 1;
            for (int i = 0; i < MAX_CLIENTS; ++i) {
                if (clients[i].active) {
                    if (!first) strcat(response, ",");
                    strcat(response, clients[i].id);
                    first = 0;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            sendto(server_fd, response, strlen(response), 0, (struct sockaddr *)client_addr, addr_len);
        } else if (strncmp(buffer, "2ALL|", 5) == 0) {
            broadcast_message(clients[idx].id, buffer + 5);
        } else if (strncmp(buffer, "2ONE|", 5) == 0) {
            char *recipient = strtok(buffer + 5, "|");
            char *message = strtok(NULL, "");
            if (recipient && message) {
                send_private_message(clients[idx].id, recipient, message);
            }
        } else if (strcmp(buffer, "PONG") == 0) {
        } else if (strcmp(buffer, "STOP") == 0) {
            printf("Client %s from %s:%d disconnected.\n", clients[idx].id, client_ip, client_port);
            pthread_mutex_lock(&clients_mutex);
            clients[idx].active = 0;
            pthread_mutex_unlock(&clients_mutex);
        } else {
            sendto(server_fd, "ERROR|Unknown command", 21, 0, (struct sockaddr *)client_addr, addr_len);
        }
    }
}

void handle_signal(int sig) {
    server_running = 0;
    close(server_fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;
    char buffer[BUFFER_SIZE];

    memset(clients, 0, sizeof(clients));

    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    pthread_t ping_tid;
    if (pthread_create(&ping_tid, NULL, ping_thread, NULL) != 0) {
        perror("Failed to create ping thread");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("UDP server listening on port %d...\n", port);

    while (server_running) {
        addr_len = sizeof(client_addr);
        ssize_t recv_len = recvfrom(server_fd, buffer, BUFFER_SIZE - 1, 0,
                                    (struct sockaddr *)&client_addr, &addr_len);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            handle_client_message(buffer, &client_addr, addr_len);
        } else {
            if (server_running) {
                perror("recvfrom failed");
            }
        }
    }

    printf("\nShutting down UDP server...\n");
    pthread_cancel(ping_tid);
    pthread_join(ping_tid, NULL);
    
    return 0;
}