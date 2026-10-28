
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 9001
#define FLAG 'F'
#define ESCAPE 'E'

int main()
{
    int serverSocket, clientSocket;
    char frame[200];
    char data[200];

    struct sockaddr_in serverAddress;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    printf("Socket created successfully\n");

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(serverSocket,
             (struct sockaddr *)&serverAddress,
             sizeof(serverAddress)) < 0)
    {
        printf("Binding failed\n");
        close(serverSocket);
        return 1;
    }

    printf("Binding successful\n");

    if (listen(serverSocket, 5) < 0)
    {
        printf("Listen failed\n");
        close(serverSocket);
        return 1;
    }

    printf("Waiting for client...\n");

    clientSocket = accept(serverSocket, NULL, NULL);

    if (clientSocket < 0)
    {
        printf("Accept failed\n");
        close(serverSocket);
        return 1;
    }

    printf("Client connected successfully\n");

    while (1)
    {
        int bytes = recv(clientSocket, frame, sizeof(frame) - 1, 0);

        if (bytes <= 0)
            break;

        frame[bytes] = '\0';

        /* Check STOP manually */
        if (bytes == 4 &&
            frame[0] == 'S' &&
            frame[1] == 'T' &&
            frame[2] == 'O' &&
            frame[3] == 'P')
        {
            break;
        }

        printf("\nReceived Stuffed Frame: %s\n", frame);

        int i;
        int outputIndex = 0;

        /* Remove starting FLAG and ending FLAG */
        for (i = 1; i < bytes - 1; i++)
        {
            if (frame[i] == ESCAPE)
            {
                i++;

                if (i < bytes - 1)
                    data[outputIndex++] = frame[i];
            }
            else
            {
                data[outputIndex++] = frame[i];
            }
        }

        data[outputIndex] = '\0';

        printf("After Destuffing: %s\n", data);
    }

    close(clientSocket);
    close(serverSocket);

    return 0;
}

