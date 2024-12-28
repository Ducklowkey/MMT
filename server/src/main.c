#include "../include/server.h"
#include "../include/exam.h"
#include "../include/database.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

volatile sig_atomic_t server_running = 1;

void handle_signal(int sig) {
    if (sig == SIGINT) {
        printf("\nReceived shutdown signal. Cleaning up...\n");
        server_running = 0;  
    }
}

int main() {
    Server* server = create_server();
    if (!server) {
        fprintf(stderr, "Failed to create server\n");
        return EXIT_FAILURE;
    }

    // Thiết lập signal handler
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error setting up signal handler");
        destroy_server(server);
        return EXIT_FAILURE;
    }

    // Load questions và khởi động server
    load_questions();
    printf("Server starting on port %d...\n", PORT);

    server->running = 1;
    
    // Thêm vòng lặp kiểm tra để đảm bảo server dừng
    while (server_running) {
        start_server(server);
        if (!server_running) {
            server->running = 0;
            break;
        }
    }

    // Cleanup
    printf("Shutting down server...\n");
    destroy_server(server);
    printf("Server shutdown complete\n");
    
    return EXIT_SUCCESS;
}