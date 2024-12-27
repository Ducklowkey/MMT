#ifndef CLIENT_PRACTICE_H
#define CLIENT_PRACTICE_H

#include "client.h"
#include <unistd.h>

void handle_practice(Client* client);
void submit_answer_practice(Client* client, const char* answer);
void start_and_set_format(Client* client);
void request_time_left(Client* client);
void submit_practice_early(Client* client);
#endif // CLIENT_PRACTICE_H
