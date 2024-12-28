// client/include/client.h
#ifndef CLIENT_H
#define CLIENT_H

#include "../../common/include/constants.h"
#include "../../common/include/protocol.h"
#include "../../common/include/types.h"

// Connection functions
int connect_to_server(const char* address, int port);
void disconnect_from_server(Client* client);
int send_message(Client* client, const char* message);
int receive_message(Client* client, char* buffer);

// Authentication functions
int handle_authentication(Client* client);
void show_auth_menu(void);

#endif // CLIENT_H
