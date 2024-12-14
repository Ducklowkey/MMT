#ifndef SERVER_H
#define SERVER_H

#include "../../common/include/constants.h"
#include "../../common/include/protocol.h"
#include "../../common/include/types.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <poll.h>

typedef struct {
    int server_fd;
    struct pollfd *pfds;
    int fd_count;
    ClientInfo *clients;
    volatile sig_atomic_t running;
} Server;

// Server function prototypes
Server* create_server(void);
void destroy_server(Server* server);
void start_server(Server* server);
void handle_client_message(Server* server, int client_index, char* buffer); // Cập nhật: Thêm tham số buffer
void handle_new_connection(Server* server);
void handle_disconnection(Server* server, int client_index);
ExamRoom* get_room(int room_id);
void leave_exam_room(int room_id, const char* username, ClientInfo* client);
void save_exam_result_to_db(const ExamResult* result);

#endif // SERVER_H
