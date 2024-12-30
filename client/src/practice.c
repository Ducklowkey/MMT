#include "../include/practice.h"
#include "../include/exam.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>  
#include <arpa/inet.h>  


// Hàm gửi thông số bài luyện tập đến server
void send_practice_config(Client* client, int num_questions_total, int time_limit, int num_easy, int num_medium, int num_hard, char* subjects) {
    // Chuẩn bị thông điệp gửi đi
    char buffer[1024];
 
    snprintf(buffer, sizeof(buffer),"START_PRACTICE %d,%d,%d,%d,%d,%s\n",
            num_questions_total, time_limit, num_easy, num_medium, num_hard, subjects); // Chuyển thời gian từ phút sang giây
    //printf("Practice config: %s\n", buffer);
    if (write(client->socket, buffer, strlen(buffer)) > 0) {
        printf("Đã gửi yêu cầu đến server");
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

    // set format 
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

