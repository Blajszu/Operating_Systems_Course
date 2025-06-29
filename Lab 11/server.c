#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define ID_LENGTH 32
#define PING_INTERVAL 30
#define PING_TIMEOUT 60

typedef struct {
    int socket;
    char id[ID_LENGTH];
    time_t last_active;
    pthread_t thread;
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int server_running = 1;

void broadcast_message(const char* sender_id, const char* message) {
    time_t now = time(NULL);
    char timestamp[20];
    strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    char formatted_msg[BUFFER_SIZE];
    snprintf(formatted_msg, BUFFER_SIZE, "MSGALL|%s|%s|%s", sender_id, timestamp, message);
    
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != 0 && strcmp(clients[i].id, sender_id) != 0) {
            if (send(clients[i].socket, formatted_msg, strlen(formatted_msg), 0) < 0) {
                perror("broadcast send failed");
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_private_message(const char* sender_id, const char* recipient_id, const char* message) {
    time_t now = time(NULL);
    char timestamp[20];
    strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    char formatted_msg[BUFFER_SIZE];
    snprintf(formatted_msg, BUFFER_SIZE, "MSGONE|%s|%s|%s", sender_id, timestamp, message);
    
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != 0 && strcmp(clients[i].id, recipient_id) == 0) {
            if (send(clients[i].socket, formatted_msg, strlen(formatted_msg), 0) < 0) {
                perror("private message send failed");
            }
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(int socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket == socket) {
            printf("Client %s disconnected\n", clients[i].id);
            close(clients[i].socket);
            clients[i].socket = 0;
            memset(clients[i].id, 0, ID_LENGTH);
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg) {
    int client_socket = *(int*)arg;
    free(arg);
    
    char buffer[BUFFER_SIZE] = {0};
    char client_id[ID_LENGTH] = {0};
    
    ssize_t read_size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (read_size <= 0) {
        close(client_socket);
        return NULL;
    }
    
    buffer[read_size] = '\0';
    
    if (strncmp(buffer, "REGI|", 5) == 0) {
        strncpy(client_id, buffer + 5, ID_LENGTH - 1);
        client_id[ID_LENGTH - 1] = '\0';
        
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket != 0 && strcmp(clients[i].id, client_id) == 0) {
                send(client_socket, "ERROR|ID already in use", 23, 0);
                close(client_socket);
                pthread_mutex_unlock(&clients_mutex);
                return NULL;
            }
        }
        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket == 0) {
                clients[i].socket = client_socket;
                strcpy(clients[i].id, client_id);
                clients[i].last_active = time(NULL);
                client_count++;
                
                if (send(client_socket, "OK", 2, 0) < 0) {
                    perror("registration send failed");
                    close(client_socket);
                    clients[i].socket = 0;
                    client_count--;
                } else {
                    printf("New client registered: %s\n", client_id);
                }
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    } else {
        send(client_socket, "ERROR|First message must be REGI", 31, 0);
        close(client_socket);
        return NULL;
    }
    
    while (server_running) {
        read_size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (read_size <= 0) {
            break;
        }
        
        buffer[read_size] = '\0';
        
        if (strncmp(buffer, "PONG", 4) == 0) {
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].socket == client_socket) {
                    clients[i].last_active = time(NULL);
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            continue;
        }
        
        if (strncmp(buffer, "LIST", 4) == 0) {
            pthread_mutex_lock(&clients_mutex);
            char response[BUFFER_SIZE] = "LIST|";
            int pos = strlen(response);
            
            for (int i = 0; i < MAX_CLIENTS && pos < BUFFER_SIZE - 1; i++) {
                if (clients[i].socket != 0) {
                    int len = snprintf(response + pos, BUFFER_SIZE - pos, "%s%s", 
                                     (pos > 5 ? "," : ""), clients[i].id);
                    if (len > 0) {
                        pos += len;
                    }
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            
            if (send(client_socket, response, strlen(response), 0) < 0) {
                perror("list send failed");
                break;
            }
        }
        else if (strncmp(buffer, "2ALL|", 5) == 0) {
            const char* message = buffer + 5;
            broadcast_message(client_id, message);
        }
        else if (strncmp(buffer, "2ONE|", 5) == 0) {
            char* recipient = strtok(buffer + 5, "|");
            char* message = strtok(NULL, "");
            
            if (recipient && message) {
                send_private_message(client_id, recipient, message);
            } else {
                if (send(client_socket, "ERROR|Invalid 2ONE format", 25, 0) < 0) {
                    perror("error send failed");
                }
            }
        }
        else if (strncmp(buffer, "STOP", 4) == 0) {
            break;
        }
        else {
            if (send(client_socket, "ERROR|Unknown command", 21, 0) < 0) {
                perror("error send failed");
            }
        }
    }
    
    remove_client(client_socket);
    return NULL;
}

void *alive_checker(void *arg) {
    while (server_running) {
        sleep(PING_INTERVAL);
        
        time_t now = time(NULL);
        pthread_mutex_lock(&clients_mutex);
        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket != 0) {
                if (now - clients[i].last_active > PING_TIMEOUT) {
                    printf("Client %s timed out (no PONG response)\n", clients[i].id);
                    close(clients[i].socket);
                    clients[i].socket = 0;
                    memset(clients[i].id, 0, ID_LENGTH);
                    client_count--;
                } else {
                    if (send(clients[i].socket, "PING", 4, 0) < 0) {
                        perror("ping send failed");
                        close(clients[i].socket);
                        clients[i].socket = 0;
                        memset(clients[i].id, 0, ID_LENGTH);
                        client_count--;
                    }
                }
            }
        }
        
        pthread_mutex_unlock(&clients_mutex);
    }
    return NULL;
}

void handle_signal(int sig) {
    server_running = 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int port = atoi(argv[1]);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d\n", port);

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    pthread_t alive_thread;
    if (pthread_create(&alive_thread, NULL, alive_checker, NULL) != 0) {
        perror("pthread_create failed");
        exit(EXIT_FAILURE);
    }

    while (server_running) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            if (server_running) perror("accept");
            break;
        }

        pthread_mutex_lock(&clients_mutex);
        if (client_count >= MAX_CLIENTS) {
            if (send(new_socket, "ERROR|Server is full", 19, 0) < 0) {
                perror("send failed");
            }
            close(new_socket);
        } else {
            int *client_sock = malloc(sizeof(int));
            if (!client_sock) {
                perror("malloc failed");
                close(new_socket);
            } else {
                *client_sock = new_socket;
                
                if (pthread_create(&clients[client_count].thread, NULL, handle_client, (void*)client_sock) != 0) {
                    perror("pthread_create failed");
                    close(new_socket);
                    free(client_sock);
                }
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }

    printf("Shutting down server...\n");
    
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != 0) {
            close(clients[i].socket);
        }
    }

    pthread_mutex_unlock(&clients_mutex);
    close(server_fd);
    pthread_join(alive_thread, NULL);

    return 0;
}