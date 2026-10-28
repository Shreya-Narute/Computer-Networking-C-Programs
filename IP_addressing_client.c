
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{
    int sock;
    struct sockaddr_in server;

    char ip[50];
    char result[1000] = {0};

    // Create TCP socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Configure server address
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(9000);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    // Connect to server
    if (connect(sock, (struct sockaddr *)&server,
                sizeof(server)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    printf("Connected to server.\n");
    printf("Enter IP address: ");

    if (scanf("%49s", ip) != 1)
    {
        printf("Invalid input\n");
        close(sock);
        return 1;
    }

    // Send IP address to server
    if (send(sock, ip, strlen(ip) + 1, 0) < 0)
    {
        perror("Send failed");
        close(sock);
        return 1;
    }

    // Receive result from server
    int n = recv(sock, result, sizeof(result) - 1, 0);

    if (n > 0)
    {
        result[n] = '\0';
        printf("\nResult received from server:\n%s", result);
    }
    else
    {
        printf("Failed to receive result\n");
    }

    close(sock);

    return 0;
}
