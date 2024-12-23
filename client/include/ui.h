// client/include/ui.h
#ifndef UI_H
#define UI_H

#include "client.h"
#include "exam.h"
#include <unistd.h>     // For STDIN_FILENO

void print_banner(void);
void print_main_menu(void);
void print_room_menu(int is_creator);
void clear_screen(void);
void print_error(const char* message);
void print_success(const char* message);
void handle_main_menu(Client* client);
void handle_room_menu(Client* client);
void handle_add_question(Client* client);
void print_add_question_menu(void);
void print_training_menu(void);
void handle_training_menu(Client* client);
void handle_training_setup(Client* client);

#endif // UI_H