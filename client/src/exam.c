#include "../include/exam.h"
#include "../include/ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <ctype.h>

void handle_exam(Client* client) {
    char buffer[BUFFER_SIZE];  
    fd_set readfds;
    int max_fd = client->socket;
    int exam_completed = 0;
    char input[BUFFER_SIZE];  

    printf("\nLuật: SUBMIT để nộp bài, TIME để xem thời gian còn lại\n");

    while (!exam_completed) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);  
        FD_SET(client->socket, &readfds);

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int activity = select(max_fd + 1, &readfds, NULL, NULL, &tv);

        if (FD_ISSET(client->socket, &readfds)) {
            int valread = receive_message(client, buffer);
            if (valread <= 0) {
                print_error("Server disconnected");
                break; 
            }

            printf("\n%s", buffer);

            if (strstr(buffer, "Exam completed") != NULL || 
                strstr(buffer, "final score") != NULL) {
                exam_completed = 1;
                break;
            }
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (fgets(input, sizeof(input), stdin)) {
                // Xóa newline
                input[strcspn(input, "\n")] = 0;
                
                // Xóa khoảng trắng ở cuối
                char* end = input + strlen(input) - 1;
                while(end > input && isspace(*end)) {
                    *end = '\0';
                    end--;
                }

                
                if (strcmp(input, "TIME") == 0) {
                    send_message(client, "TIME");
                }
                else if (strcmp(input, "SUBMIT") == 0) {
                    send_message(client, "SUBMIT");
                }
                
                else if (strncmp(input, "REVIEW", 6) == 0) {
                    char cmd[BUFFER_SIZE];
                    int question_num;
                    if(sscanf(input + 6, "%d", &question_num) == 1) {
                        snprintf(cmd, BUFFER_SIZE, "REVIEW %d", question_num);
                        send_message(client, cmd);
                    }
                }
                else if (strncmp(input, "CHANGE", 6) == 0) {
                    char cmd[BUFFER_SIZE];
                    int question_num;
                    char answer;
                    if(sscanf(input + 6, "%d %c", &question_num, &answer) == 2) {
                        snprintf(cmd, BUFFER_SIZE, "CHANGE %d %c", question_num, answer);
                        send_message(client, cmd); 
                    }
                }
                // Xử lý đáp án như bình thường
                else if (strlen(input) == 1) {
                    char answer = input[0];
                    if (answer >= 'a' && answer <= 'd') answer -= 32;
                    if (answer >= 'A' && answer <= 'D') {
                        char cmd[BUFFER_SIZE];
                        snprintf(cmd, BUFFER_SIZE, "SUBMIT_ANSWER %c", answer);
                        send_message(client, cmd);
                    }
                }
            }
        }
    }
}
// set format 
void configure_practice(Client* client, int* num_questions_total, int* time_limit, int* num_easy, int* num_medium, int* num_hard, char* subjects) {
   print_subjects_menu(client);

   char buffer[BUFFER_SIZE];
   send_message(client, "GET_SUBJECTS");
   memset(buffer, 0, BUFFER_SIZE);
   
   if (receive_message(client, buffer) <= 0 || strncmp(buffer, "SUBJECTS|", 9) != 0) {
       printf("Không thể lấy danh sách môn học từ server\n");
       return;
   }

   // Phân tích danh sách môn học
   char* subjects_list = buffer + 9;
   char* subject_names[MAX_QUESTIONS];
   int num_subjects = 0;
   char* subject = strtok(subjects_list, ",");
   while (subject != NULL && num_subjects < MAX_QUESTIONS) {
       subject_names[num_subjects++] = strdup(subject);
       subject = strtok(NULL, ",");
   }

   // Chọn môn học
   int selected_subjects[MAX_QUESTIONS] = {0};
   char input[10];
   int subject_choice;
   int has_selected = 0; // Flag để kiểm tra đã chọn môn học nào chưa

   printf("\nNhập số thứ tự môn học (0 để kết thúc):\n");
   while (1) {
       printf("Số thứ tự: ");
       if (!fgets(input, sizeof(input), stdin)) {
           printf("Lỗi khi đọc input\n");
           continue;
       }
       input[strcspn(input, "\n")] = 0;

       // Kiểm tra input rỗng
       if (strlen(input) == 0) continue;

       // Chuyển input thành số
       subject_choice = atoi(input);

       if (subject_choice == 0) {
           if (!has_selected) {
               // Nếu chưa chọn môn nào, trả về chuỗi rỗng
               subjects[0] = '\0';
               
               // Giải phóng bộ nhớ trước khi return
               for (int i = 0; i < num_subjects; i++) {
                   free(subject_names[i]);
               }
               return;
           }
           break;
       }
       
       if (subject_choice >= 1 && subject_choice <= num_subjects) {
           selected_subjects[subject_choice - 1] = 1;
           has_selected = 1;
       } else {
           printf("Lựa chọn không hợp lệ. Vui lòng thử lại.\n");
       }
   }

   // Tạo chuỗi môn học đã chọn
   subjects[0] = '\0';
   for (int i = 0; i < num_subjects; i++) {
       if (selected_subjects[i]) {
           if (strlen(subjects) > 0) {
               strcat(subjects, ",");
           }
           strcat(subjects, subject_names[i]);
       }
   }

   // Giải phóng bộ nhớ
   for (int i = 0; i < num_subjects; i++) {
       free(subject_names[i]);
   }

   // Chọn số câu hỏi
   printf("\nChọn tổng số câu hỏi:\n");
   printf("1. 15\n2. 30\n3. 45\n4. 60\n");
   while (1) {
       printf("Nhập lựa chọn: ");
       if (!fgets(input, sizeof(input), stdin)) continue;
       input[strcspn(input, "\n")] = 0;
       
       int choice = atoi(input);
       if (choice >= 1 && choice <= 4) {
           switch (choice) {
               case 1: *num_questions_total = 15; break;
               case 2: *num_questions_total = 30; break;
               case 3: *num_questions_total = 45; break;
               case 4: *num_questions_total = 60; break;
           }
           break;
       }
       printf("Lựa chọn không hợp lệ. Vui lòng thử lại.\n");
   }

   // Chọn tỷ lệ độ khó
   printf("\nChọn tỷ lệ độ khó (Dễ:Trung bình:Khó):\n");
   printf("1. 3:0:0\n2. 0:3:0\n3. 0:0:3\n4. 1:2:0\n5. 2:1:0\n");
   printf("6. 1:0:2\n7. 2:0:1\n8. 0:1:2\n9. 0:2:1\n10. 1:1:1\n");
   
   while (1) {
       printf("Nhập lựa chọn: ");
       if (!fgets(input, sizeof(input), stdin)) continue;
       input[strcspn(input, "\n")] = 0;
       
       int choice = atoi(input);
       if (choice >= 1 && choice <= 10) {
           switch (choice) {
               case 1: *num_easy = *num_questions_total; *num_medium = 0; *num_hard = 0; break;
               case 2: *num_easy = 0; *num_medium = *num_questions_total; *num_hard = 0; break;
               case 3: *num_easy = 0; *num_medium = 0; *num_hard = *num_questions_total; break;
               case 4: *num_easy = *num_questions_total/3; *num_medium = *num_questions_total*2/3; *num_hard = 0; break;
               case 5: *num_easy = *num_questions_total*2/3; *num_medium = *num_questions_total/3; *num_hard = 0; break;
               case 6: *num_easy = *num_questions_total/3; *num_medium = 0; *num_hard = *num_questions_total*2/3; break;
               case 7: *num_easy = *num_questions_total*2/3; *num_medium = 0; *num_hard = *num_questions_total/3; break;
               case 8: *num_easy = 0; *num_medium = *num_questions_total/3; *num_hard = *num_questions_total*2/3; break;
               case 9: *num_easy = 0; *num_medium = *num_questions_total*2/3; *num_hard = *num_questions_total/3; break;
               case 10: *num_easy = *num_questions_total/3; *num_medium = *num_questions_total/3; *num_hard = *num_questions_total/3; break;
           }
           break;
       }
       printf("Lựa chọn không hợp lệ. Vui lòng thử lại.\n");
   }

   // Nhập thời gian
   while (1) {
       printf("Nhập thời gian làm bài (phút): ");
       if (!fgets(input, sizeof(input), stdin)) continue;
       input[strcspn(input, "\n")] = 0;
       
       *time_limit = atoi(input);
       if (*time_limit > 0) break;
       printf("Thời gian không hợp lệ. Vui lòng nhập số phút lớn hơn 0.\n");
   }
}

void print_subjects_menu(Client* client) {  
    char subjects_list[BUFFER_SIZE];
    char command[] = "GET_SUBJECTS";
    
    if (send_message(client, command) < 0) {
        printf("Lỗi khi gửi yêu cầu lấy danh sách môn học\n");
        return;
    }

    // Nhận danh sách môn học
    char buffer[BUFFER_SIZE];
    if (receive_message(client, buffer) > 0) {
        if (strncmp(buffer, "SUBJECTS|", 9) == 0) {
            char* subjects_str = buffer + 9;
            char* subject;
            int index = 1;
            
            printf("Chọn môn học:\n");
            // Tách và in từng môn học
            subject = strtok(subjects_str, ",");
            while (subject != NULL) {
                printf("%d. %s\n", index++, subject);
                subject = strtok(NULL, ",");
            }
        }
    }
}