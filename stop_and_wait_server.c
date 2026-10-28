
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main()
{
    int server_socket, client_socket;
    int n, frame, ack;

    struct sockaddr_in server_addr, client_addr;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(9001);

    // Bind
    if (bind(server_socket,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        printf("Bind failed\n");
        return 1;
    }

    // Listen
    listen(server_socket, 5);

    printf("Server waiting for connection...\n");

    // Accept client
    socklen_t client_length = sizeof(client_addr);

    client_socket = accept(server_socket,
                           (struct sockaddr *)&client_addr,
                           &client_length);

    if (client_socket < 0)
    {
        printf("Accept failed\n");
        return 1;
    }

    printf("Client connected\n");

    // Receive number of frames
    read(client_socket, &n, sizeof(n));

    printf("Number of frames: %d\n", n);

    // Stop-and-Wait
    for (int i = 0; i < n; i++)
    {
        // Receive frame
        read(client_socket, &frame, sizeof(frame));

        printf("Frame received: %d\n", frame);

        // Send ACK
        ack = frame;

        write(client_socket, &ack, sizeof(ack));

        printf("ACK sent: %d\n", ack);
    }

    printf("\nAll frames received successfully.\n");

    close(client_socket);
    close(server_socket);

    return 0;
}








