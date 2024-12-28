#include "../include/exam.h"
#include "../include/room.h"
#include "../include/practice.h"
#include "../include/server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Khai báo mảng câu hỏi và biến số lượng câu hỏi
Question all_questions[MAX_QUESTIONS];
int num_all_question = 0;
//ClientDataPractice* client; 
// Hàm nạp câu hỏi từ file
void load_all_question(void) {
    FILE* file = fopen("questions.txt", "r");
    if (!file) {
        perror("Cannot open questions file");
        return;
    }
    
    char line[BUFFER_SIZE];
    while (num_all_question < MAX_QUESTIONS && fgets(line, BUFFER_SIZE, file)) {
        Question* q = &all_questions[num_all_question];

        line[strcspn(line, "\n")] = 0;
        strncpy(q->subject, line, sizeof(q->subject) - 1);

        if (!fgets(line, BUFFER_SIZE, file)) break;
        q->difficulty = atoi(line);

        if (!fgets(line, BUFFER_SIZE, file)) break;
        line[strcspn(line, "\n")] = 0;
        strncpy(q->question, line, sizeof(q->question) - 1);

        if (!fgets(line, BUFFER_SIZE, file)) break;
        line[strcspn(line, "\n")] = 0;
        strncpy(q->option_A, line, sizeof(q->option_A) - 1);

        if (!fgets(line, BUFFER_SIZE, file)) break;
        line[strcspn(line, "\n")] = 0;
        strncpy(q->option_B, line, sizeof(q->option_B) - 1);

        if (!fgets(line, BUFFER_SIZE, file)) break;
        line[strcspn(line, "\n")] = 0;
        strncpy(q->option_C, line, sizeof(q->option_C) - 1);

        if (!fgets(line, BUFFER_SIZE, file)) break;
        line[strcspn(line, "\n")] = 0;
        strncpy(q->option_D, line, sizeof(q->option_D) - 1);

        if (!fgets(line, BUFFER_SIZE, file)) break;
        q->correct_answer = line[0];

        num_all_question++;
    }

    fclose(file);
    printf("Loaded %d questions\n", num_all_question);
}
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

    // Sao chép chuỗi môn học
    strncpy(client->subjects_practice, subjects, sizeof(client->subjects_practice) - 1);
    client->subjects_practice[sizeof(client->subjects_practice) - 1] = '\0'; // Đảm bảo chuỗi kết thúc bằng '\0'

    // Khởi tạo câu trả lời và mảng câu hỏi thực hành
    memset(client->answers_practice, 0, sizeof(client->answers_practice));
    for (int i = 0; i < MAX_QUESTIONS; i++) {
        client->questions_practice[i] = NULL;
    }
}
int set_questions_practice(ClientDataPractice* client) {
    Question easy_questions[MAX_QUESTIONS], medium_questions[MAX_QUESTIONS], hard_questions[MAX_QUESTIONS];
    int easy_count, medium_count, hard_count;
    load_all_question();

    // Lọc câu hỏi theo độ khó và môn học
    filter_questions(easy_questions, &easy_count, 1, client->subjects_practice);
    filter_questions(medium_questions, &medium_count, 2, client->subjects_practice);
    filter_questions(hard_questions, &hard_count, 3, client->subjects_practice);

    // Điều chỉnh số lượng câu hỏi theo số lượng có sẵn
    if (client->num_easy > easy_count) {
        printf("Điều chỉnh số câu hỏi dễ từ %d xuống %d\n", client->num_easy, easy_count);
        client->num_easy = easy_count;
    }
    if (client->num_medium > medium_count) {
        printf("Điều chỉnh số câu hỏi trung bình từ %d xuống %d\n", client->num_medium, medium_count);
        client->num_medium = medium_count;
    }
    if (client->num_hard > hard_count) {
        printf("Điều chỉnh số câu hỏi khó từ %d xuống %d\n", client->num_hard, hard_count);
        client->num_hard = hard_count;
    }

    // Cập nhật tổng số câu hỏi
    int total_available = client->num_easy + client->num_medium + client->num_hard;
    if (total_available == 0) {
        const char *error_msg = "ERROR_FORMAT:Không tìm thấy câu hỏi phù hợp với các tiêu chí đã chọn.\n";
        send(client->socket, error_msg, strlen(error_msg), 0);
        return -1;
    }

    client->num_questions = total_available;
    
    // Chọn câu hỏi ngẫu nhiên
    srand(time(NULL));
    int index = 0;

    // Thêm câu hỏi dễ
    for (int i = 0; i < client->num_easy; i++) {
        client->questions_practice[index] = malloc(sizeof(Question));
        if (client->questions_practice[index]) {
            *(client->questions_practice[index]) = easy_questions[rand() % easy_count];
            index++;
        }
    }

    // Thêm câu hỏi trung bình
    for (int i = 0; i < client->num_medium; i++) {
        client->questions_practice[index] = malloc(sizeof(Question));
        if (client->questions_practice[index]) {
            *(client->questions_practice[index]) = medium_questions[rand() % medium_count];
            index++;
        }
    }

    // Thêm câu hỏi khó
    for (int i = 0; i < client->num_hard; i++) {
        client->questions_practice[index] = malloc(sizeof(Question));
        if (client->questions_practice[index]) {
            *(client->questions_practice[index]) = hard_questions[rand() % hard_count];
            index++;
        }
    }

    // Thông báo số lượng câu hỏi thực tế
    char info_msg[256];
    snprintf(info_msg, sizeof(info_msg), 
        "Bài tập gồm %d câu: %d dễ, %d trung bình, %d khó\n",
        client->num_questions, client->num_easy, client->num_medium, client->num_hard);
    send(client->socket, info_msg, strlen(info_msg), 0);

    return 0;
}
// Lọc các câu hỏi dựa trên độ khó và môn học
void filter_questions(Question* filtered_questions, int* filtered_count, int difficulty, const char* subjects) {
    *filtered_count = 0;
    
    // Vòng lặp qua tất cả câu hỏi
    for (int i = 0; i < num_questions; i++) {
        // Lọc theo độ khó
        if (all_questions[i].difficulty != difficulty) continue;

        // Lọc theo môn học
        if (strstr(subjects, "All") || strstr(subjects, all_questions[i].subject)) {
                filtered_questions[*filtered_count] = all_questions[i];
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
        send_result_to_client(client);
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
    }
    else send_practice_question(client, client->current_question); // Gửi câu hỏi tiếp theo
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
        return 0; // Hết thời gian
    }
}

