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
#define MAX_QUESTION_RETRIES 3  // Số lần được phép thay đổi đáp án
#define REVIEW_TIME_LIMIT 300   // Thời gian cho phép xem lại (5 phút)
#define MAX_EXAM_DURATION 7200  // Thời gian tối đa cho một bài thi (2 giờ)

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
#define ROOM_REVIEWING 3   // Trạng thái phòng đang trong chế độ xem lại

// Answer settings
#define MAX_ANSWER_LENGTH 1
#define VALID_ANSWERS "ABCD"

// Review mode settings
#define REVIEW_ENABLED 1
#define REVIEW_DISABLED 0
#define MAX_REVIEWS_PER_QUESTION 3  // Số lần được xem lại mỗi câu hỏi

// Format settings
#define MIN_QUESTIONS 10            // Số câu hỏi tối thiểu
#define MAX_QUESTIONS_PER_EXAM 60   // Số câu hỏi tối đa
#define MIN_TIME_LIMIT 30           // Thời gian thi tối thiểu (phút)
#define MAX_TIME_LIMIT 180          // Thời gian thi tối đa (phút)
#define MAX_DIFFICULTY_RATIO 100    // Tỷ lệ độ khó tối đa (%)

// Score settings
#define SCORE_PER_EASY 1       // Điểm cho câu dễ
#define SCORE_PER_MEDIUM 2     // Điểm cho câu trung bình
#define SCORE_PER_HARD 3       // Điểm cho câu khó
#define PENALTY_FOR_CHANGE 0.5  // Điểm trừ khi thay đổi đáp án

// Timeout settings
#define ANSWER_TIMEOUT 300     // Thời gian tối đa cho mỗi câu (giây)
#define REVIEW_TIMEOUT 60      // Thời gian tối đa xem lại mỗi câu (giây)
#define CHANGE_TIMEOUT 120     // Thời gian tối đa để thay đổi đáp án (giây)

// Debug levels
#define DEBUG_NONE 0
#define DEBUG_ERROR 1
#define DEBUG_INFO 2
#define DEBUG_VERBOSE 3

#endif // CONSTANTS_H