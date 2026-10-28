
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int clientSocket;
    int count;
    char frame[100];

    struct sockaddr_in serverAddress;

    /* Create TCP socket */
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    printf("Socket created successfully\n");

    /* Server address */
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    /* Connect */
    if (connect(clientSocket,
                (struct sockaddr *)&serverAddress,
                sizeof(serverAddress)) < 0)
    {
        perror("Connection failed");
        close(clientSocket);
        return 1;
    }

    printf("Connection established\n");

    /* Send multiple frames */
    while (1)
    {
        printf("\nEnter frame (0 to stop): ");

        /*
           fgets() allows multiple words
           Example: HELLO WORLD
        */
        fgets(frame, sizeof(frame), stdin);

        /* Remove newline */
        frame[strcspn(frame, "\n")] = '\0';

        /* Stop */
        if (strcmp(frame, "0") == 0)
        {
            count = 0;

            send(clientSocket,
                 &count,
                 sizeof(count),
                 0);

            break;
        }

        /* Ignore empty input */
        if (strlen(frame) == 0)
            continue;

        /* Byte count */
        count = strlen(frame);

        /* Send byte count */
        send(clientSocket,
             &count,
             sizeof(count),
             0);

        /* Send frame data */
        send(clientSocket,
             frame,
             count,
             0);

        printf("Sent Frame: %d%s\n", count, frame);
    }

    close(clientSocket);

    return 0;
}
