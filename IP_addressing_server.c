
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);

    char ip[50] = {0};
    char result[1000];

    int a, b, c, d;
    int network_bits, host_bits;
    long long networks, hosts;

    // Create TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    // Configure server address
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(9000);
    server.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to port 9000
    if (bind(server_fd, (struct sockaddr *)&server,
             sizeof(server)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    // Listen for client connections
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    printf("Server waiting for client...\n");

    // Accept client connection
    client_fd = accept(server_fd,
                       (struct sockaddr *)&client, &len);

    if (client_fd < 0)
    {
        perror("Accept failed");
        close(server_fd);
        return 1;
    }

    printf("Client connected.\n");

    // Receive IP address from client
    int n = recv(client_fd, ip, sizeof(ip) - 1, 0);

    if (n <= 0)
    {
        printf("Failed to receive IP address\n");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    ip[n] = '\0';
    printf("Received IP: %s\n", ip);

    // Validate IPv4 address
    struct in_addr addr;
    if (inet_pton(AF_INET, ip, &addr) != 1)
    {
        snprintf(result, sizeof(result),
                 "Invalid IPv4 address: %s\n", ip);
        send(client_fd, result, strlen(result) + 1, 0);
        close(client_fd);
        close(server_fd);
        return 0;
    }

    // Split IP address into four octets
    sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d);

    // Class A
    if (a >= 1 && a <= 126)
    {
        network_bits = 7;
        host_bits = 24;
        networks = (1LL << network_bits) - 2;
        hosts = (1LL << host_bits) - 2;

        snprintf(result, sizeof(result),
            "\n--- IP ADDRESS DETAILS ---\n"
            "IP Address        : %s\n"
            "Class             : A\n"
            "Network ID        : %d.0.0.0\n"
            "Host ID           : %d.%d.%d\n"
            "Number of Networks: %lld\n"
            "Number of Hosts   : %lld\n"
            "LBA               : 255.255.255.255\n"
            "DBA               : %d.255.255.255\n",
            ip, a, b, c, d, networks, hosts, a);
    }

    // Class B
    else if (a >= 128 && a <= 191)
    {
        network_bits = 14;
        host_bits = 16;
        networks = 1LL << network_bits;
        hosts = (1LL << host_bits) - 2;

        snprintf(result, sizeof(result),
            "\n--- IP ADDRESS DETAILS ---\n"
            "IP Address        : %s\n"
            "Class             : B\n"
            "Network ID        : %d.%d.0.0\n"
            "Host ID           : %d.%d\n"
            "Number of Networks: %lld\n"
            "Number of Hosts   : %lld\n"
            "LBA               : 255.255.255.255\n"
            "DBA               : %d.%d.255.255\n",
            ip, a, b, c, d, networks, hosts, a, b);
    }

    // Class C
    else if (a >= 192 && a <= 223)
    {
        network_bits = 21;
        host_bits = 8;
        networks = 1LL << network_bits;
        hosts = (1LL << host_bits) - 2;

        snprintf(result, sizeof(result),
            "\n--- IP ADDRESS DETAILS ---\n"
            "IP Address        : %s\n"
            "Class             : C\n"
            "Network ID        : %d.%d.%d.0\n"
            "Host ID           : %d\n"
            "Number of Networks: %lld\n"
            "Number of Hosts   : %lld\n"
            "LBA               : 255.255.255.255\n"
            "DBA               : %d.%d.%d.255\n",
            ip, a, b, c, d, networks, hosts, a, b, c);
    }

    // Class D
    else if (a >= 224 && a <= 239)
    {
        snprintf(result, sizeof(result),
            "IP Address: %s\n"
            "Class: D\n"
            "Purpose: Multicasting\n", ip);
    }

    // Class E
    else
    {
        snprintf(result, sizeof(result),
            "IP Address: %s\n"
            "Class: E\n"
            "Purpose: Reserved/Experimental\n", ip);
    }

    // Send calculated details to client
    send(client_fd, result, strlen(result) + 1, 0);

    close(client_fd);
    close(server_fd);

    return 0;
}
