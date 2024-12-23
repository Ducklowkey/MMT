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
    printf("3. Training Mode\n");  
    printf("4. Add New Question\n");
    printf("5. Logout\n");
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

void print_training_menu(void) {
    printf("\n=== Training Mode ===\n");
    printf("1. Start Training\n");
    printf("2. Back to Main Menu\n");
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
            
            case 3: // Training Mode
            handle_training_menu(client);
                break;

            case 4: // Add question
                handle_add_question(client);
                break;

            case 5: // Logout
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

void handle_add_question(Client* client) {
    char subject[50];
    int difficulty;
    char question[200];
    char option_a[100];
    char option_b[100];
    char option_c[100];
    char option_d[100];
    char correct_answer;
    char buffer[BUFFER_SIZE];
    char command[BUFFER_SIZE];

    clear_screen();
    print_add_question_menu();

    // Nhập thông tin câu hỏi như cũ...
    printf("Subject (e.g., Math, Geography, History, Literature): ");
    fgets(subject, sizeof(subject), stdin);
    subject[strcspn(subject, "\n")] = 0;

    do {
        printf("Difficulty (1-3): ");
        fgets(buffer, sizeof(buffer), stdin);
    } while (sscanf(buffer, "%d", &difficulty) != 1 || difficulty < 1 || difficulty > 3);

    printf("Question: ");
    fgets(question, sizeof(question), stdin);
    question[strcspn(question, "\n")] = 0;

    printf("Option A: ");
    fgets(option_a, sizeof(option_a), stdin);
    option_a[strcspn(option_a, "\n")] = 0;

    printf("Option B: ");
    fgets(option_b, sizeof(option_b), stdin);
    option_b[strcspn(option_b, "\n")] = 0;

    printf("Option C: ");
    fgets(option_c, sizeof(option_c), stdin);
    option_c[strcspn(option_c, "\n")] = 0;

    printf("Option D: ");
    fgets(option_d, sizeof(option_d), stdin);
    option_d[strcspn(option_d, "\n")] = 0;

    do {
        printf("Correct answer (A/B/C/D): ");
        fgets(buffer, sizeof(buffer), stdin);
        correct_answer = toupper(buffer[0]);
    } while (correct_answer != 'A' && correct_answer != 'B' && 
             correct_answer != 'C' && correct_answer != 'D');

    // Tạo command để gửi đến server
    snprintf(command, BUFFER_SIZE, "ADD_QUESTION %s|%d|%s|%s|%s|%s|%s|%c",
             subject, difficulty, question, option_a, option_b, option_c, option_d, correct_answer);

    // Gửi đến server
    if (send_message(client, command) > 0) {
        // Nhận phản hồi từ server
        if (receive_message(client, buffer) > 0) {
            if (strstr(buffer, "success") != NULL) {
                print_success("Question added successfully!");
            } else {
                print_error(buffer);
            }
        }
    } else {
        print_error("Failed to send command to server");
    }
}

void print_add_question_menu(void) {
    printf("\n=== Add New Question ===\n");
    printf("Please enter the following information:\n");
}

void handle_training_setup(Client* client) {
    char buffer[BUFFER_SIZE];
    int num_questions, time_limit, difficulty;
    char subject[50];

    printf("\n=== Training Setup ===\n");
    
    // Get number of questions
    do {
        printf("Enter number of questions (1-20): ");
    } while (scanf("%d", &num_questions) != 1 || num_questions < 1 || num_questions > 20);
    while(getchar() != '\n');

    // Get time limit
    do {
        printf("Enter time limit in minutes (5-60): ");
    } while (scanf("%d", &time_limit) != 1 || time_limit < 5 || time_limit > 60);
    while(getchar() != '\n');

    // Get subject
    printf("Available subjects: Math, Geography, History, Literature\n");
    printf("Enter subject: ");
    fgets(subject, sizeof(subject), stdin);
    subject[strcspn(subject, "\n")] = 0;

    // Get difficulty
    do {
        printf("Enter difficulty (1-3): ");
    } while (scanf("%d", &difficulty) != 1 || difficulty < 1 || difficulty > 3);
    while(getchar() != '\n');

    // Send training request
    char cmd[BUFFER_SIZE];
    snprintf(cmd, BUFFER_SIZE, "START_TRAINING|%d|%d|%d|%s", 
             num_questions, time_limit, difficulty, subject);
    
    printf("Sending training request: %s\n", cmd);
    if (send_message(client, cmd) < 0) {
        print_error("Failed to start training");
        return;
    }

    // Nhận phản hồi từ server
    if (receive_message(client, buffer) > 0) {
        if (strstr(buffer, "Training ready") != NULL) {
            // Start training session
            handle_exam(client);  // Reuse exam handler

            // After training completed, ask for continuation
            printf("\nDo you want to continue training? (Y/N): ");
            char choice;
            scanf(" %c", &choice);
            while(getchar() != '\n');

            if (toupper(choice) == 'Y') {
                handle_training_setup(client);
            }
        } else {
            print_error(buffer);  // In lỗi nếu server trả về lỗi
        }
    } else {
        print_error("Lost connection to server");
    }
}

void handle_training_menu(Client* client) {
    int choice;
    while (1) {
        print_training_menu();
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            print_error("Invalid input");
            continue;
        }
        while (getchar() != '\n');

        switch (choice) {
            case 1: // Start Training
                handle_training_setup(client);
                break;

            case 2: // Back to Main Menu
                return;

            default:
                print_error("Invalid option");
        }
    }
}
