// common/include/constants.h
#ifndef CONSTANTS_H
#define CONSTANTS_H

// Network settings
#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define MAX_FDS (MAX_CLIENTS + 1) 

// User limits
#define MAX_USERNAME 100
#define MAX_PASSWORD 100
#define MAX_ROOMNAME 50

// Session settings
#define SESSION_TIMEOUT 300  // 5 minutes in seconds

// Exam settings
#define MAX_QUESTIONS 1000
#define NUM_QUESTIONS_PER_EXAM 10
#define EXAM_TIME_LIMIT 3600  // 1 hour in seconds

// Practice settings
#define MAX_PRACTICE_QUESTIONS 100
#define MAX_PRACTICE_TIME 7200  // 2 hours in seconds
#define MAX_SUBJECTS 20
#define MAX_SUBJECT_NAME 50

// File paths
#define USERS_FILE "users.txt"
#define QUESTIONS_FILE "questions.txt"
#define RESULTS_FILE "results.txt"

// Question difficulty levels
#define DIFFICULTY_EASY 1
#define DIFFICULTY_MEDIUM 2
#define DIFFICULTY_HARD 3

// Room status
#define ROOM_WAITING 0
#define ROOM_IN_PROGRESS 1
#define ROOM_COMPLETED 2

// Answer settings
#define MAX_ANSWER_LENGTH 1
#define VALID_ANSWERS "ABCD"

#endif // CONSTANTS_H