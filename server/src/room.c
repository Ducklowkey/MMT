#include "../include/room.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static ExamRoom exam_rooms[MAX_CLIENTS];
static int num_rooms = 0;

int create_exam_room(const char* room_name, const char* creator) {
    if (num_rooms >= MAX_CLIENTS) {
        return -1;
    }

    ExamRoom* room = &exam_rooms[num_rooms];
    room->room_id = num_rooms + 1;
    strncpy(room->room_name, room_name, MAX_ROOMNAME - 1);
    strncpy(room->creator_username, creator, MAX_USERNAME - 1);
    room->is_active = 1;
    room->user_count = 1;
    strncpy(room->users[0], creator, MAX_USERNAME - 1);
    room->status = 0;  // PENDING
    room->start_time = 0;
    room->difficulty = 1;
    memset(room->question_ids, 0, sizeof(room->question_ids));
    room->num_questions = 0;

    printf("Created room %d: %s by %s\n", room->room_id, room_name, creator);
    num_rooms++;
    return room->room_id;
}

int join_exam_room(int room_id, const char* username, ClientInfo* client) {
    printf("Attempting to join room %d by user %s\n", room_id, username);
    
    for (int i = 0; i < num_rooms; i++) {
        if (exam_rooms[i].room_id == room_id && exam_rooms[i].is_active) {
            // Kiểm tra xem user đã trong room chưa
            for (int j = 0; j < exam_rooms[i].user_count; j++) {
                if (strcmp(exam_rooms[i].users[j], username) == 0) {
                    printf("User %s already in room %d\n", username, room_id);
                    client->current_room_id = room_id;
                    return 0;
                }
            }

            // Thêm user mới
            if (exam_rooms[i].user_count < MAX_CLIENTS) {
                strncpy(exam_rooms[i].users[exam_rooms[i].user_count],
                        username, MAX_USERNAME - 1);
                exam_rooms[i].user_count++;
                client->current_room_id = room_id;
                printf("User %s joined room %d successfully\n", username, room_id);
                return 0;
            }
            printf("Room %d is full\n", room_id);
            return -1;
        }
    }
    printf("Room %d not found or not active\n", room_id);
    return -2;
}

void leave_exam_room(int room_id, const char* username, ClientInfo* client) {
    printf("User %s attempting to leave room %d\n", username, room_id);
    
    for (int i = 0; i < num_rooms; i++) {
        if (exam_rooms[i].room_id == room_id && exam_rooms[i].is_active) {
            for (int j = 0; j < exam_rooms[i].user_count; j++) {
                if (strcmp(exam_rooms[i].users[j], username) == 0) {
                    // Di chuyển các user còn lại lên
                    for (int k = j; k < exam_rooms[i].user_count - 1; k++) {
                        strcpy(exam_rooms[i].users[k], exam_rooms[i].users[k + 1]);
                    }
                    exam_rooms[i].user_count--;
                    
                    // Reset client state
                    client->current_room_id = -1;
                    client->current_question = -1;
                    client->score = 0;

                    printf("User %s left room %d\n", username, room_id);
                    
                    // Deactivate room if empty
                    if (exam_rooms[i].user_count == 0) {
                        exam_rooms[i].is_active = 0;
                        exam_rooms[i].status = 0;
                        printf("Room %d deactivated (no users)\n", room_id);
                    }
                    return;
                }
            }
        }
    }
}

void delete_exam_room(int room_id, ClientInfo* clients) {
    printf("Attempting to delete room %d\n", room_id);
    
    for (int i = 0; i < num_rooms; i++) {
        if (exam_rooms[i].room_id == room_id && exam_rooms[i].is_active) {
            // Notify and remove all users
            for (int j = 0; j < exam_rooms[i].user_count; j++) {
                for (int k = 0; k < MAX_CLIENTS; k++) {
                    if (clients[k].active && 
                        strcmp(clients[k].username, exam_rooms[i].users[j]) == 0) {
                        char msg[] = "Room has been deleted by creator\n";
                        send(clients[k].fd, msg, strlen(msg), 0);
                        clients[k].current_room_id = -1;
                        clients[k].current_question = -1;
                        clients[k].score = 0;
                    }
                }
            }
            exam_rooms[i].is_active = 0;
            exam_rooms[i].status = 0;
            exam_rooms[i].user_count = 0;
            printf("Room %d deleted successfully\n", room_id);
            break;
        }
    }
}

void get_room_list(char* buffer) {
    buffer[0] = '\0';
    char temp[BUFFER_SIZE];
    int active_rooms = 0;

    for (int i = 0; i < num_rooms; i++) {
        if (exam_rooms[i].is_active) {
            snprintf(temp, BUFFER_SIZE,
                    "Room %d: %s (Created by: %s, Users: %d)\n",
                    exam_rooms[i].room_id,
                    exam_rooms[i].room_name,
                    exam_rooms[i].creator_username,
                    exam_rooms[i].user_count);
            strcat(buffer, temp);
            active_rooms++;
        }
    }

    if (active_rooms == 0) {
        strcpy(buffer, "No active rooms available\n");
    }
}

int is_room_creator(int room_id, const char* username) {
    for (int i = 0; i < num_rooms; i++) {
        if (exam_rooms[i].room_id == room_id && exam_rooms[i].is_active) {
            return strcmp(exam_rooms[i].creator_username, username) == 0;
        }
    }
    return 0;
}

ExamRoom* get_room(int room_id) {
    if (room_id <= 0) {
        printf("Invalid room ID: %d\n", room_id);
        return NULL;
    }

    for (int i = 0; i < num_rooms; i++) {
        if (exam_rooms[i].room_id == room_id && exam_rooms[i].is_active) {
            printf("Found room %d (Active: %d, Status: %d, Users: %d)\n",
                   room_id, exam_rooms[i].is_active, 
                   exam_rooms[i].status, exam_rooms[i].user_count);
            return &exam_rooms[i];
        }
    }
    printf("Room %d not found or not active\n", room_id);
    return NULL;
}