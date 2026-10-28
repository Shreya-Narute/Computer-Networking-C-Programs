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
    int base = 0;
    int nextFrame = 0;

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
    server_addr.sin_port = htons(9002);

    /* Connect to server */
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
    write(client_socket, &windowSize, sizeof(windowSize));

    printf("\n--- Go-Back-N Transmission ---\n");

    while(base < n)
    {
        /*
           Send all frames present
           in the current window.
        */

        while(nextFrame < n &&
              nextFrame < base + windowSize)
        {
            write(client_socket,
                  &nextFrame,
                  sizeof(nextFrame));

            printf("Frame sent: %d\n", nextFrame);

            nextFrame++;
        }

        /*
           Wait for ACK.
           Timeout = 3 seconds.
        */

        fd_set readfds;
        struct timeval timeout;

        FD_ZERO(&readfds);
        FD_SET(client_socket, &readfds);

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

            printf("ACK received for frame: %d\n", ack);

            /*
               Go-Back-N uses cumulative ACK.
               If ACK = 2,
               frames 0,1,2 are accepted.
            */

            if(ack >= base)
            {
                base = ack + 1;
            }
        }

        /*
           Timeout
        */

        else if(result == 0)
        {
            printf("\n*** TIMEOUT ***\n");

            printf("Retransmitting from frame %d\n\n",
                   base);

            /*
               Go back to the oldest
               unacknowledged frame.
            */

            nextFrame = base;
        }

        /*
           Error
        */

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
