
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
    int server_fd, client_fd;
    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(client);

    int n, source;
    int graph[MAX][MAX];
    char result[4096];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server,
             sizeof(server)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    printf("Link State Routing server waiting on port %d...\n",
           PORT);

    client_fd = accept(server_fd,
                       (struct sockaddr *)&client,
                       &client_len);

    if (client_fd < 0)
    {
        perror("Accept failed");
        close(server_fd);
        return 1;
    }

    printf("Client connected.\n");

    int header[2];

    if (!receive_all(client_fd, header, sizeof(header)))
        goto communication_error;

    n = ntohl((unsigned int)header[0]);
    source = ntohl((unsigned int)header[1]);

    if (n < 2 || n > MAX || source < 0 || source >= n)
    {
        const char *msg = "Invalid router count or source.\n";
        send_all(client_fd, msg, strlen(msg) + 1);
        goto cleanup;
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            unsigned int value;

            if (!receive_all(client_fd, &value, sizeof(value)))
                goto communication_error;

            graph[i][j] = (int)ntohl(value);
        }
    }

    int dist[MAX], visited[MAX], parent[MAX];

    for (int i = 0; i < n; i++)
    {
        dist[i] = INF;
        visited[i] = 0;
        parent[i] = -1;
    }

    dist[source] = 0;

    for (int count = 0; count < n; count++)
    {
        int u = -1;
        int best = INF;

        for (int i = 0; i < n; i++)
        {
            if (!visited[i] && dist[i] < best)
            {
                best = dist[i];
                u = i;
            }
        }

        if (u == -1)
            break;

        visited[u] = 1;

        for (int v = 0; v < n; v++)
        {
            if (graph[u][v] >= 0 &&
                graph[u][v] < INF &&
                !visited[v] &&
                dist[u] + graph[u][v] < dist[v])
            {
                dist[v] = dist[u] + graph[u][v];
                parent[v] = u;
            }
        }
    }

    int used = snprintf(result, sizeof(result),
        "\nLINK STATE ROUTING - DIJKSTRA'S ALGORITHM\n"
        "Source Router: R%d\n\n"
        "Destination\tCost\tShortest Path\n",
        source + 1);

    for (int v = 0; v < n && used < (int)sizeof(result); v++)
    {
        char path[256] = "";
        int nodes[MAX];
        int count = 0;
        int current = v;

        if (dist[v] == INF)
        {
            used += snprintf(result + used,
                             sizeof(result) - used,
                             "R%d\t\tINF\tUnreachable\n", v + 1);
            continue;
        }

        while (current != -1 && count < MAX)
        {
            nodes[count++] = current;
            current = parent[current];
        }

        for (int k = count - 1; k >= 0; k--)
        {
            char part[24];
            snprintf(part, sizeof(part), "R%d",
                     nodes[k] + 1);
            strncat(path, part, sizeof(path) - strlen(path) - 1);

            if (k > 0)
                strncat(path, " -> ",
                        sizeof(path) - strlen(path) - 1);
        }

        used += snprintf(result + used,
                         sizeof(result) - used,
                         "R%d\t\t%d\t%s\n",
                         v + 1, dist[v], path);
    }

    if (used >= (int)sizeof(result))
        result[sizeof(result) - 1] = '\0';

    if (!send_all(client_fd, result, strlen(result) + 1))
        printf("Failed to send result.\n");
    else
        printf("Shortest-path results sent to client.\n");

    goto cleanup;

communication_error:
    printf("Communication error.\n");

cleanup:
    close(client_fd);
    close(server_fd);
    return 0;
}
