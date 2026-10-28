#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

int main()
{
    int server_socket;
    int client_socket;

    int n;
    int windowSize;

    int frame;
    int ack;

    int expectedFrame = 0;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

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
    server_addr.sin_port = htons(9002);

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
    read(client_socket,
         &windowSize,
         sizeof(windowSize));

    printf("Number of frames: %d\n", n);
    printf("Window size: %d\n", windowSize);

    printf("\n--- Receiving Frames ---\n");

    while(expectedFrame < n)
    {
        /*
           Receive frame
        */

        if(read(client_socket,
                &frame,
                sizeof(frame)) <= 0)
        {
            break;
        }

        printf("\nFrame received: %d\n", frame);

        /*
           Check whether this is
           the expected frame.
        */

        if(frame == expectedFrame)
        {
            printf("Correct frame received.\n");

            /*
               Ask user whether ACK
               should be sent.
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

                printf("ACK sent: %d\n", ack);

                expectedFrame++;
            }
            else
            {
                /*
                   ACK is intentionally lost.
                */

                printf("ACK lost for frame %d\n",
                       frame);

                /*
                   Do not increment expectedFrame.
                   Client will timeout and retransmit.
                */
            }
        }

        /*
           Duplicate frame
        */

        else if(frame < expectedFrame)
        {
            printf("Duplicate frame received: %d\n",
                   frame);

            /*
               Send ACK for the last
               correctly received frame.
            */

            ack = expectedFrame - 1;

            write(client_socket,
                  &ack,
                  sizeof(ack));

            printf("Duplicate ACK sent: %d\n",
                   ack);
        }

        /*
           Out-of-order frame
        */

        else
        {
            printf("Out-of-order frame received: %d\n",
                   frame);

            /*
               Send ACK for previous
               correctly received frame.
            */

            ack = expectedFrame - 1;

            if(ack >= 0)
            {
                write(client_socket,
                      &ack,
                      sizeof(ack));

                printf("ACK sent: %d\n", ack);
            }
        }
    }

    printf("\nAll frames received successfully.\n");

    close(client_socket);
    close(server_socket);

    return 0;
}
