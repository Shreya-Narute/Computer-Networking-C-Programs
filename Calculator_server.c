#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int serverSocket, clientSocket;
    int num1, num2, result;
    char operator;

    struct sockaddr_in serverAddress;

    /* Create TCP socket */
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(serverSocket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    printf("Socket created successfully\n");

    /* Server address */
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    /* Bind */
    if(bind(serverSocket,
            (struct sockaddr *)&serverAddress,
            sizeof(serverAddress)) < 0)
    {
        printf("Binding failed\n");
        close(serverSocket);
        return 1;
    }

    printf("Binding successful\n");

    /* Listen */
    listen(serverSocket, 3);

    printf("Waiting for client...\n");

    /* Accept client */
    clientSocket = accept(serverSocket, NULL, NULL);

    if(clientSocket < 0)
    {
        printf("Connection failed\n");
        close(serverSocket);
        return 1;
    }

    printf("Client connected successfully\n");

    /*
       Receive and calculate continuously
       until operator becomes 0
    */
    while(1)
    {
        /* Receive first number */
        recv(clientSocket, &num1, sizeof(num1), 0);

        /* Receive operator */
        recv(clientSocket, &operator, sizeof(operator), 0);

        /* Receive second number */
        recv(clientSocket, &num2, sizeof(num2), 0);

        printf("\nReceived: %d %c %d\n",
               num1, operator, num2);

        /* Stop condition */
        if(operator == '0')
        {
            break;
        }

        /* Calculator logic */

        if(operator == '+')
        {
            result = num1 + num2;
        }
        else if(operator == '-')
        {
            result = num1 - num2;
        }
        else if(operator == '*')
        {
            result = num1 * num2;
        }
        else if(operator == '/')
        {
            if(num2 == 0)
            {
                printf("Division by zero is not possible\n");
                result = 0;
            }
            else
            {
                result = num1 / num2;
            }
        }
        else
        {
            printf("Invalid operator\n");
            result = 0;
        }

        printf("Result = %d\n", result);

        /* Send result */
        send(clientSocket, &result, sizeof(result), 0);
    }

    close(clientSocket);
    close(serverSocket);

    return 0;
}
