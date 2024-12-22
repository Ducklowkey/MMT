// server/include/auth.h
#ifndef AUTH_H
#define AUTH_H

#include "server.h"

int register_user(const char* username, const char* password);
int login_user(const char* username, const char* password, char* session_id);
void handle_authentication(ClientInfo* client, const char* command);
void handle_logout(ClientInfo* client);

#endif // AUTH_H
