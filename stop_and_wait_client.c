#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main()
{
    int client_socket;
    int n, frame, ack;

    struct sockaddr_in server_addr;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(9001);

    // Connect to server
    if (connect(client_socket,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        printf("Connection failed\n");
        return 1;
    }

    printf("Connected to server\n");

    // Enter number of frames
    printf("Enter number of frames: ");
    scanf("%d", &n);

    // Send number of frames
    write(client_socket, &n, sizeof(n));

    // Stop-and-Wait
    for (frame = 0; frame < n; frame++)
    {
        // Send one frame
        write(client_socket, &frame, sizeof(frame));

        printf("Frame sent: %d\n", frame);

        // Wait for ACK
        read(client_socket, &ack, sizeof(ack));

        printf("ACK received: %d\n", ack);
    }

    printf("\nAll frames sent successfully.\n");

    close(client_socket);

    return 0;
}
