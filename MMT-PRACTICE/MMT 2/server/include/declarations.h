// server/include/declarations.h
#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#include "../../common/include/types.h"

// Authentication functions
void handle_authentication(ClientInfo* client, const char* command);
void handle_logout(ClientInfo* client);

// Room management functions
int create_exam_room(const char* room_name, const char* creator);
int join_exam_room(int room_id, const char* username, ClientInfo* client);
void leave_exam_room(int room_id, const char* username, ClientInfo* client);
void delete_exam_room(int room_id, ClientInfo* clients);
void get_room_list(char* buffer);
int is_room_creator(int room_id, const char* username);
ExamRoom* get_room(int room_id);

// Exam functions
void send_question(ClientInfo* client, int question_number);
void handle_answer(ClientInfo* client, char answer);
void calculate_score(ClientInfo* client);
void start_exam(Server* server, ExamRoom* room);

#endif // DECLARATIONS_H
