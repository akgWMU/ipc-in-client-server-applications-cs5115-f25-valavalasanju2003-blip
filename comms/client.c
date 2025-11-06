/*
  MULTI-CLIENT CLIENT PROGRAM USING UNIX DOMAIN SOCKETS

  This client:
    Connects to the server
    Prompts user for operations
    Sends arithmetic requests to server
    Receives result or error flag
    Runs in its own terminal
    Multiple clients can run at the same time
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "arith_socket"

// Same struct used in server
struct message {
    char operation[4];
    int operand1;
    int operand2;
};

int main() {
    int client_fd;
    struct sockaddr_un server_addr;

    // 1️⃣ Create socket
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("socket failed");
        exit(1);
    }

    // 2️⃣ Set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCKET_PATH);

    // 3️⃣ Connect to server
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        exit(1);
    }

    printf("✅ Connected to server.\n");

    while (1) {
        struct message msg;

        // Get operation from user
        printf("\nEnter operation (add/sub/mul/div) or 'exit': ");
        scanf("%s", msg.operation);

        if (strcmp(msg.operation, "exit") == 0) {
            write(client_fd, &msg, sizeof(msg));
            printf("👋 Client exiting.\n");
            break;
        }

        // Get operands
        printf("Enter operand1: ");
        scanf("%d", &msg.operand1);

        printf("Enter operand2: ");
        scanf("%d", &msg.operand2);

        // Send full request struct to server
        write(client_fd, &msg, sizeof(msg));

        // Receive reply
        int valid, result;

        read(client_fd, &valid, sizeof(valid));
        read(client_fd, &result, sizeof(result));

        if (!valid)
            printf("❌ Error from server (invalid op or divide-by-zero)\n");
        else
            printf("✅ Result: %d\n", result);
    }

    close(client_fd);
    return 0;
}
