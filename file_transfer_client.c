#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    FILE *fp;
    size_t bytes_read;

    if (argc != 3)
    {
        printf("Usage: %s <server_ip> <filename>\n", argv[0]);
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) != 1)
    {
        printf("Invalid server IP address.\n");
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    fp = fopen(argv[2], "rb");

    if (fp == NULL)
    {
        perror("File opening failed");
        close(sock);
        return 1;
    }

    const char *base = strrchr(argv[2], '/');
    base = base ? base + 1 : argv[2];

    size_t name_len = strlen(base);

    if (name_len == 0 || name_len >= 100)
    {
        printf("Invalid filename.\n");
        fclose(fp);
        close(sock);
        return 1;
    }

    int net_name_len = htonl((int)name_len);

    if (send(sock, &net_name_len, sizeof(net_name_len), 0)
        != sizeof(net_name_len))
    {
        perror("Filename length send failed");
        fclose(fp);
        close(sock);
        return 1;
    }

    size_t sent = 0;
    while (sent < name_len)
    {
        ssize_t n = send(sock, base + sent, name_len - sent, 0);

        if (n <= 0)
        {
            perror("Filename send failed");
            fclose(fp);
            close(sock);
            return 1;
        }

        sent += (size_t)n;
    }

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0)
    {
        size_t sent_bytes = 0;

        while (sent_bytes < bytes_read)
        {
            ssize_t n = send(sock, buffer + sent_bytes,
                             bytes_read - sent_bytes, 0);

            if (n <= 0)
            {
                perror("File send failed");
                fclose(fp);
                close(sock);
                return 1;
            }

            sent_bytes += (size_t)n;
        }
    }

    if (ferror(fp))
    {
        perror("File reading failed");
        fclose(fp);
        close(sock);
        return 1;
    }

    printf("File sent successfully: %s\n", base);

    fclose(fp);

    /* Tell the server that no more file data will be sent. */
    shutdown(sock, SHUT_WR);
    close(sock);

    return 0;
}
