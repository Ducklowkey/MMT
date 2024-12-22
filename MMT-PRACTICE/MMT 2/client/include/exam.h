// client/include/exam.h
#ifndef CLIENT_EXAM_H
#define CLIENT_EXAM_H

#include "client.h"
#include <unistd.h>

void handle_exam(Client* client);
void submit_answer(Client* client, char answer);
void submit_exam_early(Client* client);

#endif // CLIENT_EXAM_H
