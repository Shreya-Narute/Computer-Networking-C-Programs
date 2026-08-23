#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int serverSocket, clientSocket;
    int hops;
    int frequencies[] = {900, 905, 910, 915, 920};

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

    recv(clientSocket, &hops, sizeof(hops), 0);

    printf("\nFHSS Frequency Hopping Sequence:\n");

    for(int i = 0; i < hops; i++)
    {
        printf("Hop %d -> %d MHz\n",
               i + 1,
               frequencies[i % 5]);
    }

    char message[] = "FHSS sequence generated successfully";

    send(clientSocket, message, sizeof(message), 0);

    close(clientSocket);
    close(serverSocket);

    return 0;
}
