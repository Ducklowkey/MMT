// client/src/main.c
#include "../include/client.h"
#include "../include/ui.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    Client* client = malloc(sizeof(Client));
    if (!client) {
        fprintf(stderr, "Failed to allocate memory for client\n");
        return EXIT_FAILURE;
    }

    // Initialize client
    client->socket = -1;
    client->is_authenticated = 0;
    client->current_room = -1;
    client->is_room_creator = 0;

    print_banner();

    // Connect to server
    client->socket = connect_to_server("127.0.0.1", PORT);
    if (client->socket < 0) {
        free(client);
        return EXIT_FAILURE;
    }

    // Handle authentication first
    if (!handle_authentication(client)) {
        disconnect_from_server(client);
        free(client);
        return EXIT_FAILURE;
    }

    printf("\033[1;32mXác thực thành công \033[0m\n");

    // Main program loop
    handle_main_menu(client);

    // Cleanup
    disconnect_from_server(client);
    free(client);
    return EXIT_SUCCESS;
}
