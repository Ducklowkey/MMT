// common/include/types.h
#ifndef TYPES_H
#define TYPES_H

#include "constants.h"
#include <time.h>
#include <signal.h>

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
    char subjects_practice[BUFFER_SIZE];            // Môn học
    char answers_practice[MAX_QUESTIONS];             // Câu trả lời
    Question* questions_practice[MAX_QUESTIONS];           // Mảng câu hỏi
} ClientDataPractice;

typedef struct { //Giúp server quản lý thông tin client 
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
    char answers[MAX_QUESTIONS];  // Mảng lưu đáp án đã chọn
    int question_answered[MAX_QUESTIONS];  // Đánh dấu câu hỏi đã trả lời
    int in_review_mode;  // Flag đánh dấu đang trong chế độ xem lại
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
    int difficulty;     
    time_t start_time;
    int num_easy;           // Số câu hỏi dễ
    int num_medium;         // Số câu hỏi trung bình
    int num_hard;           // Số câu hỏi khó
    char subjects[256];     // Các môn học đã chọn
    int time_limit;         // Thời gian làm bài (phút)
} ExamRoom;

typedef struct {
    int exam_id;
    char username[MAX_USERNAME];
    int score;
    time_t exam_date;
    int total_questions;
} ExamResult;

typedef struct {
    int server_fd;
    struct pollfd *pfds;
    int fd_count;
    ClientInfo *clients;
    char subject_list[BUFFER_SIZE];
    volatile sig_atomic_t running;
} Server;

typedef struct {
    int socket;
    char username[MAX_USERNAME];
    int is_authenticated;
    int current_room;
    int is_room_creator;
} Client;



#endif // TYPES_H
