
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(client);

    char ip[INET_ADDRSTRLEN];
    char input[100] = {0};
    char result[1000];

    struct in_addr ip_addr, net_addr, bc_addr;
    char mask_str[INET_ADDRSTRLEN];
    char network_str[INET_ADDRSTRLEN];
    char broadcast_str[INET_ADDRSTRLEN];
    char first_ip[INET_ADDRSTRLEN];
    char last_ip[INET_ADDRSTRLEN];

    unsigned int ip_num, mask_num;
    unsigned int network, broadcast;
    unsigned int first, last;
    int prefix, ip_class;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(9001);
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

    printf("Subnetting server waiting for client...\n");

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

    int n = recv(client_fd, input, sizeof(input) - 1, 0);

    if (n <= 0)
    {
        printf("Failed to receive IP address.\n");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    input[n] = '\0';

    input[strcspn(input, "\r\n")] = '\0';

    if (inet_pton(AF_INET, input, &ip_addr) != 1)
    {
        snprintf(result, sizeof(result),
                 "Invalid IPv4 address.\n");

        send(client_fd, result, strlen(result) + 1, 0);
        close(client_fd);
        close(server_fd);
        return 0;
    }

    strcpy(ip, input);

    ip_num = ntohl(ip_addr.s_addr);

    first = (ip_num >> 24) & 255U;

    if (first >= 1 && first <= 126)
    {
        ip_class = 1;       // Class A
        prefix = 8;
        mask_num = 0xFF000000U;
    }
    else if (first >= 128 && first <= 191)
    {
        ip_class = 2;       // Class B
        prefix = 16;
        mask_num = 0xFFFF0000U;
    }
    else if (first >= 192 && first <= 223)
    {
        ip_class = 3;       // Class C
        prefix = 24;
        mask_num = 0xFFFFFF00U;
    }
    else
    {
        snprintf(result, sizeof(result),
                 "Unsupported IP address for this "
                 "classful assignment.\n"
                 "Use a normal Class A, B, or C address.\n");

        send(client_fd, result, strlen(result) + 1, 0);
        close(client_fd);
        close(server_fd);
        return 0;
    }

    network = ip_num & mask_num;
    broadcast = network | (~mask_num);

    first = network + 1U;
    last = broadcast - 1U;

    net_addr.s_addr = htonl(network);
    bc_addr.s_addr = htonl(broadcast);

    inet_ntop(AF_INET, &net_addr, network_str,
              sizeof(network_str));
    inet_ntop(AF_INET, &bc_addr, broadcast_str,
              sizeof(broadcast_str));

    struct in_addr first_addr, last_addr;
    first_addr.s_addr = htonl(first);
    last_addr.s_addr = htonl(last);

    inet_ntop(AF_INET, &first_addr, first_ip,
              sizeof(first_ip));
    inet_ntop(AF_INET, &last_addr, last_ip,
              sizeof(last_ip));

    struct in_addr mask_addr;
    mask_addr.s_addr = htonl(mask_num);

    inet_ntop(AF_INET, &mask_addr, mask_str,
              sizeof(mask_str));

    snprintf(result, sizeof(result),
        "\n--- SUBNETTING DETAILS ---\n"
        "Input IP Address : %s\n"
        "IP Class         : Class %c\n"
        "Subnet Mask      : %s\n"
        "CIDR Prefix      : /%d\n"
        "Network Address  : %s\n"
        "Broadcast Address: %s\n"
        "First Usable IP  : %s\n"
        "Last Usable IP   : %s\n"
        "Usable IP Range  : %s - %s\n",
        ip,
        ip_class == 1 ? 'A' :
        ip_class == 2 ? 'B' : 'C',
        mask_str, prefix,
        network_str, broadcast_str,
        first_ip, last_ip,
        first_ip, last_ip);

    send(client_fd, result, strlen(result) + 1, 0);

    printf("Subnetting result sent to client.\n");

    close(client_fd);
    close(server_fd);

    return 0;
}

