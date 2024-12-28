// client/src/ui.c
#include "../include/ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void print_banner(void) {
    printf("\n===================================\n");
    printf("Welcome to the Online Exam System\n");
    printf("===================================\n\n");
}

void print_main_menu(void) {
    printf("\n=== Main Menu ===\n");
    printf("1. Create Exam Room\n");
    printf("2. Join Exam Room\n");  
    printf("3. Logout\n");
}

void print_room_menu(int is_creator) {
    printf("\n=== Exam Room Menu ===\n");
    if (is_creator) {
        printf("1. Start Exam\n");
        printf("2. Leave Room\n");
        printf("3. Delete Room\n");
    } else {
        printf("1. Leave Room\n");
        printf("\nWaiting for exam to start...\n");
    }
    printf("\nChoose an option: ");
}

void clear_screen(void) {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void print_error(const char* message) {
    printf("\033[1;31mError: %s\033[0m\n", message);
}

void print_success(const char* message) {
    printf("\033[1;32m%s\033[0m\n", message);
}

void handle_room_menu(Client* client) {
    char buffer[BUFFER_SIZE];
    fd_set readfds;
    int max_fd = client->socket;

    while (1) {
        print_room_menu(client->is_room_creator);
        
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(client->socket, &readfds);

        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            print_error("Select error");
            break;
        }

        // Check for server messages
        if (FD_ISSET(client->socket, &readfds)) {
            int valread = receive_message(client, buffer);
            if (valread <= 0) {
                print_error("Server disconnected");
                return;
            }

            // Nếu nhận được thông báo bắt đầu thi và không phải chủ room
            if (strstr(buffer, "Exam has started") != NULL && !client->is_room_creator) {
                printf("\n%s", buffer);
                handle_exam(client);  // Chuyển sang màn hình thi
                return;
            }
            
            printf("\n%s", buffer);
        }

        // Check for user input only if no message from server
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            int choice;
            if (scanf("%d", &choice) != 1) {
                while (getchar() != '\n');
                print_error("Invalid input");
                continue;
            }
            while (getchar() != '\n');

            if (client->is_room_creator) {
                switch (choice) {
                    case 1: // Start Exam
                        if (send_message(client, "START_EXAM") >= 0) {
                            // Chủ room không cần thêm xử lý, chỉ đợi thông báo từ server
                        }
                        break;

                    case 2: // Leave Room
                        if (send_message(client, "LEAVE_ROOM") >= 0) {
                            receive_message(client, buffer);
                            print_success("Left room successfully");
                            return;
                        }
                        break;

                    case 3: // Delete Room
                        if (send_message(client, "DELETE_ROOM") >= 0) {
                            receive_message(client, buffer);
                            print_success("Room deleted successfully");
                            return;
                        }
                        break;

                    default:
                        print_error("Invalid option");
                        break;
                }
            } else {
                if (choice == 1) { // Leave Room
                    if (send_message(client, "LEAVE_ROOM") >= 0) {
                        receive_message(client, buffer);
                        print_success("Left room successfully");
                        return;
                    }
                } else {
                    print_error("Invalid option");
                }
            }
        }
    }
}

void handle_main_menu(Client* client) {
    char buffer[BUFFER_SIZE];
    int choice;

    while (1) {
        print_main_menu();

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            print_error("Invalid input");
            continue;
        }
        while (getchar() != '\n');

        switch (choice) {
            case 1: { // Create Exam Room
                printf("Enter room name: ");
                char room_name[BUFFER_SIZE];
                char cmd[BUFFER_SIZE];
                
                if (fgets(room_name, BUFFER_SIZE, stdin)) {
                    room_name[strcspn(room_name, "\n")] = 0;
                    
                    if (strlen(room_name) < 1) {
                        print_error("Room name cannot be empty");
                        continue;
                    }

                    memset(cmd, 0, BUFFER_SIZE);
                    if (snprintf(cmd, BUFFER_SIZE, "CREATE_ROOM %s", room_name) >= BUFFER_SIZE) {
                        print_error("Room name too long");
                        continue;
                    }

                    printf("Sending message: %s\n", cmd);

                    if (send_message(client, cmd) < 0) {
                        print_error("Failed to send command to server");
                        continue;
                    }

                    char response[BUFFER_SIZE];
                    memset(response, 0, BUFFER_SIZE);
                    int received = receive_message(client, response);
                    if (received < 0) {
                        print_error("Failed to receive server response");
                        continue;
                    }

                    // Kiểm tra response bằng prefix rõ ràng
                    if (strncmp(response, "ROOM_CREATED", 11) == 0) {
                        int room_id;
                        if (sscanf(response + 12, "%d", &room_id) == 1) {
                            client->current_room = room_id;
                            client->is_room_creator = 1;
                            print_success("Room created successfully");
                            handle_room_menu(client);
                        }
                    } else {
                        print_error(response);
                    }
                }
                break;
            }

            case 2: { // Join Exam Room
                if (send_message(client, "LIST_ROOMS") >= 0) {
                    if (receive_message(client, buffer) >= 0) {
                        printf("\nAvailable Rooms:\n%s", buffer);

                        printf("Enter room ID to join (0 to cancel): ");
                        int room_id;
                        if (scanf("%d", &room_id) == 1) {
                            while (getchar() != '\n');
                            
                            if (room_id == 0) {
                                continue;
                            }

                            char cmd[BUFFER_SIZE];
                            snprintf(cmd, BUFFER_SIZE, "JOIN_ROOM %d", room_id);

                            if (send_message(client, cmd) >= 0) {
                                if (receive_message(client, buffer) >= 0) {
                                    if (strstr(buffer, "successfully") != NULL) {
                                        client->current_room = room_id;
                                        client->is_room_creator = 0;
                                        print_success("Joined room successfully");
                                        handle_room_menu(client);
                                    } else {
                                        print_error(buffer);
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            }
            
            case 3: // 
                if (send_message(client, "LOGOUT") >= 0) {
                    receive_message(client, buffer); // Đợi phản hồi từ server
                    print_success("Logged out successfully");
                    return;
                }
                break;

            default:
                print_error("Invalid option");
                break;
        }
    }
}


