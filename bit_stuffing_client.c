#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 9002
#define FLAG "01111110"

int main()
{
    int clientSocket;

    char data[500];
    char stuffed[1000];
    char frame[1200];

    struct sockaddr_in serverAddress;

    /* Create socket */
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    printf("Socket created successfully\n");

    /* Server address */
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
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

    /* Multiple frames */
    while (1)
    {
        printf("\nEnter binary data (0 to stop): ");

        scanf("%s", data);

        /* Stop */
        if (strcmp(data, "0") == 0)
        {
            send(clientSocket, "STOP", 4, 0);
            break;
        }

        int i;
        int j = 0;
        int count = 0;

        /*
           Bit stuffing
           Insert 0 after five consecutive 1s
        */

        for (i = 0; data[i] != '\0'; i++)
        {
            stuffed[j++] = data[i];

            if (data[i] == '1')
            {
                count++;

                if (count == 5)
                {
                    stuffed[j++] = '0';
                    count = 0;
                }
            }
            else
            {
                count = 0;
            }
        }

        stuffed[j] = '\0';

        /*
           Add starting and ending flags
        */

        strcpy(frame, FLAG);

        strcat(frame, stuffed);

        strcat(frame, FLAG);

        /* Send frame */
        send(clientSocket,
             frame,
             strlen(frame),
             0);

        printf("Original Data : %s\n", data);
        printf("Stuffed Frame : %s\n", frame);
    }

    close(clientSocket);

    return 0;
}
