// server/include/exam.h
#ifndef SERVER_EXAM_H
#define SERVER_EXAM_H

#include "server.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#define NUM_QUESTIONS_PER_EXAM 10  // Số câu hỏi mỗi bài thi
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

void load_questions(void);
void send_question(ClientInfo* client, int question_number);
void handle_answer(ClientInfo* client, char answer);
void calculate_score(ClientInfo* client);
void save_exam_result(ClientInfo* client, ExamRoom* room);

void start_exam(Server* server, ExamRoom* room);
extern Question questions[MAX_QUESTIONS];
extern int num_questions;

#endif // SERVER_EXAM_H
