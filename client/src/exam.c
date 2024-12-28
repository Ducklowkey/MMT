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

    while (!exam_completed) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(client->socket, &readfds);

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int activity = select(max_fd + 1, &readfds, NULL, NULL, &tv);

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

            printf("\n%s", buffer);

            if (strstr(buffer, "Exam completed") != NULL) {
                exam_completed = 1;
                break;
            }
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            char input[10];
            if (fgets(input, sizeof(input), stdin)) {
                input[strcspn(input, "\n")] = 0;

                if (strcmp(input, "TIME") == 0) {
                    send_message(client, "TIME");
                }
                else if (strcmp(input, "SUBMIT") == 0) {
                    send_message(client, "SUBMIT");
                }
                else if (strlen(input) == 1) {
                    char answer = input[0];
                    if (answer >= 'a' && answer <= 'd') answer -= 32;
                    if (answer >= 'A' && answer <= 'D') {
                        char cmd[BUFFER_SIZE];
                        snprintf(cmd, BUFFER_SIZE, "SUBMIT_ANSWER %c", answer);
                        send_message(client, cmd);
                    } else {
                        printf("Invalid answer. Please enter A, B, C, or D\n");
                    }
                }
            }
        }
    }
}