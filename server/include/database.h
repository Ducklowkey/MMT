// server/include/database.h
#ifndef DATABASE_H
#define DATABASE_H

#include "../../common/include/types.h"

void init_database(void);
void save_exam_result_to_db(const ExamResult* result);
void get_user_results(const char* username, ExamResult** results, int* count);
void close_database(void);

#endif // DATABASE_H
