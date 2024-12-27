#ifndef PRACTICE_H
#define PRACTICE_H

#include "../include/exam.h"
#include "../include/server.h"
#include "../include/constants.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

// typedef struct {
//     int socket;                 // Socket của client
//     char answers[MAX_QUESTIONS];   // Lưu câu trả lời của client
//     int current_question;          // Câu hỏi hiện tại
//     int num_questions;             // Số lượng câu hỏi
//     int time_limit;                // Thời gian giới hạn
//     int num_easy;                  // Số lượng câu dễ
//     int num_medium;              // Số lượng câu trung bình
//     int num_hardt;                // Số lượng câu khó
//     time_t start_time;             // Thời gian bắt đầu
//     int score;                     // Điểm của client
//     char subjects[100];            // Môn học
//     char answers[100];             // Câu trả lời
//     Question* questions_practice[num_questions];           // Mảng câu hỏi
// } ClientDataPractice;
ClientDataPractice* create_client_data_practice(int socket, int num_questions, int time_limit, int num_easy, int num_medium, int num_hard, const char* subjects);
// enum subjects {
//     MATH = 1,
//     GEOGRAPHY,
//     HISTORY,
//     LITERATURE,
//     ENGLISH,
//     PHYSICS,
//     CHEMISTRY,
//     BIOLOGY,
//     ALL
// };

// Khai báo mảng câu hỏi và biến số lượng câu hỏi
extern Question all_questions[MAX_QUESTIONS];
extern int num_all_question;
void load_all_questions(void);
int set_questions_practice(ClientDataPractice* client);
void filter_questions(Question* filtered_questions, int* filtered_count, int difficulty, const char* subjects);
void handle_practice_mode(ClientDataPractice* client);
void handel_answer_practice(ClientDataPractice* client, const char* answer);
void send_result_to_client(ClientDataPractice* client);
int calculate_score_practice(ClientDataPractice* client);
void send_practice_question(ClientDataPractice* client, int current_question);
int is_time_remaining(ClientDataPractice* client);

#endif // PRACTICE_H
