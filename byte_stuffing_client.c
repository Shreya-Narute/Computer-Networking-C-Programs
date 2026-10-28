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
    int clientSocket;

    char data[100];
    char frame[200];

    struct sockaddr_in serverAddress;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    printf("Socket created successfully\n");

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(clientSocket,
                (struct sockaddr *)&serverAddress,
                sizeof(serverAddress)) < 0)
    {
        perror("Connection failed");
        close(clientSocket);
        return 1;
    }

    printf("Connection established\n");

    while (1)
    {
        printf("\nEnter data (0 to stop): ");

        fgets(data, sizeof(data), stdin);

        data[strcspn(data, "\n")] = '\0';

        if (strcmp(data, "0") == 0)
        {
            send(clientSocket, "STOP", 4, 0);
            break;
        }

        if (strlen(data) == 0)
            continue;

        int j = 0;
        int i;

        /* Starting FLAG */
        frame[j++] = FLAG;

        /* Byte stuffing */
        for (i = 0; data[i] != '\0'; i++)
        {
            if (data[i] == FLAG || data[i] == ESCAPE)
            {
                frame[j++] = ESCAPE;
            }

            frame[j++] = data[i];
        }

        /* Ending FLAG */
        frame[j++] = FLAG;

        frame[j] = '\0';

        send(clientSocket, frame, j, 0);

        printf("Stuffed Frame : %s\n", frame);
    }

    close(clientSocket);

    return 0;
}
