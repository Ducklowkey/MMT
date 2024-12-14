// server/include/room.h
#ifndef ROOM_H
#define ROOM_H

#include "server.h"
#include "../../common/include/types.h"

// Room management function prototypes
int create_exam_room(const char* room_name, const char* creator);
int join_exam_room(int room_id, const char* username, ClientInfo* client);
void leave_exam_room(int room_id, const char* username, ClientInfo* client);
void delete_exam_room(int room_id, ClientInfo* clients);
void get_room_list(char* buffer);
int is_room_creator(int room_id, const char* username);

#endif // ROOM_H
