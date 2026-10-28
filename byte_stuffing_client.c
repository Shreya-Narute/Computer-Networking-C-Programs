
#include <stdio.h>
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
    char ch;

    struct sockaddr_in serverAddress;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket < 0)
    {
        printf("Socket creation failed\n");
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
        printf("Connection failed\n");
        close(clientSocket);
        return 1;
    }

    printf("Connection established\n");

    while (1)
    {
        int n = 0;
        int inputResult;

        printf("\nEnter data (0 to stop): ");
        fflush(stdout);

        /* Read input one character at a time */
        while (n < 99)
        {
            inputResult = read(STDIN_FILENO, &ch, 1);

            if (inputResult <= 0)
            {
                close(clientSocket);
                return 0;
            }

            if (ch == '\n')
                break;

            if (ch == '\r')
                continue;

            data[n++] = ch;
        }

        data[n] = '\0';

        /* Check stop condition manually */
        if (n == 1 && data[0] == '0')
        {
            char stopMessage[4] = {'S', 'T', 'O', 'P'};
            send(clientSocket, stopMessage, 4, 0);
            break;
        }

        if (n == 0)
            continue;

        int j = 0;
        int i;

        /* Starting FLAG */
        frame[j++] = FLAG;

        /* Perform byte stuffing */
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

        printf("Stuffed Frame: %s\n", frame);
    }

    close(clientSocket);

    return 0;
}
