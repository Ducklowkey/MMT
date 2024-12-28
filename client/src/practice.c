#include "../include/practice.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>  
#include <arpa/inet.h>  

// Hàm cho phép người dùng lựa chọn thông số bài luyện tập
void configure_practice(Client* client, int* num_questions_total, int* time_limit, int* num_easy, int* num_medium, int* num_hard, char* subjects) {
    printf("Chọn môn học:\n");
    print_subjects_menu(client);  // Lấy danh sách môn học từ server
    
    char buffer[BUFFER_SIZE];
    send_message(client, "GET_SUBJECTS");
    memset(buffer, 0, BUFFER_SIZE);
    
    if (receive_message(client, buffer) <= 0 || strncmp(buffer, "SUBJECTS|", 9) != 0) {
        printf("Không thể lấy danh sách môn học từ server\n");
        return;
    }

    // Phân tích danh sách môn học
    char* subjects_list = buffer + 9; // Bỏ qua "SUBJECTS|"
    char* subject_names[MAX_QUESTIONS];
    int num_subjects = 0;

    // Tách các môn học
    char* subject = strtok(subjects_list, ",");
    while (subject != NULL && num_subjects < MAX_QUESTIONS) {
        subject_names[num_subjects++] = strdup(subject);
        subject = strtok(NULL, ",");
    }

    // In menu môn học
    printf("\nChọn môn học:\n");
    for (int i = 0; i < num_subjects; i++) {
        printf("%d. %s\n", i + 1, subject_names[i]);
    }

    // Chọn môn học
    int selected_subjects[MAX_QUESTIONS] = {0};
    int subject_choice;
    printf("\nNhập số thứ tự môn học (0 để kết thúc):\n");
    while (1) {
        printf("Số thứ tự: ");
        scanf("%d", &subject_choice);

        if (subject_choice == 0) {
            break;
        } else if (subject_choice >= 1 && subject_choice <= num_subjects) {
            selected_subjects[subject_choice - 1] = 1;
        } else {
            printf("Lựa chọn không hợp lệ. Vui lòng thử lại.\n");
        }
    }

    // Tạo chuỗi môn học đã chọn
    subjects[0] = '\0';
    for (int i = 0; i < num_subjects; i++) {
        if (selected_subjects[i]) {
            if (strlen(subjects) > 0) {
                strcat(subjects, ",");
            }
            strcat(subjects, subject_names[i]);
        }
    }

    // Giải phóng bộ nhớ
    for (int i = 0; i < num_subjects; i++) {
        free(subject_names[i]);
    }

    printf("\nChọn tổng số câu hỏi:\n");
    printf("1. 15\n2. 30\n3. 45\n4. 60\n");
    int question_choice;
    do {
        printf("Nhập lựa chọn: ");
        scanf("%d", &question_choice);
        switch (question_choice) {
            case 1: *num_questions_total = 15; break;
            case 2: *num_questions_total = 30; break;
            case 3: *num_questions_total = 45; break;
            case 4: *num_questions_total = 60; break;
            default: printf("Lựa chọn không hợp lệ. Vui lòng thử lại.\n");
        }
    } while (question_choice < 1 || question_choice > 4);

    printf("Select difficulty ratio (Easy:Medium:Hard):\n");
    printf("1. 3:0:0\n2. 0:3:0\n3. 0:0:3\n4. 1:2:0\n5. 2:1:0\n6. 1:0:2\n7. 2:0:1\n8. 0:1:2\n9. 0:2:1\n10. 1:1:1\n");
    int ratio_choice;
    do {
        printf("Enter your choice: ");
        scanf("%d", &ratio_choice);
        switch (ratio_choice) {
            case 1: *num_easy = *num_questions_total; *num_medium = 0; *num_hard = 0; break;
            case 2: *num_easy = 0; *num_medium = *num_questions_total; *num_hard = 0; break;
            case 3: *num_easy = 0; *num_medium = 0; *num_hard = *num_questions_total; break;
            case 4: *num_easy = *num_questions_total / 3; *num_medium = *num_questions_total * 2 / 3; *num_hard = 0; break;
            case 5: *num_easy = *num_questions_total * 2 / 3; *num_medium = *num_questions_total / 3; *num_hard = 0; break;
            case 6: *num_easy = *num_questions_total / 3; *num_medium = 0; *num_hard = *num_questions_total * 2 / 3; break;
            case 7: *num_easy = *num_questions_total * 2 / 3; *num_medium = 0; *num_hard = *num_questions_total / 3; break;
            case 8: *num_easy = 0; *num_medium = *num_questions_total / 3; *num_hard = *num_questions_total * 2 / 3; break;
            case 9: *num_easy = 0; *num_medium = *num_questions_total * 2 / 3; *num_hard = *num_questions_total / 3; break;
            case 10: *num_easy = *num_questions_total / 3; *num_medium = *num_questions_total / 3; *num_hard = *num_questions_total / 3; break;
            default: printf("Invalid choice. Try again.\n");
        }
    } while (ratio_choice < 1 || ratio_choice > 10);

    printf("Enter time limit (in minutes): ");
    scanf("%d", time_limit);
}

// Hàm gửi thông số bài luyện tập đến server
void send_practice_config(Client* client, int num_questions_total, int time_limit, int num_easy, int num_medium, int num_hard, char* subjects) {
    // Chuẩn bị thông điệp gửi đi
    char buffer[1024];
 
    snprintf(buffer, sizeof(buffer),"START_PRACTICE %d,%d,%d,%d,%d,%s\n",
            num_questions_total, time_limit, num_easy, num_medium, num_hard, subjects); // Chuyển thời gian từ phút sang giây
    //printf("Practice config: %s\n", buffer);
    if (write(client->socket, buffer, strlen(buffer)) > 0) {
        printf("Practice config sent to server: %s\n", buffer);
    } else {
        printf("Failed to send practice config.\n");
    }
    
}
void request_time_left(Client* client) {
    char time_request[] = "TIME";
    send(client->socket, time_request, strlen(time_request), 0);
}

// Hàm send thông điệp START_PRACTICE VÀ FORMAT bài luyện tập cho server
void start_and_set_format(Client* client) {
    char buffer[1024];
    char subjects[256];
    int num_questions_total = 0, num_easy = 0, num_medium = 0, num_hard = 0, time_limit = 0;

    // Gọi hàm configure_practice với tham số client
    configure_practice(client, &num_questions_total, &time_limit, &num_easy, &num_medium, &num_hard, subjects);
    send_practice_config(client, num_questions_total, time_limit, num_easy, num_medium, num_hard, subjects);
}

void handle_practice(Client* client) {
    char buffer[1024];
    printf("Enter your answer (A/B/C/D or type 'SUBMIT' to quit or type 'TIME' to request time left): \n");
    while (1) {
        // Nhận câu hỏi từ server
        memset(buffer, 0, sizeof(buffer)); // Xóa nội dung cũ của buffer
        int bytes_received = read(client->socket, buffer, sizeof(buffer) - 1);
        if (bytes_received <= 0) {
            printf("Disconnected from server.\n");
            break;
        }
        if( buffer[bytes_received] != '\0')
            buffer[bytes_received] = 0; // Đảm bảo buffer là chuỗi kết thúc null
        // Client kiểm tra tín hiệu TIMEOUT
        if (strncmp(buffer, "TIMEOUT:", 8) == 0) {
            printf("%s\n", buffer);  // In thông báo hết thời gian và điểm số
            return;
        }
        if (strncmp(buffer, "SCORE:", 6) == 0) {
            printf("%s\n", buffer);
            return;
        }
        if (strncmp(buffer, "ERROR_FORMAT", 12) == 0) {
            printf("%s\n", buffer);
            //start_and_set_format(client);
            return;
        }
        printf("%s\n", buffer);

        char answer[16];
        while (1) {
           // printf("Enter your answer (A/B/C/D or type 'SUBMIT' to quit or type 'TIME' to request time left): ");
           //printf("%s1\n", buffer);
            scanf("%s", answer);
            if (strcmp(answer, "SUBMIT") == 0 || strcmp(answer, "TIME") == 0 || strcmp(answer,"CHANGE_ANSWER")|| (strlen(answer) == 1 &&
                (answer[0] == 'A' || answer[0] == 'B' || answer[0] == 'C' || answer[0] == 'D'))) {
                break;
            }
            else printf("Enter your answer (A/B/C/D or type 'SUBMIT' to quit or type 'TIME' to request time left): ");
        }
        if (strcmp(answer, "SUBMIT") == 0) {
                submit_practice_early(client);
                printf("%s\n", buffer);
                break;
        }
        if (strcmp(answer, "TIME") == 0) {
                request_time_left(client);
               // printf("ÁDASD%s\n", buffer);
        }
        if(strcmp(answer,"CHANGE_ANSWER") == 0){
            printf("Enter your Question_number and new answer (A/B/C/D): ");
            int question_number;
            scanf("%d",&question_number);
            //scanf("%s",answer);
            while (1) {
            scanf("%s", answer);
            if ((strlen(answer) == 1 &&(answer[0] == 'A' || answer[0] == 'B' || answer[0] == 'C' || answer[0] == 'D'))) {
                break;
            }
            else printf("Enter your  new answer (A/B/C/D): ");
        }
        }
        submit_answer_practice(client, answer);
    }
}
// Hàm gửi câu trả lời thực hành
void submit_answer_practice(Client* client, const char* answer) {
    // Kiểm tra tính hợp lệ của client và socket
    if (client == NULL || answer == NULL) {
        fprintf(stderr, "Client hoặc câu trả lời không hợp lệ.\n");
        return;
    }
    const char* command = "SUBMIT_PRACTICE_ANSWER"; // Lệnh gửi câu trả lời
    char message[256]; // Kích thước đủ lớn để chứa thông điệp

    // Tạo thông điệp gửi
    snprintf(message, sizeof(message), "%s %s\n", command, answer);

    // Gửi thông điệp qua socket
    if (write(client->socket, message, strlen(message)) > 0) {
        printf("Submit answer practice success.\n");
    } else {
        perror("Submit answer practice failed"); // In lỗi chi tiết
    }
}

// Hàm gửi tín hiệu kết thúc chế độ thực hành sớm
void submit_practice_early(Client* client) {
    const char* message = "SUBMIT";
    if (write(client->socket, message, strlen(message)) > 0) { // Sửa sockfd thành socket nếu cần
        printf("Submit success.\n");
    } else {
        printf("Submit failed .\n");
    }
}

void print_subjects_menu(Client* client) {  
    char subjects_list[BUFFER_SIZE];
    char command[] = "GET_SUBJECTS";
    
    if (send_message(client, command) < 0) {
        printf("Lỗi khi gửi yêu cầu lấy danh sách môn học\n");
        return;
    }

    // Nhận danh sách môn học
    char buffer[BUFFER_SIZE];
    if (receive_message(client, buffer) > 0) {
        if (strncmp(buffer, "SUBJECTS|", 9) == 0) {
            char* subjects_str = buffer + 9;
            char* subject;
            int index = 1;
            
            printf("Chọn môn học:\n");
            // Tách và in từng môn học
            subject = strtok(subjects_str, ",");
            while (subject != NULL) {
                printf("%d. %s\n", index++, subject);
                subject = strtok(NULL, ",");
            }
        }
    }
}