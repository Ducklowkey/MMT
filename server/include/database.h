#ifndef DATABASE_H
#define DATABASE_H

#include "server.h"

// Ghi kết quả thi vào file results.txt , định dạng: username,room_id,score,total,timestamp
void save_exam_result(const char* username, int room_id, int score, int total);

// Lấy kết quả thi của một user
void get_user_results(const char* username);

#endif // DATABASE_H