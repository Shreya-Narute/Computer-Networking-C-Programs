#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void calculate_crc(int code[], int n, int generator[], int g)
{
    int i, j;

    for(i = 0; i < n; i++)
    {
        if(code[i] == 1)
        {
            for(j = 0; j < g; j++)
                code[i + j] = code[i + j] ^ generator[j];
        }
    }
}

int main()
{
    int serverSocket, clientSocket;

    int generator[100];
    int code[200];

    int n, g;
    int i;

    struct sockaddr_in server, client;
    socklen_t clientLength = sizeof(client);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(serverSocket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(9001);

    if(bind(serverSocket,
            (struct sockaddr *)&server,
            sizeof(server)) < 0)
    {
        printf("Bind failed\n");
        close(serverSocket);
        return 1;
    }

    listen(serverSocket, 5);

    printf("Server waiting for client...\n");

    clientSocket = accept(serverSocket,
                          (struct sockaddr *)&client,
                          &clientLength);

    if(clientSocket < 0)
    {
        printf("Accept failed\n");
        close(serverSocket);
        return 1;
    }

    /* Receive data length */
    recv(clientSocket, &n, sizeof(n), 0);

    /* Receive generator length */
    recv(clientSocket, &g, sizeof(g), 0);

    /* Receive generator */
    recv(clientSocket,
         generator,
         sizeof(int) * g,
         0);

    /* Receive complete codeword */
    recv(clientSocket,
         code,
         sizeof(int) * (n + g - 1),
         0);

    printf("\nReceived Codeword: ");

    for(i = 0; i < n + g - 1; i++)
        printf("%d", code[i]);

    printf("\n");

    printf("Generator: ");

    for(i = 0; i < g; i++)
        printf("%d", generator[i]);

    printf("\n");

    /* Perform CRC division on received codeword */
    calculate_crc(code, n, generator, g);

    printf("\nReceiver Remainder: ");

    for(i = n; i < n + g - 1; i++)
        printf("%d", code[i]);

    /* Check remainder */
    int error = 0;

    for(i = n; i < n + g - 1; i++)
    {
        if(code[i] != 0)
        {
            error = 1;
            break;
        }
    }

    if(error == 0)
        printf("\n\nNo Error Detected\n");
    else
        printf("\n\nError Detected\n");

    close(clientSocket);
    close(serverSocket);

    return 0;
}
