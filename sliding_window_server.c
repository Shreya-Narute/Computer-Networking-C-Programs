#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

int main()
{
    int server_socket, client_socket;
    int frame;
    int ack;

    struct sockaddr_in server_addr, client_addr;

    /* Create TCP socket */
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    /* Allow reuse of port */
    int opt = 1;

    setsockopt(server_socket,
               SOL_SOCKET,
               SO_REUSEADDR,
               &opt,
               sizeof(opt));

    /* Server address */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(9001);

    /* Bind */
    if (bind(server_socket,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        printf("Failed to bind\n");
        close(server_socket);
        return 1;
    }

    /* Listen */
    listen(server_socket, 5);

    printf("Server waiting for connection...\n");

    /* Accept client */
    socklen_t client_length = sizeof(client_addr);

    client_socket = accept(server_socket,
                           (struct sockaddr *)&client_addr,
                           &client_length);

    if (client_socket < 0)
    {
        printf("Failed to accept connection\n");
        close(server_socket);
        return 1;
    }

    printf("Client connected.\n");

    /*
       Continuously receive frames
       and send ACKs.
    */
    while (1)
    {
        int result = read(client_socket,
                          &frame,
                          sizeof(frame));

        if (result <= 0)
            break;

        printf("Frame received: %d\n", frame);

        /* ACK = received frame number */
        ack = frame;

        write(client_socket,
              &ack,
              sizeof(ack));

        printf("ACK sent: %d\n", ack);
    }

    printf("\nServer finished.\n");

    close(client_socket);
    close(server_socket);

    return 0;
}
