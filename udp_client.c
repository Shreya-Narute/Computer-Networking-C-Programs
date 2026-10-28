#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
    int clientsocket;

    struct sockaddr_in serveraddress;

    clientsocket = socket(AF_INET, SOCK_DGRAM, 0);

    if (clientsocket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    printf("Socket created successfully\n");

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(9000);
    serveraddress.sin_addr.s_addr = INADDR_ANY;

    char message[] = "Hello Server";

    sendto(clientsocket,
           message,
           sizeof(message),
           0,
           (struct sockaddr *)&serveraddress,
           sizeof(serveraddress));

    char serverresponse[256];

    socklen_t len = sizeof(serveraddress);

    recvfrom(clientsocket,
             serverresponse,
             sizeof(serverresponse),
             0,
             (struct sockaddr *)&serveraddress,
             &len);

    printf("Reply from server: %s\n", serverresponse);

    close(clientsocket);

    return 0;
}
