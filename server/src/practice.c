#include "../include/exam.h"
#include "../include/room.h"
#include "../include/practice.h"
#include "../include/server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


ClientDataPractice* create_client_data_practice(int socket, int num_questions, int time_limit, int num_easy, int num_medium, int num_hard, const char* subjects) {
    // Kiểm tra số lượng câu hỏi không vượt quá giới hạn
    if (num_questions > MAX_QUESTIONS) {
        fprintf(stderr, "Số lượng câu hỏi vượt quá giới hạn.\n");
        return NULL;
    }

    // Cấp phát bộ nhớ cho ClientDataPractice
    ClientDataPractice* client = (ClientDataPractice*)malloc(sizeof(ClientDataPractice));
    if (client == NULL) {
        perror("Không thể cấp phát bộ nhớ");
        return NULL;
    }

    // Khởi tạo các giá trị trong cấu trúc
    client->socket = socket;
    client->current_question = 0; // Câu hỏi bắt đầu từ 1
    client->num_questions = num_questions;
    client->time_limit = time_limit*60; // Đổi phút thành giây
    client->num_easy = num_easy;
    client->num_medium = num_medium;
    client->num_hard = num_hard;
    client->start_time = time(NULL); // Gán thời gian bắt đầu là thời điểm hiện tại
    client->score = 0; // Điểm ban đầu là 0
    memset(client->subjects_practice, 0, sizeof(client->subjects_practice));

    // Reset answers_practice
    memset(client->answers_practice, 0, sizeof(client->answers_practice));

    // Reset questions_practice
    memset(client->questions_practice, 0, sizeof(client->questions_practice));
    //printf("Client %d: %d %d %d %d %d %s\n", client->socket, client->num_questions, client->time_limit, client->num_easy, client->num_medium, client->num_hard, client->subjects_practice);
    // Sao chép chuỗi môn học
    strncpy(client->subjects_practice, subjects, sizeof(client->subjects_practice) - 1);
    client->subjects_practice[sizeof(client->subjects_practice) - 1] = '\0'; // Đảm bảo chuỗi kết thúc bằng '\0'
    //printf("Client %d: %d %d %d %d %d %s\n", client->socket, client->num_questions, client->time_limit, client->num_easy, client->num_medium, client->num_hard, client->subjects_practice);
    // Khởi tạo câu trả lời và mảng câu hỏi thực hành
    memset(client->answers_practice, 0, sizeof(client->answers_practice));
    for (int i = 0; i < MAX_QUESTIONS; i++) {
        client->questions_practice[i] = NULL;
    }
}
int set_questions_practice(ClientDataPractice* client) {
    Question easy_questions[MAX_QUESTIONS], medium_questions[MAX_QUESTIONS], hard_questions[MAX_QUESTIONS];
    int easy_count, medium_count, hard_count;

    // Lọc câu hỏi theo độ khó và môn học
    filter_questions(easy_questions, &easy_count, 1, client->subjects_practice);
    filter_questions(medium_questions, &medium_count, 2, client->subjects_practice);
    filter_questions(hard_questions, &hard_count, 3, client->subjects_practice);

    if (easy_count < client->num_easy || medium_count < client->num_medium || hard_count < client->num_hard) {
        const char *error_msg = "ERROR_FORMAT: Not enough questions available for the selected criteria.\n";
        send(client->socket, error_msg, strlen(error_msg), 0);
        return -1;
    }

    srand(time(NULL));
    int index = 0;

    // Chọn câu hỏi ngẫu nhiên theo độ khó
    for (int i = 0; i < client->num_easy; i++) {
        if (index < MAX_QUESTIONS) { // Đảm bảo không vượt quá kích thước mảng
            client->questions_practice[index] = &easy_questions[rand() % easy_count];
            index++;
        }
    }
    for (int i = 0; i < client->num_medium; i++) {
        if (index < MAX_QUESTIONS) { // Đảm bảo không vượt quá kích thước mảng
            client->questions_practice[index] = &medium_questions[rand() % medium_count];
            index++;
        }
    }
    for (int i = 0; i < client->num_hard; i++) {
        if (index < MAX_QUESTIONS) { // Đảm bảo không vượt quá kích thước mảng
            client->questions_practice[index] = &hard_questions[rand() % hard_count];
            index++;
        }
    }

    return 0;
}

// Lọc các câu hỏi dựa trên độ khó và môn học
void filter_questions(Question* filtered_questions, int* filtered_count, int difficulty, const char* subjects) {
    *filtered_count = 0;
    
    // Vòng lặp qua tất cả câu hỏi
    for (int i = 0; i < num_questions; i++) {
        // Lọc theo độ khó
        if (questions[i].difficulty != difficulty) continue;
        // Lọc theo môn học
        if (strstr(subjects, "All") || strstr(subjects, questions[i].subject)) {
                filtered_questions[*filtered_count] = questions[i];
                (*filtered_count)++;
        }
    }
    // In thông báo số câu hỏi lọc được cho client
    //printf("Client %d: Filtered %d questions\n", client_socket, *filtered_count);
}
// Xử lý câu trả lời của client trong chế độ thực hành
void handel_answer_practice(ClientDataPractice* client, const char* answer) {
    if (client == NULL) {
         fprintf(stderr, "Error: client is NULL\n");
         return;
    }
    if (strcmp(answer, "TIME") == 0) {

        time_t current_time = time(NULL);
        int time_left = client->time_limit - (current_time - client->start_time);
        char time_message[1024];
        snprintf(time_message, sizeof(time_message), "TIME LEFT: %d seconds\n", time_left);
        send(client->socket, time_message, strlen(time_message), 0);
        return;
    }
    if (strcmp(answer, "SUBMIT") == 0) {
        client->score = calculate_score_practice(client);
        printf("Client %d has submitted the practice early.\n", client->socket);
        send_result_to_client(client);
        //free_client_data_practice(client);
        return;
    }
    int current_question = client->current_question;
    if (current_question < client->num_questions) {
        client->answers_practice[current_question] = answer[0]; // Lưu câu trả lời
        if (client->questions_practice[current_question]->correct_answer == answer[0]) {
            client->score += client->questions_practice[current_question]->difficulty; // Cộng điểm theo độ khó
        }
        client->current_question++; // Chuyển sang câu hỏi tiếp theo
    }
    if (client->current_question == client->num_questions) {
        client->score = calculate_score_practice(client); // Tính điểm
        send_result_to_client(client); // Gửi kết quả bài thi
        //free_client_data_practice(client); // Giải phóng bộ nhớ
    }
    else send_practice_question(client, client->current_question); // Gửi câu hỏi tiếp theo
}
void change_answer_practice(ClientDataPractice* client, int question_number, char new_answer) {
    //printf("%d\n",client->current_question);
    if (question_number >= 1 && question_number <= client->num_questions && question_number <= client->current_question 
        && client->answers_practice[question_number - 1] != new_answer) {
            client->answers_practice[question_number - 1] = new_answer;
            send(client->socket, "CHANGE_SUCCESS: Answer changed successfully\n", 256, 0);
           
    }
    else {
        send(client->socket, "ERROR_CHANGE_ANS: Fail to change question number answer\n", 256, 0);
    }
    //send_practice_question(client, client->current_question); // Gửi câu hỏi tiếp theo
}

// Gửi kết quả bài thi thực hành cho client
void send_result_to_client(ClientDataPractice* client) {
    char result_message[1024];
    snprintf(result_message, sizeof(result_message), 
             "SCORE: %d/%d\n", 
             client->score, client->num_questions);
    send(client->socket, result_message, strlen(result_message), 0);
}

// Tính điểm của client trong chế độ thực hành
int calculate_score_practice(ClientDataPractice* client) {
    int score = 0;
    for (int i = 0; i < client->num_questions; i++) {
        if (client->questions_practice[i]->correct_answer == client->answers_practice[i]) {
            score += 1; // Cộng 1 điểm cho mỗi câu trả lời đúng
        }
    }
    return score;
}

// Gửi câu hỏi thực hành cho client
void send_practice_question(ClientDataPractice* client, int current_question) {
    char question_message[1024];
    Question* question = client->questions_practice[current_question];

    snprintf(question_message, sizeof(question_message),
             "Question %d/%d: %s\nA. %s\nB. %s\nC. %s\nD. %s\n",
             current_question + 1,client->num_questions, question->question, 
             question->option_A, question->option_B, question->option_C, question->option_D);

    send(client->socket, question_message, strlen(question_message), 0);
}
int is_time_remaining(ClientDataPractice* client) {
    time_t current_time = time(NULL); // Lấy thời gian hiện tại
    double elapsed_time = difftime(current_time, client->start_time); // Thời gian đã trôi qua

    if (elapsed_time < client->time_limit) {
        return 1; // Còn thời gian
    } else {
       // free_client_data_practice(client);
        return 0; // Hết thời gian
    }
}
void free_client_data_practice(ClientDataPractice* client) {
    if (client == NULL) {
        return; // Đảm bảo con trỏ không NULL
    }

    // Giải phóng từng câu hỏi trong mảng questions_practice
    for (int i = 0; i < client->num_questions; i++) {
        if (client->questions_practice[i] != NULL) {
            free(client->questions_practice[i]); // Giải phóng câu hỏi
            client->questions_practice[i] = NULL; // Đặt con trỏ NULL để tránh giải phóng lại
        }
    }

    // Cuối cùng, giải phóng cấu trúc chính
    free(client);
}



