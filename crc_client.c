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
    int clientSocket;

    int data[100];
    int generator[100];
    int code[200];
    int temp[200];

    int n, g;
    int i;

    struct sockaddr_in server;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(clientSocket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(9001);

    if(connect(clientSocket,
               (struct sockaddr *)&server,
               sizeof(server)) < 0)
    {
        printf("Connection failed\n");
        close(clientSocket);
        return 1;
    }

    printf("Connected to server\n");

    printf("Enter number of data bits: ");
    scanf("%d", &n);

    printf("Enter data bits:\n");

    for(i = 0; i < n; i++)
        scanf("%d", &data[i]);

    printf("Enter number of generator bits: ");
    scanf("%d", &g);

    printf("Enter generator bits:\n");

    for(i = 0; i < g; i++)
        scanf("%d", &generator[i]);

    /* Create data + zeros */
    for(i = 0; i < n; i++)
        code[i] = data[i];

    for(i = n; i < n + g - 1; i++)
        code[i] = 0;

    /* Keep a copy for CRC calculation */
    for(i = 0; i < n + g - 1; i++)
        temp[i] = code[i];

    /* Calculate CRC */
    calculate_crc(temp, n, generator, g);

    printf("\nCRC Remainder: ");

    for(i = n; i < n + g - 1; i++)
        printf("%d", temp[i]);

    /* Create final codeword = data + CRC */
    for(i = 0; i < n; i++)
        code[i] = data[i];

    for(i = 0; i < g - 1; i++)
        code[n + i] = temp[n + i];

    printf("\nTransmitted Codeword: ");

    for(i = 0; i < n + g - 1; i++)
        printf("%d", code[i]);

    printf("\n");

    /* Send information to server */
    send(clientSocket, &n, sizeof(n), 0);
    send(clientSocket, &g, sizeof(g), 0);

    send(clientSocket,
         generator,
         sizeof(int) * g,
         0);

    send(clientSocket,
         code,
         sizeof(int) * (n + g - 1),
         0);

    printf("\nCodeword sent to server.\n");

    close(clientSocket);

    return 0;
}
