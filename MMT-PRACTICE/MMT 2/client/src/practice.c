#include "../include/practice.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h> // Để dùng read và write
#include <sys/socket.h>  // Dành cho Linux/Mac
#include <arpa/inet.h>   // Dành cho Linux/Mac

// Hàm cho phép người dùng lựa chọn thông số bài luyện tập
void configure_practice(int* num_questions_total, int* time_limit, int* num_easy, int* num_medium, int* num_hard, char* subjects) {
    
    printf("Select subjects:\n");
    printf("1. Math\n2. Geography\n3. History\n4. Literature\n5. English\n6. Physics\n7. Chemistry\n8. Biology\n9. All\n");

    int selected_subjects[9] = {0};
    int subject_choice;
    printf("Enter subject numbers (0 to finish selection):\n");
    while (1) {
        printf("Subject number: ");
        scanf("%d", &subject_choice);

        if (subject_choice == 0) {
            break;
        } else if (subject_choice >= 1 && subject_choice <= 9) {
            selected_subjects[subject_choice - 1] = 1;
        } else {
            printf("Invalid choice. Try again.\n");
        }
    }

    subjects[0] = '\0';
    for (int i = 0; i < 9; i++) {
        if (selected_subjects[i]) {
            switch (i + 1) {
                case 1: strcat(subjects, "Math,"); break;
                case 2: strcat(subjects, "Geography,"); break;
                case 3: strcat(subjects, "History,"); break;
                case 4: strcat(subjects, "Literature,"); break;
                case 5: strcat(subjects, "English,"); break;
                case 6: strcat(subjects, "Physics,"); break;
                case 7: strcat(subjects, "Chemistry,"); break;
                case 8: strcat(subjects, "Biology,"); break;
                case 9: strcat(subjects, "All,"); break;
            }
        }
    }

    if (strlen(subjects) > 0) {
        subjects[strlen(subjects) - 1] = '\0'; // Remove trailing comma
    }

    printf("Select total number of questions:\n");
    printf("1. 15\n2. 30\n3. 45\n4. 60\n");
    int question_choice;
    do {
        printf("Enter your choice: ");
        scanf("%d", &question_choice);
        switch (question_choice) {
            case 1: *num_questions_total = 15; break;
            case 2: *num_questions_total = 30; break;
            case 3: *num_questions_total = 45; break;
            case 4: *num_questions_total = 60; break;
            default: printf("Invalid choice. Try again.\n");
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
 
    snprintf(buffer, sizeof(buffer), "-SET_FORMAT: %d,%d,%d,%d,%d,%s\n",
            num_questions_total, time_limit, num_easy, num_medium, num_hard, subjects); // Chuyển thời gian từ phút sang giây
    //printf("Practice config: %s\n", buffer);
    if (write(client->socket, buffer, strlen(buffer)) > 0) {
        printf("Practice config sent to server: %s\n", buffer);
    } else {
        printf("Failed to send practice config.\n");
    }
}
int flag_time = 0;
void request_time_left(Client* client) {
    char time_request[] = "TIME";
    send(client->socket, time_request, strlen(time_request), 0);

    char buffer[1024];
    int bytes_received = recv(client->socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        if (strncmp(buffer, "TIMEOUT:", 8) == 0) {
                    printf("%s\n", buffer);  // In thông báo hết thời gian và điểm số
                    flag_time=1;
        }
        else printf("%s", buffer); // In thời gian còn lại
    }
}

// Hàm xử lý chế độ thực hành
void handle_practice(Client* client) {
    char buffer[1024];
    char subjects[256];
    int num_questions_total = 0, num_easy = 0, num_medium = 0, num_hard = 0, time_limit = 0;

    // Gọi hàm cấu hình bài luyện tập
    configure_practice(&num_questions_total, &time_limit, &num_easy, &num_medium, &num_hard, subjects);
    send_practice_config(client, num_questions_total, time_limit, num_easy, num_medium, num_hard, subjects);
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
        printf("%s\n", buffer);

        char answer[16];
        while (1) {
           // printf("Enter your answer (A/B/C/D or type 'SUBMIT' to quit or type 'TIME' to request time left): ");
           //printf("%s1\n", buffer);
            scanf("%s", answer);
            if (strcmp(answer, "SUBMIT") == 0) {
                submit_practice_early(client);
                printf("%s\n", buffer);
                break;
            }
            if(strcmp(answer, "TIME") == 0){
                request_time_left(client);
                if(flag_time==1) return;
                continue;
            }
            if (strlen(answer) == 1 && 
                (answer[0] == 'A' || answer[0] == 'B' || answer[0] == 'C' || answer[0] == 'D')) {
                break;
            }
            else printf("Enter your answer (A/B/C/D or type 'SUBMIT' to quit or type 'TIME' to request time left): ");
        }
        if (strcmp(answer, "SUBMIT") == 0) {
                break;
        }
        // Gửi câu trả lời tới server
        submit_answer_practice(client, answer[0]);
    }
}
// Hàm gửi câu trả lời thực hành
void submit_answer_practice(Client* client, char answer) {
    char buffer[2];
    buffer[0] = answer;
    buffer[1] = '\0';

    if (write(client->socket, buffer, sizeof(buffer)) > 0) { // Sửa sockfd thành socket nếu cần
        printf("Answer submitted: %c\n", answer);
    } else {
        printf("Failed to submit answer.\n");
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