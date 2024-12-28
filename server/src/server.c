#include "../include/server.h"
#include "../include/room.h"
#include "../include/declarations.h"
#include "../include/exam.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>

extern Question questions[MAX_QUESTIONS];  
extern int num_questions; 

#define MAX_FDS (MAX_CLIENTS + 1)  // +1 for server socket

Server* create_server(void) {
    Server* server = malloc(sizeof(Server));
    if (!server) {
        return NULL;
    }

    // Create socket
    if ((server->server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        free(server);
        return NULL;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        close(server->server_fd);
        free(server);
        return NULL;
    }

    // Configure server address
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(server->server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server->server_fd);
        free(server);
        return NULL;
    }

    // Listen for connections
    if (listen(server->server_fd, 3) < 0) {
        perror("Listen failed");
        close(server->server_fd);
        free(server);
        return NULL;
    }

    // Initialize clients array
    server->clients = calloc(MAX_CLIENTS, sizeof(ClientInfo));
    if (!server->clients) {
        perror("Failed to allocate clients array");
        close(server->server_fd);
        free(server);
        return NULL;
    }

    // Initialize poll file descriptors
    server->pfds = calloc(MAX_FDS, sizeof(struct pollfd));
    if (!server->pfds) {
        perror("Failed to allocate poll fds");
        free(server->clients);
        close(server->server_fd);
        free(server);
        return NULL;
    }

    // Add server socket to poll set
    server->pfds[0].fd = server->server_fd;
    server->pfds[0].events = POLLIN;
    server->fd_count = 1;
    server->running = 1;

    return server;
}

void destroy_server(Server* server) {
    if (!server) return;

    // Close all client connections
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].active) {
            close(server->clients[i].fd);
        }
    }

    // Free allocated memory
    free(server->clients);
    free(server->pfds);

    // Close server socket
    close(server->server_fd);
    free(server);
}

static int add_to_pfds(Server* server, int new_fd) {
    if (server->fd_count == MAX_FDS) return -1;

    server->pfds[server->fd_count].fd = new_fd;
    server->pfds[server->fd_count].events = POLLIN;
    server->fd_count++;
    return 0;
}

static void del_from_pfds(Server* server, int i) {
    server->pfds[i] = server->pfds[server->fd_count-1];
    server->fd_count--;
}

void start_server(Server* server) {
    printf("Server starting... waiting for connections\n");

    while (server->running) {
        int poll_count = poll(server->pfds, server->fd_count, -1);

        if (poll_count < 0) {
            if (errno == EINTR) continue;
            perror("Poll error");
            break;
        }

        for (int i = 0; i < server->fd_count; i++) {
            if (server->pfds[i].revents & POLLIN) {
                if (server->pfds[i].fd == server->server_fd) {
                    handle_new_connection(server);
                } else {
                    char buffer[BUFFER_SIZE];
                    memset(buffer, 0, BUFFER_SIZE);
                    int valread = read(server->pfds[i].fd, buffer, BUFFER_SIZE - 1);

                    if (valread <= 0) {
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (server->clients[j].active &&
                                server->clients[j].fd == server->pfds[i].fd) {
                                handle_disconnection(server, j);
                                del_from_pfds(server, i);
                                i--;
                                break;
                            }
                        }
                    } else {
                        buffer[valread] = '\0';
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (server->clients[j].active &&
                                server->clients[j].fd == server->pfds[i].fd) {
                                handle_client_message(server, j, buffer);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void handle_new_connection(Server* server) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int new_fd = accept(server->server_fd, (struct sockaddr*)&client_addr, &addr_len);
    if (new_fd < 0) {
        perror("Accept failed");
        return;
    }

    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (!server->clients[i].active) {
            break;
        }
    }

    if (i == MAX_CLIENTS || add_to_pfds(server, new_fd) < 0) {
        printf("Too many clients. Connection rejected.\n");
        close(new_fd);
        return;
    }

    server->clients[i].fd = new_fd;
    server->clients[i].active = 1;
    server->clients[i].authenticated = 0;
    server->clients[i].current_room_id = -1;
    server->clients[i].current_question = -1;
    server->clients[i].score = 0;

    printf("New client connected on socket %d\n", new_fd);
}

void handle_disconnection(Server* server, int client_index) {
    ClientInfo* client = &server->clients[client_index];

    if (client->current_room_id != -1) {
        leave_exam_room(client->current_room_id, client->username, client);
    }

    printf("Client disconnected from socket %d\n", client->fd);
    close(client->fd);
    memset(client, 0, sizeof(ClientInfo));
}

void handle_client_message(Server* server, int client_index, char* buffer) {
    ClientInfo* client = &server->clients[client_index];
    char response[BUFFER_SIZE];
   
    // Xóa newline ở cuối buffer
    buffer[strcspn(buffer, "\n")] = 0;
    printf("Received from client %d: '%s'\n", client->fd, buffer);

    // Xử lý authentication
    if (!client->authenticated) {
        if (strncmp(buffer, "REGISTER", 8) == 0 || strncmp(buffer, "LOGIN", 5) == 0) {
            printf("Processing authentication: %s\n", buffer);
            handle_authentication(client, buffer);
        }
        return;
    }

    printf("Command from user %s: '%s'\n", client->username, buffer);

    // ===== EXAM ROOM MODE =====
    // Trả lời câu hỏi trong room
    if (strncmp(buffer, "SUBMIT_ANSWER", 13) == 0) {
        printf("Processing exam answer from %s\n", client->username);
        char answer = '\0';
        int i = 13;
        while (buffer[i] == ' ') i++;
        answer = buffer[i];
        
        if (answer >= 'a' && answer <= 'd') answer = answer - 'a' + 'A';
        
        if (answer >= 'A' && answer <= 'D') {
            printf("Valid exam answer received: %c\n", answer);
            handle_answer(client, answer);
        } else {
            printf("Invalid exam answer received: %c\n", answer);
            snprintf(response, BUFFER_SIZE, "Invalid answer. Please enter A, B, C, or D\n");
            send(client->fd, response, strlen(response), 0);
        }
        return;
    }

    // Bắt đầu thi trong room
    if (strcmp(buffer, "START_EXAM") == 0) {
        if (client->current_room_id != -1) {
            ExamRoom* room = get_room(client->current_room_id);
            if (room) {
                if (is_room_creator(client->current_room_id, client->username)) {
                    if (room->status == 0) {
                        start_exam(server, room);
                    } else {
                        send(client->fd, "Exam already in progress\n", strlen("Exam already in progress\n"), 0);
                    }
                } else {
                    send(client->fd, "Only room creator can start exam\n", strlen("Only room creator can start exam\n"), 0);
                }
            } else {
                send(client->fd, "Room not found\n", strlen("Room not found\n"), 0);
            }
        } else {
            send(client->fd, "You are not in any room\n", strlen("You are not in any room\n"), 0);
        }
        return;
    }

    // ===== ROOM MANAGEMENT =====
    // Tạo room mới
    if (strncmp(buffer, "CREATE_ROOM", 11) == 0) {
        char* room_name = buffer + 12;
        if (strlen(room_name) > 0) {
            int room_id = create_exam_room(room_name, client->username);
            if (room_id > 0) {
                client->current_room_id = room_id;
                snprintf(response, BUFFER_SIZE, "ROOM_CREATED %d\n", room_id);
            } else {
                snprintf(response, BUFFER_SIZE, "CREATE_FAILED\n");
            }
            send(client->fd, response, strlen(response), 0);
        }
        return;
    }

    // Liệt kê rooms
    if (strcmp(buffer, "LIST_ROOMS") == 0) {
        get_room_list(response);
        send(client->fd, response, strlen(response), 0);
        return;
    }

    // Tham gia room
    if (strncmp(buffer, "JOIN_ROOM", 9) == 0) {
        int room_id = atoi(buffer + 10);
        int result = join_exam_room(room_id, client->username, client);
        if (result == 0) {
            send(client->fd, "Joined room successfully\n", strlen("Joined room successfully\n"), 0);
        } else {
            send(client->fd, "Failed to join room\n", strlen("Failed to join room\n"), 0);
        }
        return;
    }

    // Rời room
    if (strcmp(buffer, "LEAVE_ROOM") == 0) {
        if (client->current_room_id != -1) {
            leave_exam_room(client->current_room_id, client->username, client);
            send(client->fd, "Left room successfully\n", strlen("Left room successfully\n"), 0);
        } else {
            send(client->fd, "You are not in any room\n", strlen("You are not in any room\n"), 0);
        }
        return;
    }

    // Xóa room
    if (strcmp(buffer, "DELETE_ROOM") == 0) {
        if (client->current_room_id != -1) {
            if (is_room_creator(client->current_room_id, client->username)) {
                int room_to_delete = client->current_room_id;
                
                // Thông báo cho các user khác trong room
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    ClientInfo* other = &server->clients[i];
                    if (other->active && other->current_room_id == room_to_delete && other->fd != client->fd) {
                        send(other->fd, "Room has been deleted by creator\n", 
                             strlen("Room has been deleted by creator\n"), 0);
                        other->current_room_id = -1;
                    }
                }
                
                delete_exam_room(room_to_delete, server->clients);
                client->current_room_id = -1;
                send(client->fd, "ROOM_DELETED\n", strlen("ROOM_DELETED\n"), 0);
            } else {
                send(client->fd, "Only room creator can delete room\n", 
                     strlen("Only room creator can delete room\n"), 0);
            }
        } else {
            send(client->fd, "You are not in any room\n", strlen("You are not in any room\n"), 0);
        }
        return;
    }

    // ===== OTHER FUNCTIONS =====
    // Đăng xuất
    if (strcmp(buffer, "LOGOUT") == 0) {
        if (client->current_room_id != -1) {
            leave_exam_room(client->current_room_id, client->username, client);
        }
        
        memset(client->username, 0, MAX_USERNAME);
        memset(client->session_id, 0, sizeof(client->session_id));
        client->authenticated = 0;
        client->session_start = 0;
        client->current_room_id = -1;
        client->current_question = -1;
        client->score = 0;

        send(client->fd, "Logged out successfully\n", strlen("Logged out successfully\n"), 0);
        printf("User logged out: fd=%d\n", client->fd);
        return;
    }

    // Nhận môn học 
    if (strcmp(buffer, "GET_SUBJECTS") == 0) {
        char subjects_list[BUFFER_SIZE - 10];  // Để lại space cho "SUBJECTS|" và "\n"
        get_available_subjects(subjects_list, sizeof(subjects_list));
    
        char response[BUFFER_SIZE];
        snprintf(response, BUFFER_SIZE, "SUBJECTS|%s\n", subjects_list);
        send(client->fd, response, strlen(response), 0);
    }
}

