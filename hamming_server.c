#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{
    int serverSocket, clientSocket;
    int data[4];
    int hamming[7];

    int p1, p2, p4;

    struct sockaddr_in server, client;
    socklen_t clientLength = sizeof(client);

    /* Create socket */
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(serverSocket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    /* Server details */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(9001);

    /* Bind */
    if(bind(serverSocket,
            (struct sockaddr *)&server,
            sizeof(server)) < 0)
    {
        printf("Bind failed\n");
        close(serverSocket);
        return 1;
    }

    /* Listen */
    listen(serverSocket, 5);

    printf("Server waiting for client...\n");

    /* Accept client */
    clientSocket = accept(serverSocket,
                          (struct sockaddr *)&client,
                          &clientLength);

    if(clientSocket < 0)
    {
        printf("Accept failed\n");
        close(serverSocket);
        return 1;
    }

    /* Receive 4 data bits */
    recv(clientSocket, data, sizeof(data), 0);

    printf("\nReceived Data Bits: ");

    for(int i = 0; i < 4; i++)
        printf("%d", data[i]);

    /*
       Hamming positions:

       Position:  1  2  3  4  5  6  7
                  P1 P2 D1 P4 D2 D3 D4
    */

    hamming[0] = 0;       /* P1 */
    hamming[1] = 0;       /* P2 */
    hamming[2] = data[0]; /* D1 */
    hamming[3] = 0;       /* P4 */
    hamming[4] = data[1]; /* D2 */
    hamming[5] = data[2]; /* D3 */
    hamming[6] = data[3]; /* D4 */

    /*
       Calculate parity bits using XOR.

       P1 checks positions:
       1, 3, 5, 7

       P2 checks positions:
       2, 3, 6, 7

       P4 checks positions:
       4, 5, 6, 7
    */

    p1 = hamming[2] ^ hamming[4] ^ hamming[6];
    p2 = hamming[2] ^ hamming[5] ^ hamming[6];
    p4 = hamming[4] ^ hamming[5] ^ hamming[6];

    hamming[0] = p1;
    hamming[1] = p2;
    hamming[3] = p4;

    printf("\nParity P1 = %d", p1);
    printf("\nParity P2 = %d", p2);
    printf("\nParity P4 = %d", p4);

    printf("\n\nHamming Code: ");

    for(int i = 0; i < 7; i++)
        printf("%d", hamming[i]);

    /* Send Hamming code to client */
    send(clientSocket, hamming, sizeof(hamming), 0);

    close(clientSocket);
    close(serverSocket);

    return 0;
}
