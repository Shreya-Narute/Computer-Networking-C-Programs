#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{
    int clientSocket;
    int data[4];
    int hamming[7];

    struct sockaddr_in server;

    /* Create socket */
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(clientSocket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    /* Server details */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(9001);

    /* Connect */
    if(connect(clientSocket,
               (struct sockaddr *)&server,
               sizeof(server)) < 0)
    {
        printf("Connection failed\n");
        close(clientSocket);
        return 1;
    }

    printf("Connected to server\n");

    /* Input data bits */
    printf("Enter 4 data bits:\n");

    for(int i = 0; i < 4; i++)
        scanf("%d", &data[i]);

    /* Send data to server */
    send(clientSocket, data, sizeof(data), 0);

    printf("\nData sent to server.\n");

    /* Receive Hamming code */
    recv(clientSocket, hamming, sizeof(hamming), 0);

    printf("\nReceived Hamming Code: ");

    for(int i = 0; i < 7; i++)
        printf("%d", hamming[i]);

    printf("\n");

    close(clientSocket);

    return 0;
}
