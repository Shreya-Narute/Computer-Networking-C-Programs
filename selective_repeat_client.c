#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

int main()
{
    int client_socket;

    int n, windowSize;
    int frame, ack;

    int acknowledged[100];

    int base = 0;
    int i;

    struct sockaddr_in server_addr;

    /* Create TCP socket */
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if(client_socket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    /* Server address */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(9003);

    /* Connect */
    if(connect(client_socket,
               (struct sockaddr *)&server_addr,
               sizeof(server_addr)) < 0)
    {
        printf("Connection failed\n");
        return 1;
    }

    printf("Connected to server\n");

    /* Input number of frames */
    printf("Enter number of frames: ");
    scanf("%d", &n);

    /* Input window size */
    printf("Enter window size: ");
    scanf("%d", &windowSize);

    /* Send n and window size */

    write(client_socket, &n, sizeof(n));

    write(client_socket,
          &windowSize,
          sizeof(windowSize));

    /* Initially no ACK received */

    for(i = 0; i < n; i++)
    {
        acknowledged[i] = 0;
    }

    printf("\n--- Selective Repeat Transmission ---\n");

    while(base < n)
    {
        /*
           Send frames inside current window
           which have not been acknowledged.
        */

        for(i = base;
            i < base + windowSize && i < n;
            i++)
        {
            if(acknowledged[i] == 0)
            {
                frame = i;

                write(client_socket,
                      &frame,
                      sizeof(frame));

                printf("Frame sent: %d\n",
                       frame);
            }
        }

        /*
           Wait for ACK
        */

        fd_set readfds;
        struct timeval timeout;

        FD_ZERO(&readfds);

        FD_SET(client_socket, &readfds);

        /*
           Timeout = 3 seconds
        */

        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        int result = select(client_socket + 1,
                            &readfds,
                            NULL,
                            NULL,
                            &timeout);

        /*
           ACK received
        */

        if(result > 0)
        {
            read(client_socket,
                 &ack,
                 sizeof(ack));

            /*
               -1 means all frames
               are received.
            */

            if(ack == -1)
            {
                break;
            }

            printf("ACK received for frame: %d\n",
                   ack);

            /*
               Mark only this frame
               as acknowledged.
            */

            acknowledged[ack] = 1;

            /*
               Slide window forward only
               over consecutively acknowledged
               frames.
            */

            while(base < n &&
                  acknowledged[base] == 1)
            {
                base++;
            }
        }

        /*
           Timeout
        */

        else if(result == 0)
        {
            printf("\n*** TIMEOUT ***\n");

            /*
               Selective Repeat:
               retransmit only frames
               which have not been ACKed.
            */

            printf("Retransmitting unacknowledged frames:\n");

            for(i = base;
                i < base + windowSize && i < n;
                i++)
            {
                if(acknowledged[i] == 0)
                {
                    frame = i;

                    write(client_socket,
                          &frame,
                          sizeof(frame));

                    printf("Retransmitted frame: %d\n",
                           frame);
                }
            }
        }

        else
        {
            printf("Select error\n");
            break;
        }
    }

    printf("\nAll frames transmitted successfully.\n");

    close(client_socket);

    return 0;
}
