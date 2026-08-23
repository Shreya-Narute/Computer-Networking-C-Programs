#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int serverSocket, clientSocket;
    int bit;

    int chip[] = {1, -1, 1, -1};

    struct sockaddr_in serverAddress;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(serverSocket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    printf("Socket created successfully\n");

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if(bind(serverSocket,
            (struct sockaddr *)&serverAddress,
            sizeof(serverAddress)) < 0)
    {
        printf("Binding failed\n");
        return 1;
    }

    printf("Binding successful\n");

    listen(serverSocket, 3);

    printf("Waiting for client...\n");

    clientSocket = accept(serverSocket, NULL, NULL);

    if(clientSocket < 0)
    {
        printf("Connection failed\n");
        return 1;
    }

    printf("Client connected\n");

    recv(clientSocket, &bit, sizeof(bit), 0);

    printf("\nData bit received: %d\n", bit);

    printf("DSSS Spread Signal:\n");

    for(int i = 0; i < 4; i++)
    {
        if(bit == 1)
            printf("%d ", chip[i]);
        else
            printf("%d ", -chip[i]);
    }

    printf("\n");

    char message[] = "DSSS spreading completed";

    send(clientSocket, message, sizeof(message), 0);

    close(clientSocket);
    close(serverSocket);

    return 0;
}
