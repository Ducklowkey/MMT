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
    int waiting_for_answer = 0;
    int exam_started = 0;  // Flag để kiểm tra bắt đầu thi

    while (!exam_completed) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(client->socket, &readfds);

        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            print_error("Select error");
            break;
        }

        if (FD_ISSET(client->socket, &readfds)) {
            int valread = receive_message(client, buffer);
            if (valread <= 0) {
                print_error("Server disconnected");
                break;
            }

            if (strstr(buffer, "Exam has started") != NULL) {
                exam_started = 1;
                clear_screen();
                printf("\n%s", buffer);
            }
            else if (strstr(buffer, "Question") != NULL && exam_started) {
                printf("\n%s", buffer);
                waiting_for_answer = 1;
                printf("\nYour answer (A/B/C/D): ");
                fflush(stdout);
            }
            else if (strstr(buffer, "Exam completed") != NULL) {
                printf("\n%s", buffer);
                exam_completed = 1;
                printf("\nPress Enter to continue...");
                getchar();
                break;
            }
            else if (strstr(buffer, "Correct answer") != NULL || 
                     strstr(buffer, "Wrong answer") != NULL) {
                printf("%s", buffer);
            }
            else if (!exam_started) {
                printf("%s", buffer);
            }
        }

        if (waiting_for_answer && FD_ISSET(STDIN_FILENO, &readfds)) {
            char answer;
            if (scanf(" %c", &answer) == 1) {
                while (getchar() != '\n');  // Clear buffer
                
                answer = toupper(answer);
                if (answer >= 'A' && answer <= 'D') {
                    char cmd[BUFFER_SIZE];
                    snprintf(cmd, BUFFER_SIZE, "SUBMIT_ANSWER %c", answer);
                    if (send_message(client, cmd) > 0) {
                        printf("Answer submitted: %c\n", answer);
                        waiting_for_answer = 0;
                    } else {
                        print_error("Failed to send answer");
                    }
                } else {
                    print_error("Invalid answer. Please enter A, B, C, or D.");
                    printf("Your answer (A/B/C/D): ");
                    fflush(stdout);
                }
            }
        }
    }
}

