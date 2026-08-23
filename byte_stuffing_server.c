#include <stdio.h>
#include <string.h>
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

    struct sockaddr_in serverAddress;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket < 0)
    {
        perror("Socket creation failed");
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
        perror("Binding failed");
        close(serverSocket);
        return 1;
    }

    printf("Binding successful\n");

    listen(serverSocket, 5);

    printf("Waiting for client...\n");

    clientSocket = accept(serverSocket, NULL, NULL);

    if (clientSocket < 0)
    {
        perror("Accept failed");
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

        /* Stop condition */
        if (strcmp(frame, "STOP") == 0)
            break;

        printf("\nReceived Stuffed Frame : %s\n", frame);

        /* Remove starting and ending FLAG */
        int i;
        int outputIndex = 0;
        char data[200];

        for (i = 1; i < bytes - 1; i++)
        {
            if (frame[i] == ESCAPE)
            {
                i++;
                data[outputIndex++] = frame[i];
            }
            else
            {
                data[outputIndex++] = frame[i];
            }
        }

        data[outputIndex] = '\0';

        printf("After Destuffing       : %s\n", data);
    }

    close(clientSocket);
    close(serverSocket);

    return 0;
}
