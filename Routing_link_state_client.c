
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9002
#define MAX 10

int send_all(int sock, const void *data, size_t length)
{
    const char *ptr = data;
    size_t sent = 0;

    while (sent < length)
    {
        ssize_t n = send(sock, ptr + sent, length - sent, 0);

        if (n <= 0)
            return 0;

        sent += (size_t)n;
    }

    return 1;
}

int receive_all(int sock, void *data, size_t length)
{
    char *ptr = data;
    size_t received = 0;

    while (received < length)
    {
        ssize_t n = recv(sock, ptr + received,
                         length - received, 0);

        if (n <= 0)
            return 0;

        received += (size_t)n;
    }

    return 1;
}

int main(void)
{
    int sock;
    int n, source;
    int matrix[MAX][MAX];
    struct sockaddr_in server;
    unsigned int value;
    char result[4096];
    size_t pos = 0;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server,
                sizeof(server)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    printf("LINK STATE ROUTING - CLIENT\n");
    printf("Enter number of routers (maximum %d): ", MAX);

    if (scanf("%d", &n) != 1 || n < 1 || n > MAX)
    {
        printf("Invalid number of routers\n");
        close(sock);
        return 1;
    }

    printf("\nEnter the adjacency matrix\n");
    printf("Use 999999 when there is no direct link\n\n");

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("Cost from R%d to R%d: ", i + 1, j + 1);

            if (scanf("%d", &matrix[i][j]) != 1 ||
                matrix[i][j] < 0)
            {
                printf("Invalid link cost\n");
                close(sock);
                return 1;
            }
        }
    }

    printf("\nEnter source router (1 to %d): ", n);

    if (scanf("%d", &source) != 1 ||
        source < 1 || source > n)
    {
        printf("Invalid source router\n");
        close(sock);
        return 1;
    }

    value = htonl((unsigned int)n);

    if (!send_all(sock, &value, sizeof(value)))
        goto communication_error;

    value = htonl((unsigned int)(source - 1));

    if (!send_all(sock, &value, sizeof(value)))
        goto communication_error;

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            value = htonl((unsigned int)matrix[i][j]);

            if (!send_all(sock, &value, sizeof(value)))
                goto communication_error;
        }
    }

    printf("\nWaiting for routing table from server...\n\n");

    while (pos < sizeof(result) - 1)
    {
        char ch;

        if (!receive_all(sock, &ch, 1))
            goto communication_error;

        result[pos++] = ch;

        if (ch == '\0')
            break;
    }

    result[sizeof(result) - 1] = '\0';

    if (pos == 0 || result[pos - 1] != '\0')
    {
        printf("Response from server was too long\n");
        close(sock);
        return 1;
    }

    printf("%s\n", result);

    close(sock);
    return 0;

communication_error:
    printf("\nCommunication with server failed\n");
    close(sock);
    return 1;
}
