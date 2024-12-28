#ifndef SERVER_EXAM_H
#define SERVER_EXAM_H

#include "server.h"

void load_questions(void);
void send_question(ClientInfo* client, int question_number);
void broadcast_to_room(Server* server, ExamRoom* room, const char* message);
void start_exam(Server* server, ExamRoom* room);
void handle_answer(ClientInfo* client, char answer);
void get_available_subjects(char* subjects_list, size_t size);
int is_exam_time_remaining(ExamRoom* room);
void handle_exam_submit(ClientInfo* client);
void send_time_remaining(ClientInfo* client);

extern Question questions[MAX_QUESTIONS];
extern int num_questions;

#endif // SERVER_EXAM_H
