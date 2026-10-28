#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int clientSocket;
    int bit;
    char response[256];

    struct sockaddr_in serverAddress;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(clientSocket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    printf("Socket created successfully\n");

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if(connect(clientSocket,
               (struct sockaddr *)&serverAddress,
               sizeof(serverAddress)) < 0)
    {
        printf("Connection failed\n");
        return 1;
    }

    printf("Connection established\n");

    printf("Enter data bit (0 or 1): ");
    scanf("%d", &bit);

    send(clientSocket, &bit, sizeof(bit), 0);

    recv(clientSocket, response, sizeof(response), 0);

    printf("Server: %s\n", response);

    close(clientSocket);

    return 0;
}
