/*

  This server:
     Creates an AF_UNIX socket (local IPC)
     Allows multiple clients to connect simultaneously
     Receives arithmetic requests from each client
     Sends back computation results
     Handles errors like divide-by-zero
     Uses fork() to handle each client separately
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "arith_socket"

// Structure for arithmetic messages
struct message {
    char operation[4];   // add, sub, mul, div
    int operand1;
    int operand2;
};

int main() {
    int server_fd, client_fd;
    struct sockaddr_un server_addr;

    // 1️⃣ Create a UNIX Domain Socket
    // AF_UNIX → IPC on same machine
    // SOCK_STREAM → reliable stream like TCP
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket creation failed");
        exit(1);
    }

    // Ensure old socket file is removed
    unlink(SOCKET_PATH);

    // 2️⃣ Configure socket address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCKET_PATH);

    // 3️⃣ Bind the socket to a file path
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(1);
    }

    // 4️⃣ Listen for incoming clients
    if (listen(server_fd, 5) < 0) {
        perror("listen failed");
        exit(1);
    }

    printf("✅ Server started. Waiting for clients...\n");

    while (1) {
        // 5️⃣ Accept new client connections
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }

        printf("🔔 New client connected!\n");

        // 6️⃣ Fork a new process to handle each client
        pid_t pid = fork();

        if (pid == 0) {
            // ---------- CHILD PROCESS: handles ONE client ----------

            close(server_fd); // Child doesn't need listening socket

            while (1) {
                struct message msg;
                int n = read(client_fd, &msg, sizeof(msg));

                if (n <= 0) {
                    printf("❌ Client disconnected.\n");
                    break;
                }

                // If client wants to exit
                if (strcmp(msg.operation, "exit") == 0) {
                    printf("Client requested exit.\n");
                    break;
                }

                printf("📩 Server received: %s(%d, %d)\n",
                       msg.operation, msg.operand1, msg.operand2);

                int result = 0, valid = 1;

                // Perform operation
                if (strcmp(msg.operation, "add") == 0)
                    result = msg.operand1 + msg.operand2;

                else if (strcmp(msg.operation, "sub") == 0)
                    result = msg.operand1 - msg.operand2;

                else if (strcmp(msg.operation, "mul") == 0)
                    result = msg.operand1 * msg.operand2;

                else if (strcmp(msg.operation, "div") == 0) {
                    if (msg.operand2 == 0) {
                        valid = 0;
                        printf("❌ Divide by zero error.\n");
                    } else {
                        result = msg.operand1 / msg.operand2;
                    }
                } else {
                    valid = 0;
                    printf("❌ Invalid operation.\n");
                }

                // Send result back:
                // First send valid flag (1=ok, 0=error)
                write(client_fd, &valid, sizeof(valid));
                // Then send result value
                write(client_fd, &result, sizeof(result));

                printf("✅ Sent result: %d\n", result);
            }

            close(client_fd);
            exit(0);
        }

        // PARENT keeps listening – close client_fd copy
        close(client_fd);
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}
