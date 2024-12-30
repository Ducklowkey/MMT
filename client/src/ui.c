// client/src/ui.c
#include "../include/ui.h"
#include "../include/practice.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void print_banner(void) {
    printf("\n\033[1;34m=========================================\033[0m\n");
    printf("Chào mừng đến với hệ thống thi trực tuyến\n");
    printf("\033[1;34m=========================================\033[0m\n\n");
}

void print_main_menu(void) {
    printf("\n\033[1;34m===Main Menu===\033[0m\n");
    printf("1. Create Exam Room\n");
    printf("2. Join Exam Room\n");  
    printf("3. Practice Mode\n");
    printf("4 Logout\n");
}

void print_practice_room_menu(void) {
    printf("\n\033[1;34m===Practice Mode Menu===\033[0m\n");
    printf("1. Start Practice\n");      
    printf("2. Leave Practice Mode\n");      
}

void print_room_menu(int is_creator, int exam_completed) {
    printf("\n\033[1;34m===Exam Room===\033[0m\n");
    if (is_creator) {
        if (!exam_completed) {
            printf("1. Start Exam\n");
        }
        printf("2. Leave Room\n");
        printf("3. Delete Room\n");
    } else {
        printf("1. Leave Room\n");
        if (!exam_completed) {
            printf("Luật : gõ SUBMIT để nộp bài sớm , TIME để biết còn lại bao nhiêu thời gian \n");
            printf("\nChờ cho chủ phòng bắt đầu bài thi...\n");
        }
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

void handle_practice_menu(Client* client) {
    while (1) {
        print_practice_room_menu();
        char input[10];
        int choice;

        // Đọc lựa chọn từ người dùng
        printf("Nhập lựa chọn (1-2): ");
        if (!fgets(input, sizeof(input), stdin)) {
            print_error("Lỗi khi đọc input");
            continue;
        }

        // Xóa newline
        input[strcspn(input, "\n")] = 0;

        // Kiểm tra input hợp lệ
        if (strlen(input) != 1 || input[0] < '1' || input[0] > '2') {
            print_error("Lựa chọn không hợp lệ. Vui lòng chọn 1 hoặc 2");
            continue;
        }

        choice = input[0] - '0';

        switch (choice) {
            case 1: {  // Bắt đầu luyện tập
                char buffer[BUFFER_SIZE];
                char subjects[256];
                int num_questions_total = 0, time_limit = 0;
                int num_easy = 0, num_medium = 0, num_hard = 0;

                // Cấu hình bài tập
                configure_practice(client, &num_questions_total, &time_limit,
                                &num_easy, &num_medium, &num_hard, subjects);
                
                // Kiểm tra nếu người dùng hủy cấu hình (nhập 0)
                if (strlen(subjects) == 0) {
                    printf("\nĐã hủy cấu hình bài tập.\n");
                    continue;
                }

                // Gửi cấu hình đến server
                char config_cmd[BUFFER_SIZE];
                snprintf(config_cmd, sizeof(config_cmd), 
                        "START_PRACTICE %d,%d,%d,%d,%d,%s",
                        num_questions_total, time_limit, 
                        num_easy, num_medium, num_hard, subjects);

                if (send_message(client, config_cmd) < 0) {
                    print_error("Không thể gửi cấu hình đến server");
                    continue;
                }

                // Nhận phản hồi từ server
                if (receive_message(client, buffer) <= 0) {
                    print_error("Mất kết nối với server");
                    return;
                }

                if (strcmp(buffer, "PRACTICE_ACCEPT\n") == 0) {
                    printf("\nBắt đầu bài tập...\n");
                    handle_practice(client);
                } else {
                    print_error("Server từ chối bài tập");
                }
                break;
            }

            case 2: {  // Thoát chế độ luyện tập
                if (send_message(client, "LEAVE_PRACTICE") >= 0) {
                    print_success("Đã thoát chế độ luyện tập");
                    return;
                } else {
                    print_error("Không thể thoát chế độ luyện tập");
                }
                break;
            }
        }
    }
}

void handle_room_menu(Client* client) {
    char buffer[BUFFER_SIZE];
    fd_set readfds;
    int max_fd = client->socket;
    int exam_completed = 0;  // Thêm flag kiểm tra đã thi xong

    while (1) {
        print_room_menu(client->is_room_creator, exam_completed);  
        
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(client->socket, &readfds);

        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            print_error("Select error");
            break;
        }

        // Kiểm tra server res 
        if (FD_ISSET(client->socket, &readfds)) {
            int valread = receive_message(client, buffer);
            if (valread <= 0) {
                print_error("Server disconnected");
                return;
            }

            if (strstr(buffer, "Exam has started") != NULL && !client->is_room_creator) {
                printf("\n%s", buffer);
                handle_exam(client);  // Chuyển sang màn hình thi
                exam_completed = 1;   
                continue;            
            }

            if (strstr(buffer, "Exam completed") != NULL) {
                printf("\n%s", buffer);
                exam_completed = 1;  
                continue;           
            }
            
            printf("\n%s", buffer);
        }

        // Kiểm tra client input 
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
                        if (!exam_completed) {
                            char subjects[256];
                            int num_questions_total = 0, time_limit = 0;
                            int num_easy = 0, num_medium = 0, num_hard = 0;

                            //  cấu hình bài thi 
                            configure_practice(client, &num_questions_total, &time_limit, &num_easy, &num_medium, &num_hard, subjects);

                            if (strlen(subjects) == 0) {
                                printf("Đã hủy cấu hình bài thi.\n");
                                continue;
                            }
                            //client req
                            char config_cmd[BUFFER_SIZE];
                            snprintf(config_cmd, BUFFER_SIZE, "SET_EXAM_FORMAT %d,%d,%d,%d,%d,%s", num_questions_total, time_limit, num_easy, num_medium, num_hard, subjects);
                            if (send_message(client, config_cmd) < 0) {
                                print_error("Không thể gửi cấu hình bài thi");
                                continue;
                            }

                            //server res
                            char buffer[BUFFER_SIZE];
                            if (receive_message(client, buffer) > 0 && 
                                strstr(buffer, "FORMAT_ACCEPTED") != NULL) {
                                // Bắt đầu thi
                                if (send_message(client, "START_EXAM") >= 0) {
                                    print_success("Bài thi đã bắt đầu");
                                }
                            } else {
                            print_error("Cấu hình bài thi không hợp lệ");
                            }
                        } else {
                            print_error("Bài thi đã kết thúc");
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
            case 1: { // Tạo phòng thi 
                printf("Enter room name: ");
                char room_name[BUFFER_SIZE];
                char cmd[BUFFER_SIZE];
                
                if (fgets(room_name, BUFFER_SIZE, stdin)) {
                    room_name[strcspn(room_name, "\n")] = 0;
                    
                    if (strlen(room_name) < 1) {
                        print_error("Không để trống tên phòng");
                        continue;
                    }

                    memset(cmd, 0, BUFFER_SIZE);
                    if (snprintf(cmd, BUFFER_SIZE, "CREATE_ROOM %s", room_name) >= MAX_ROOMNAME) {
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

            case 3: {  // Chế độ luyện tập
                print_success("Starting practice mode...");
                handle_practice_menu(client);
                
                break;
            }
            
            case 4: // 
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


