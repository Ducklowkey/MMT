#ifndef CLIENT_PRACTICE_H
#define CLIENT_PRACTICE_H

#include "client.h"
#include <unistd.h>

void handle_practice(Client* client);
void submit_answer_practice(Client* client, char answer);
void submit_practice_early(Client* client);

#endif // CLIENT_PRACTICE_H
