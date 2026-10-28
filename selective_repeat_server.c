#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

int main()
{
    int server_socket, client_socket;

    int n, windowSize;
    int frame, ack;

    int received[100];

    int i;

    struct sockaddr_in server_addr, client_addr;

    /* Create TCP socket */
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if(server_socket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    /* Server address */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(9003);

    /* Bind */
    if(bind(server_socket,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)) < 0)
    {
        printf("Failed to bind\n");
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

    if(client_socket < 0)
    {
        printf("Accept failed\n");
        return 1;
    }

    printf("Client connected\n");

    /* Receive number of frames */
    read(client_socket, &n, sizeof(n));

    /* Receive window size */
    read(client_socket, &windowSize, sizeof(windowSize));

    printf("Number of frames: %d\n", n);
    printf("Window size: %d\n", windowSize);

    /* Initially no frame is received */
    for(i = 0; i < n; i++)
    {
        received[i] = 0;
    }

    printf("\n--- Selective Repeat Receiving ---\n");

    while(1)
    {
        /*
           Receive frame number
        */

        if(read(client_socket,
                &frame,
                sizeof(frame)) <= 0)
        {
            break;
        }

        printf("\nFrame received: %d\n", frame);

        /*
           If frame is new
        */

        if(received[frame] == 0)
        {
            received[frame] = 1;

            printf("Frame %d accepted.\n", frame);
        }
        else
        {
            printf("Duplicate frame received: %d\n",
                   frame);
        }

        /*
           Ask whether ACK should be sent.
        */

        char choice;

        printf("Send ACK for frame %d? (y/n): ",
               frame);

        scanf(" %c", &choice);

        if(choice == 'y' || choice == 'Y')
        {
            ack = frame;

            write(client_socket,
                  &ack,
                  sizeof(ack));

            printf("ACK sent for frame: %d\n",
                   ack);
        }
        else
        {
            /*
               ACK is intentionally lost.
            */

            printf("ACK lost for frame: %d\n",
                   frame);
        }

        /*
           Check whether all frames
           have been received.
        */

        int allReceived = 1;

        for(i = 0; i < n; i++)
        {
            if(received[i] == 0)
            {
                allReceived = 0;
                break;
            }
        }

        if(allReceived == 1)
        {
            printf("\nAll frames received successfully.\n");

            /*
               Send -1 to tell client
               transmission is complete.
            */

            ack = -1;

            write(client_socket,
                  &ack,
                  sizeof(ack));

            break;
        }
    }

    close(client_socket);
    close(server_socket);

    return 0;
}
