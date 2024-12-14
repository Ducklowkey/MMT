// common/include/types.h
#ifndef TYPES_H
#define TYPES_H

#include "constants.h"
#include <time.h>

typedef struct {
    int fd;
    char username[MAX_USERNAME];
    char session_id[50];
    time_t session_start;
    int authenticated;
    int current_room_id;
    int active;
    int current_question;
    int score;
} ClientInfo;

typedef struct {
    int room_id;
    char room_name[MAX_ROOMNAME];
    char creator_username[MAX_USERNAME];
    int is_active;
    int user_count;
    char users[MAX_CLIENTS][MAX_USERNAME];
    int num_questions;
    int question_ids[MAX_QUESTIONS];
    int status;
    int difficulty;     // Added difficulty level
    time_t start_time;
} ExamRoom;

typedef struct {
    int exam_id;
    char username[MAX_USERNAME];
    int score;
    time_t exam_date;
    int total_questions;
} ExamResult;

#endif // TYPES_H
