#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int serversocket, bindstatus;
    char servermessage[256] = "Hello Client and we are connected now";

    struct sockaddr_in serveraddress, clientaddress;
    socklen_t clientlen = sizeof(clientaddress);

    serversocket = socket(AF_INET, SOCK_DGRAM, 0);

    if (serversocket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(9000);
    serveraddress.sin_addr.s_addr = INADDR_ANY;

    bindstatus = bind(serversocket,
                      (struct sockaddr *)&serveraddress,
                      sizeof(serveraddress));

    if (bindstatus < 0)
    {
        printf("Binding failed\n");
        return 1;
    }

    printf("Binding is successful\n");
    printf("Waiting for client...\n");

    char buffer[256];

    recvfrom(serversocket,
             buffer,
             sizeof(buffer),
             0,
             (struct sockaddr *)&clientaddress,
             &clientlen);

    printf("Message from client: %s\n", buffer);

    sendto(serversocket,
           servermessage,
           sizeof(servermessage),
           0,
           (struct sockaddr *)&clientaddress,
           clientlen);

    printf("Reply sent to client\n");

    close(serversocket);

    return 0;
}
