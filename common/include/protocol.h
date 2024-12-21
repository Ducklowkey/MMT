// common/include/protocol.h
#ifndef PROTOCOL_H
#define PROTOCOL_H

// Authentication commands
#define CMD_REGISTER "REGISTER"
#define CMD_LOGIN "LOGIN"
#define CMD_LOGOUT "LOGOUT"

// Room management commands
#define CMD_CREATE_ROOM "CREATE_ROOM"
#define CMD_JOIN_ROOM "JOIN_ROOM"
#define CMD_LEAVE_ROOM "LEAVE_ROOM"
#define CMD_LIST_ROOMS "LIST_ROOMS"
#define CMD_DELETE_ROOM "DELETE_ROOM"
#define CMD_START_EXAM "START_EXAM"

// Add question 
#define CMD_ADD_QUESTION "ADD_QUESTION"

// Exam commands
#define CMD_SUBMIT_ANSWER "SUBMIT_ANSWER"
#define CMD_SUBMIT_EARLY "SUBMIT_EARLY"

// Response codes
#define RESP_SUCCESS 200
#define RESP_AUTH_FAILED 401
#define RESP_NOT_FOUND 404
#define RESP_ERROR 500

#endif // PROTOCOL_H
