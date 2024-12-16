// client/include/ui.h
#ifndef UI_H
#define UI_H

#include "client.h"
#include "exam.h"
#include <unistd.h>     // For STDIN_FILENO

void print_banner(void);
void print_main_menu(void);
void print_room_menu(int is_creator);
void handle_main_menu(Client* client);
void handle_room_menu(Client* client);
void clear_screen(void);
void print_error(const char* message);
void print_success(const char* message);

#endif // UI_H
