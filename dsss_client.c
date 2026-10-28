
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 9000
#define MAX 100

int main()
{
    int clientSocket;
    int dataLength, chipLength;
    int data[MAX], chip[MAX], recovered[MAX];
    int i;

    struct sockaddr_in serverAddress;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr *)&serverAddress,
                sizeof(serverAddress)) < 0)
    {
        printf("Connection failed\n");
        close(clientSocket);
        return 1;
    }

    printf("Connected to server\n");

    printf("Enter number of data bits (1-%d): ", MAX);
    if (scanf("%d", &dataLength) != 1 ||
        dataLength < 1 || dataLength > MAX)
    {
        printf("Invalid data length\n");
        close(clientSocket);
        return 1;
    }

    printf("Enter data bits (0 or 1): ");
    for (i = 0; i < dataLength; i++)
    {
        if (scanf("%d", &data[i]) != 1 ||
            (data[i] != 0 && data[i] != 1))
        {
            printf("Invalid data bit\n");
            close(clientSocket);
            return 1;
        }
    }

    printf("Enter number of chip bits (1-%d): ", MAX);
    if (scanf("%d", &chipLength) != 1 ||
        chipLength < 1 || chipLength > MAX)
    {
        printf("Invalid chip length\n");
        close(clientSocket);
        return 1;
    }

    printf("Enter chip bits (0 or 1): ");
    for (i = 0; i < chipLength; i++)
    {
        if (scanf("%d", &chip[i]) != 1 ||
            (chip[i] != 0 && chip[i] != 1))
        {
            printf("Invalid chip bit\n");
            close(clientSocket);
            return 1;
        }
    }

    if (send(clientSocket, &dataLength, sizeof(int), 0)
        != sizeof(int) ||
        send(clientSocket, &chipLength, sizeof(int), 0)
        != sizeof(int) ||
        send(clientSocket, data, dataLength * sizeof(int), 0)
        != dataLength * sizeof(int) ||
        send(clientSocket, chip, chipLength * sizeof(int), 0)
        != chipLength * sizeof(int))
    {
        printf("Data sending failed\n");
        close(clientSocket);
        return 1;
    }

    if (recv(clientSocket, recovered,
             dataLength * sizeof(int), MSG_WAITALL)
        != dataLength * sizeof(int))
    {
        printf("Response reception failed\n");
        close(clientSocket);
        return 1;
    }

    printf("Recovered data from server: ");
    for (i = 0; i < dataLength; i++)
        printf("%d ", recovered[i]);

    printf("\n");

    close(clientSocket);

    return 0;
}
