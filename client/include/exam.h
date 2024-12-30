// client/include/exam.h
#ifndef CLIENT_EXAM_H
#define CLIENT_EXAM_H

#include "client.h"

void handle_exam(Client* client);
void configure_practice(Client* client, int* num_questions_total, int* time_limit, int* num_easy, int* num_medium, int* num_hard, char* subjects);
void print_subjects_menu(Client* client);

#endif // CLIENT_EXAM_H
