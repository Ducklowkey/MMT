// common/include/types.h
#ifndef TYPES_H
#define TYPES_H

#include "constants.h"
#include <time.h>
#include <signal.h>

typedef struct { // Quản lý thông tin cho client 
    int fd;
    int active;
    int authenticated;
    char username[MAX_USERNAME];
    char session_id[50];
    time_t session_start;
    int current_room_id;
    int current_question;
    int score;
    
    time_t training_start_time;  // Thời điểm bắt đầu training
    int time_limit;             // Thời hạn (phút)
    int num_questions;          // Số câu hỏi trong bài training
    int question_ids[MAX_QUESTIONS];  // Mảng lưu ID các câu hỏi
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
    int server_fd;
    struct pollfd *pfds;
    int fd_count;
    ClientInfo *clients;
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
