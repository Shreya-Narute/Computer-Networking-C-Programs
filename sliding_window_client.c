#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

struct Frame
{
    int time;
    int transmitted;
    int acknowledgement;
    int outstanding;
};

int main()
{
    int Tt, Tp;
    int a, maxFrames;

    int n = 0;
    int windowSize = 1;

    int sent = 0;
    int ack = 0;
    int outstanding;

    int currentTime = 0;
    int nextSendTime = 0;

    int ackTime[100];

    struct Frame table[200];

    int rows = 0;
    int i;

    int client_socket;

    struct sockaddr_in server_addr;

    /* ---------------------------------
       INPUT
       --------------------------------- */

    printf("Enter Transmission Time: ");
    scanf("%d", &Tt);

    printf("Enter Propagation Delay: ");
    scanf("%d", &Tp);


    /* ---------------------------------
       CALCULATE a
       --------------------------------- */

    a = Tp / Tt;


    /* ---------------------------------
       CALCULATE MAXIMUM FRAMES
       --------------------------------- */

    maxFrames = 1 + 2 * a;


    /* ---------------------------------
       CALCULATE n AND WINDOW SIZE
       --------------------------------- */

    while (windowSize * 2 <= maxFrames)
    {
        windowSize = windowSize * 2;
        n++;
    }


    /* ---------------------------------
       INITIAL OUTSTANDING FRAMES
       --------------------------------- */

    outstanding = maxFrames;


    /* ---------------------------------
       CREATE TCP SOCKET
       --------------------------------- */

    client_socket = socket(AF_INET,
                           SOCK_STREAM,
                           0);

    if (client_socket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }


    /* ---------------------------------
       SERVER ADDRESS
       --------------------------------- */

    server_addr.sin_family = AF_INET;

    server_addr.sin_addr.s_addr = INADDR_ANY;

    server_addr.sin_port = htons(9001);


    /* ---------------------------------
       CONNECT TO SERVER
       --------------------------------- */

    if (connect(client_socket,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        printf("Failed to connect\n");
        close(client_socket);
        return 1;
    }

    printf("\nConnected to server.\n");


    /* ---------------------------------
       SLIDING WINDOW
       --------------------------------- */

    while (ack < maxFrames)
    {

        /*
           If window has space,
           transmit next frame.
        */

        if (sent < maxFrames &&
            sent - ack < windowSize)
        {

            /*
               Transmission can start only
               after previous transmission finishes.
            */

            if (nextSendTime > currentTime)
                currentTime = nextSendTime;


            /*
               Store transmission information
            */

            table[rows].time = currentTime;

            table[rows].transmitted = sent;

            table[rows].acknowledgement = -1;

            table[rows].outstanding = outstanding;

            rows++;


            /*
               Send frame through TCP
            */

            write(client_socket,
                  &sent,
                  sizeof(sent));


            /*
               ACK arrival time
            */

            ackTime[sent] =
                currentTime + Tt + (2 * Tp);


            /*
               Move to next frame
            */

            sent++;


            /*
               Decrease outstanding
            */

            outstanding--;


            /*
               Next frame starts after Tt
            */

            nextSendTime =
                currentTime + Tt;


            currentTime = nextSendTime;
        }


        /*
           Window is full.
           Wait for ACK.
        */

        else
        {

            /*
               Wait until ACK of oldest
               frame is expected.
            */

            if (currentTime < ackTime[ack])
                currentTime = ackTime[ack];


            /*
               Receive ACK from server
            */

            int received_ack;

            read(client_socket,
                 &received_ack,
                 sizeof(received_ack));


            /*
               Store ACK information
            */

            table[rows].time = currentTime;

            table[rows].transmitted = -1;

            table[rows].acknowledgement =
                received_ack;

            table[rows].outstanding =
                outstanding;

            rows++;


            /*
               Move acknowledgement forward
            */

            ack++;
        }
    }


    /* ---------------------------------
       PRINT CALCULATIONS
       --------------------------------- */

    printf("\na = %d", a);

    printf("\nMaximum Frames = %d",
           maxFrames);

    printf("\nn = %d", n);

    printf("\nWindow Size = %d\n",
           windowSize);


    /* ---------------------------------
       PRINT TABLE
       --------------------------------- */

    printf("\n--------------------------------------------------\n");

    printf("Time\tTransmitted\tACK\tOutstanding\n");

    printf("--------------------------------------------------\n");


    for (i = 0; i < rows; i++)
    {
        printf("%d\t",
               table[i].time);


        if (table[i].transmitted == -1)
            printf("-\t\t");
        else
            printf("%d\t\t",
                   table[i].transmitted);


        if (table[i].acknowledgement == -1)
            printf("-\t");
        else
            printf("%d\t",
                   table[i].acknowledgement);


        printf("%d\n",
               table[i].outstanding);
    }


    printf("--------------------------------------------------\n");


    close(client_socket);

    return 0;
}
