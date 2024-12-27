// common/include/types.h
#ifndef TYPES_H
#define TYPES_H

#include "constants.h"
#include <time.h>

typedef struct {
    char subject[50];
    int difficulty;
    char question[200];
    char option_A[100];
    char option_B[100];
    char option_C[100];
    char option_D[100];
    char correct_answer;
} Question;
typedef struct {
    int socket;                 // Socket của client
    //char answers[MAX_QUESTIONS];   // Lưu câu trả lời của client
    int current_question;          // Câu hỏi hiện tại
    int num_questions;             // Số lượng câu hỏi
    int time_limit;                // Thời gian giới hạn
    int num_easy;                  // Số lượng câu dễ
    int num_medium;              // Số lượng câu trung bình
    int num_hard;                // Số lượng câu khó
    time_t start_time;             // Thời gian bắt đầu
    int score;                     // Điểm của client
    char subjects_practice[256];            // Môn học
    char answers_practice[256];             // Câu trả lời
    Question* questions_practice[100];           // Mảng câu hỏi
} ClientDataPractice;
typedef struct {
    int fd;
    char username[MAX_USERNAME];
    char session_id[50];
    time_t session_start;
    int authenticated;
    int current_room_id;
    int active;
    int current_question;
    int score;
    ClientDataPractice* client_practice;    
} ClientInfo;

typedef struct {
    int room_id;
    char room_name[MAX_ROOMNAME];
    char creator_username[MAX_USERNAME];
    int is_active;
    int user_count;
    char users[MAX_CLIENTS][MAX_USERNAME];
    int num_questions;
    int question_ids[MAX_QUESTIONS];
    int status;
    int difficulty;     // Added difficulty level
    time_t start_time;
} ExamRoom;

typedef struct {
    int exam_id;
    char username[MAX_USERNAME];
    int score;
    time_t exam_date;
    int total_questions;
} ExamResult;


#endif // TYPES_H
