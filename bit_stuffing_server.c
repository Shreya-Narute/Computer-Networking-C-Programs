#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 9002
#define FLAG "01111110"

int main()
{
    int serverSocket, clientSocket;
    char frame[1000];
    int bytes;

    struct sockaddr_in serverAddress;

    /* Create TCP socket */
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    printf("Socket created successfully\n");

    /* Server address */
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    /* Bind */
    if (bind(serverSocket,
             (struct sockaddr *)&serverAddress,
             sizeof(serverAddress)) < 0)
    {
        perror("Binding failed");
        close(serverSocket);
        return 1;
    }

    printf("Binding successful\n");

    /* Listen */
    listen(serverSocket, 5);

    printf("Waiting for client...\n");

    /* Accept client */
    clientSocket = accept(serverSocket, NULL, NULL);

    if (clientSocket < 0)
    {
        perror("Accept failed");
        close(serverSocket);
        return 1;
    }

    printf("Client connected successfully\n");

    /* Receive multiple frames */
    while (1)
    {
        bytes = recv(clientSocket, frame, sizeof(frame) - 1, 0);

        if (bytes <= 0)
            break;

        frame[bytes] = '\0';

        /* Stop condition */
        if (strcmp(frame, "STOP") == 0)
            break;

        printf("\nReceived Frame:\n");
        printf("%s\n", frame);

        /* Find data between flags */
        char *start = strstr(frame, FLAG);

        if (start == NULL)
        {
            printf("Invalid frame\n");
            continue;
        }

        start += 8;

        char *end = strstr(start, FLAG);

        if (end == NULL)
        {
            printf("Invalid frame\n");
            continue;
        }

        int length = end - start;

        char stuffedData[1000];

        strncpy(stuffedData, start, length);
        stuffedData[length] = '\0';

        printf("Stuffed Data:\n");
        printf("%s\n", stuffedData);

        /* Bit destuffing */
        char destuffedData[1000];

        int i;
        int j = 0;
        int count = 0;

        for (i = 0; stuffedData[i] != '\0'; i++)
        {
            destuffedData[j++] = stuffedData[i];

            if (stuffedData[i] == '1')
            {
                count++;

                if (count == 5)
                {
                    /*
                       Skip stuffed 0
                    */
                    i++;
                    count = 0;
                }
            }
            else
            {
                count = 0;
            }
        }

        destuffedData[j] = '\0';

        printf("After Destuffing:\n");
        printf("%s\n", destuffedData);
    }

    close(clientSocket);
    close(serverSocket);

    return 0;
}
