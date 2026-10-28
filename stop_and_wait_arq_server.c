#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main()
{
    int server_socket, client_socket;
    int n, frame, ack;
    int expected_frame = 0;
    int choice;

    struct sockaddr_in server_addr, client_addr;

    // Create TCP socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    // Allow port reuse
    int opt = 1;

    setsockopt(server_socket,
               SOL_SOCKET,
               SO_REUSEADDR,
               &opt,
               sizeof(opt));

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

    // Stop-and-Wait ARQ
    while (expected_frame < n)
    {
        // Receive frame
        read(client_socket, &frame, sizeof(frame));

        printf("\nFrame received: %d\n", frame);

        // Check expected frame
        if (frame == expected_frame)
        {
            printf("Expected frame received.\n");

            printf("Send ACK for frame %d? (1=Yes, 0=No): ",
                   frame);

            scanf("%d", &choice);

            if (choice == 1)
            {
                ack = frame;

                write(client_socket, &ack, sizeof(ack));

                printf("ACK sent: %d\n", ack);

                expected_frame++;
            }
            else
            {
                // Simulate ACK loss
                printf("ACK lost for frame: %d\n", frame);
            }
        }
        else
        {
            // Duplicate frame due to retransmission
            printf("Duplicate frame received: %d\n", frame);

            ack = frame;

            write(client_socket, &ack, sizeof(ack));

            printf("ACK resent: %d\n", ack);
        }
    }

    printf("\nAll frames received successfully.\n");

    close(client_socket);
    close(server_socket);

    return 0;
}
