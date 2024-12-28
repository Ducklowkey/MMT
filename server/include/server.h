#ifndef SERVER_H
#define SERVER_H

#include "../../common/include/constants.h"
#include "../../common/include/protocol.h"
#include "../../common/include/types.h"
#include "../../server/include/room.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Server function prototypes
Server* create_server(void);
void destroy_server(Server* server);
void start_server(Server* server);
void handle_client_message(Server* server, int client_index, char* buffer);
void handle_new_connection(Server* server);
void handle_disconnection(Server* server, int client_index);
void cleanup_practice_session(ClientDataPractice* practice);

#endif // SERVER_H