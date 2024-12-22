#ifndef PRACTICE_H
#define PRACTICE_H

#include "../include/exam.h"
#include "../include/server.h"
#include "../include/constants.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

typedef struct {
    char answers[MAX_QUESTIONS];   // Lưu câu trả lời của client
    time_t start_time;             // Thời gian bắt đầu
    int score;                     // Điểm của client
} ClientData;


// Khai báo mảng câu hỏi và biến số lượng câu hỏi
extern Question questions_practice[MAX_QUESTIONS];
extern int num_questions_practice;

// Hàm nạp câu hỏi từ file
void load_questions_practices(void);

// Hàm lọc câu hỏi theo thông số đã chọn
void filter_questions(Question* filtered_questions, int* filtered_count, int difficulty, const char* subjects, int client_id);

// Hàm bắt đầu chế độ luyện tập
void send_result_to_client(int score, int num_questions_total, int client_socket);
int calculate_score_practice(const char* answers, const Question* questions, int num_questions, int client_id);
void start_practice_mode(int num_questions_total, int time_limit, int num_easy, int num_medium, int num_hard, const char* subjects, int client_socket);

#endif // PRACTICE_H
