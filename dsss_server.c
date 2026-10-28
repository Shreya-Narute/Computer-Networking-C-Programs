#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 9000
#define MAX 100

int receive_all(int sock, void *buffer, int length)
{
    char *p = (char *)buffer;
    int received = 0;

    while (received < length)
    {
        int n = recv(sock, p + received, length - received, 0);

        if (n <= 0)
            return -1;

        received += n;
    }

    return received;
}

int send_all(int sock, void *buffer, int length)
{
    char *p = (char *)buffer;
    int sent = 0;

    while (sent < length)
    {
        int n = send(sock, p + sent, length - sent, 0);

        if (n <= 0)
            return -1;

        sent += n;
    }

    return sent;
}

int main()
{
    int serverSocket, clientSocket;
    int dataLength, chipLength;
    int data[MAX], chip[MAX];
    int spread[MAX * MAX], recovered[MAX];
    int i, j, k = 0;

    struct sockaddr_in serverAddress;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    int option = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
               &option, sizeof(option));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(serverSocket, (struct sockaddr *)&serverAddress,
             sizeof(serverAddress)) < 0)
    {
        perror("Binding failed");
        close(serverSocket);
        return 1;
    }

    printf("Binding successful\n");

    if (listen(serverSocket, 3) < 0)
    {
        perror("Listen failed");
        close(serverSocket);
        return 1;
    }

    printf("Waiting for client...\n");

    clientSocket = accept(serverSocket, NULL, NULL);

    if (clientSocket < 0)
    {
        perror("Accept failed");
        close(serverSocket);
        return 1;
    }

    printf("Client connected\n");

    if (receive_all(clientSocket, &dataLength, sizeof(int)) < 0 ||
        receive_all(clientSocket, &chipLength, sizeof(int)) < 0)
    {
        printf("Length reception failed\n");
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    if (dataLength < 1 || dataLength > MAX ||
        chipLength < 1 || chipLength > MAX)
    {
        printf("Invalid length\n");
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    if (receive_all(clientSocket, data,
                    dataLength * sizeof(int)) < 0 ||
        receive_all(clientSocket, chip,
                    chipLength * sizeof(int)) < 0)
    {
        printf("Data reception failed\n");
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    for (i = 0; i < dataLength; i++)
    {
        if (data[i] != 0 && data[i] != 1)
        {
            printf("Invalid data bit\n");
            close(clientSocket);
            close(serverSocket);
            return 1;
        }
    }

    for (i = 0; i < chipLength; i++)
    {
        if (chip[i] != 0 && chip[i] != 1)
        {
            printf("Invalid chip bit\n");
            close(clientSocket);
            close(serverSocket);
            return 1;
        }
    }

    printf("Original data: ");

    for (i = 0; i < dataLength; i++)
        printf("%d ", data[i]);

    printf("\nChip sequence: ");

    for (i = 0; i < chipLength; i++)
        printf("%d ", chip[i]);

    /* DSSS spreading using XOR */
    printf("\nSpread signal: ");

    for (i = 0; i < dataLength; i++)
    {
        for (j = 0; j < chipLength; j++)
        {
            spread[k] = data[i] ^ chip[j];
            printf("%d ", spread[k]);
            k++;
        }
    }

    /* Despreading using XOR */
    for (i = 0; i < dataLength; i++)
    {
        recovered[i] = spread[i * chipLength] ^ chip[0];

        for (j = 1; j < chipLength; j++)
        {
            if ((spread[i * chipLength + j] ^ chip[j])
                != recovered[i])
            {
                printf("\nDespreading verification failed\n");
                close(clientSocket);
                close(serverSocket);
                return 1;
            }
        }
    }

    printf("\nRecovered data: ");

    for (i = 0; i < dataLength; i++)
        printf("%d ", recovered[i]);

    printf("\n");

    if (send_all(clientSocket, recovered,
                 dataLength * sizeof(int)) < 0)
    {
        printf("Response sending failed\n");
    }

    close(clientSocket);
    close(serverSocket);

    return 0;
}
