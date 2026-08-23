#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int clientSocket;
    int num1, num2, result;
    char operator;

    struct sockaddr_in serverAddress;

    /* Create socket */
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(clientSocket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    printf("Socket created successfully\n");

    /* Server address */
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    /* Connect */
    if(connect(clientSocket,
               (struct sockaddr *)&serverAddress,
               sizeof(serverAddress)) < 0)
    {
        printf("Connection failed\n");
        close(clientSocket);
        return 1;
    }

    printf("Connection established\n");

    /*
       Continue taking calculations
    */
    while(1)
    {
        printf("\nEnter first number: ");
        scanf("%d", &num1);

        printf("Enter operator (+, -, *, /) or 0 to exit: ");
        scanf(" %c", &operator);

        printf("Enter second number: ");
        scanf("%d", &num2);

        /* Send data to server */
        send(clientSocket, &num1, sizeof(num1), 0);

        send(clientSocket, &operator, sizeof(operator), 0);

        send(clientSocket, &num2, sizeof(num2), 0);

        /* Stop condition */
        if(operator == '0')
        {
            break;
        }

        /* Receive result */
        recv(clientSocket, &result, sizeof(result), 0);

        printf("Result = %d\n", result);
    }

    close(clientSocket);

    return 0;
}
