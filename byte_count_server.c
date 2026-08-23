#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int serverSocket, clientSocket;
    int count;
    char frame[100];

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
    serverAddress.sin_port = htons(9000);
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
        /* Receive byte count */
        if (recv(clientSocket, &count, sizeof(count), 0) <= 0)
            break;

        /* 0 means client wants to stop */
        if (count == 0)
            break;

        /* Receive frame data */
        if (recv(clientSocket, frame, count, 0) <= 0)
            break;

        frame[count] = '\0';

        printf("\nReceived Frame: %d%s\n", count, frame);
        printf("Byte Count    : %d\n", count);
        printf("Data          : %s\n", frame);
    }

    printf("\nConnection closed.\n");

    close(clientSocket);
    close(serverSocket);

    return 0;
}
