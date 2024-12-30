#ifndef CLIENT_PRACTICE_H
#define CLIENT_PRACTICE_H

#include "client.h"
#include <unistd.h>

void handle_practice(Client* client);
void submit_answer_practice(Client* client, const char* answer);
void start_and_set_format(Client* client);
void request_time_left(Client* client);
void submit_practice_early(Client* client);
void change_answer(Client* client);
void print_subjects_menu(Client* client);
void configure_practice(Client* client, int* num_questions_total, int* time_limit, int* num_easy, int* num_medium, int* num_hard, char* subjects);
void send_practice_config(Client* client, int num_questions_total, int time_limit, int num_easy, int num_medium, int num_hard, char* subjects);
#endif // CLIENT_PRACTICE_H