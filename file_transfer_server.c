#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char filename[100];
    FILE *fp;
    ssize_t bytes_received;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
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

    printf("Server waiting for client connection...\n");

    client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                       &client_len);

    if (client_fd < 0)
    {
        perror("Accept failed");
        close(server_fd);
        return 1;
    }

    printf("Client connected successfully.\n");

    int name_len;
    if (recv(client_fd, &name_len, sizeof(name_len), MSG_WAITALL)
        != sizeof(name_len))
    {
        printf("Failed to receive filename length.\n");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    name_len = ntohl(name_len);

    if (name_len < 1 || name_len >= (int)sizeof(filename))
    {
        printf("Invalid filename length.\n");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    if (recv(client_fd, filename, name_len, MSG_WAITALL) != name_len)
    {
        printf("Failed to receive filename.\n");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    filename[name_len] = '\0';

    /* Save received files with a fixed local name. */
    fp = fopen("received_file", "wb");

    if (fp == NULL)
    {
        perror("File creation failed");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    printf("Receiving file: %s\n", filename);

    while ((bytes_received = recv(client_fd, buffer,
                                  sizeof(buffer), 0)) > 0)
    {
        if (fwrite(buffer, 1, bytes_received, fp)
            != (size_t)bytes_received)
        {
            perror("File write failed");
            fclose(fp);
            close(client_fd);
            close(server_fd);
            return 1;
        }
    }

    fclose(fp);

    if (bytes_received < 0)
        perror("File reception failed");
    else
        printf("File received successfully as received_file\n");

    close(client_fd);
    close(server_fd);

    return bytes_received < 0 ? 1 : 0;
}
