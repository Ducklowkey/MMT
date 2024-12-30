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
    int socket;    
    int current_question;          
    int num_questions;  // Số câu 
    int time_limit;                
    int num_easy;                  
    int num_medium;              
    int num_hard;                
    time_t start_time;  
    int score;                     
    char subjects_practice[256];          
    char answers_practice[256];  // Câu trả lời
    Question* questions_practice[100];           
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
    char answers[MAX_QUESTIONS];  // lưu đáp án đã chọn
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
    int num_easy;           
    int num_medium;         
    int num_hard;           
    char subjects[256];     
    int time_limit;        
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
