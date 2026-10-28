
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX 10
#define INF 999999
#define PORT 9002

int receive_all(int fd, void *buffer, size_t length)
{
    char *p = buffer;
    size_t received = 0;

    while (received < length)
    {
        ssize_t n = recv(fd, p + received,
                         length - received, 0);

        if (n <= 0)
            return 0;

        received += (size_t)n;
    }

    return 1;
}

int send_all(int fd, const void *buffer, size_t length)
{
    const char *p = buffer;
    size_t sent = 0;

    while (sent < length)
    {
        ssize_t n = send(fd, p + sent,
                         length - sent, 0);

        if (n <= 0)
            return 0;

        sent += (size_t)n;
    }

    return 1;
}

int main(void)
{
    int sock;
    struct sockaddr_in server;

    int n, source;
    int graph[MAX][MAX];
    char result[4096] = {0};

    printf("Enter number of routers (2-%d): ", MAX);
    if (scanf("%d", &n) != 1 || n < 2 || n > MAX)
    {
        printf("Invalid number of routers.\n");
        return 1;
    }

    printf("\nEnter adjacency matrix.\n");
    printf("Use 0 for same router, %d for no direct link,\n",
           INF);
    printf("and positive numbers for link costs.\n");

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("Cost from R%d to R%d: ", i + 1, j + 1);

            if (scanf("%d", &graph[i][j]) != 1 ||
                graph[i][j] < 0 || graph[i][j] > INF)
            {
                printf("Invalid link cost.\n");
                return 1;
            }
        }
    }

    printf("Enter source router (1-%d): ", n);
    if (scanf("%d", &source) != 1 ||
        source < 1 || source > n)
    {
        printf("Invalid source router.\n");
        return 1;
    }

    source--;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    if (connect(sock, (struct sockaddr *)&server,
                sizeof(server)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    printf("\nConnected to routing server.\n");

    unsigned int header[2];
    header[0] = htonl((unsigned int)n);
    header[1] = htonl((unsigned int)source);

    if (!send_all(sock, header, sizeof(header)))
        goto communication_error;

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            unsigned int value =
                htonl((unsigned int)graph[i][j]);

            if (!send_all(sock, &value, sizeof(value)))
                goto communication_error;
        }
    }

    if (!receive_all(sock, result, 1))
        goto communication_error;

    /*
       Receive the result string one byte at a time
       until its terminating null character arrives.
    */
    int pos = 1;

    while (result[pos - 1] != '\0' && pos < (int)sizeof(result))
    {
        if (!receive_all(sock, &result[pos], 1))
            goto communication_error;

        pos++;
    }

    printf("%s\n", result);

    close(sock);
    return 0;

communication_error:
    printf("Communication with server failed.\n");
    close(sock);
    return 1;
}
