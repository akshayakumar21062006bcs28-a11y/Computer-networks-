[24bcs131@mepcolinux EX6]$cat ex1_server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // Added for close()
#include <sys/socket.h>  // Added for Linux socket API
#include <netinet/in.h>  // Added for sockaddr_in
#include <arpa/inet.h>   // Added for inet_ntoa

#define PORT 5000
#define BUFFER_SIZE 1024

int main()
{
    int sockfd; // Changed from SOCKET to standard int

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    char buffer[BUFFER_SIZE];
    char reply[BUFFER_SIZE];

    socklen_t addr_len; // Changed int to socklen_t for Linux compliance

    /* Create UDP socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) // Linux sockets return -1 on error
    {
        printf("Socket creation failed\n");
        return 1;
    }

    /* Clear server address */
    memset(&server_addr, 0, sizeof(server_addr));

    /* Set server address */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    /* Bind socket */
    if (bind(sockfd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) // Checked against < 0
    {
        printf("Bind failed\n");
        close(sockfd); // Changed closesocket to close
        return 1;
    }

    printf("UDP Chat Server started...\n");
    printf("Listening on port %d...\n", PORT);

    /*
     * Iterative communication
     * Server handles one message at a time.
     */
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        addr_len = sizeof(client_addr);

        /* Receive message from any client */
        int bytes_received = recvfrom(
            sockfd,
            buffer,
            BUFFER_SIZE - 1,
            0,
            (struct sockaddr *)&client_addr,
            &addr_len
        );

        if (bytes_received < 0)
        {
            printf("recvfrom failed\n");
            continue;
        }

        buffer[bytes_received] = '\0';

        /* Display client information */
        printf("\n----------------------------------\n");

        printf("Client IP   : %s\n",
               inet_ntoa(client_addr.sin_addr));

        printf("Client Port : %d\n",
               ntohs(client_addr.sin_port));

        printf("Client      : %s", buffer);

        /* Check for exit message */
        if (strncmp(buffer, "exit", 4) == 0)
        {
            printf("Client disconnected.\n");
            continue;
        }

        /* Get reply from server */
        printf("Server      : ");

        fgets(reply, BUFFER_SIZE, stdin);

        /* Send reply to the same client */
        int bytes_sent = sendto(
            sockfd,
            reply,
            strlen(reply),
            0,
            (struct sockaddr *)&client_addr,
            addr_len
        );

        if (bytes_sent < 0)
        {
            printf("sendto failed\n");
            continue;
        }

        printf("Reply sent to client.\n");

        /* Server continues handling other clients */
    }

    /* Close socket */
    close(sockfd); // Changed closesocket to close

    return 0;
}
[24bcs131@mepcolinux EX6]$cc ex1_server.c
[24bcs131@mepcolinux EX6]$./a.out
UDP Chat Server started...
Listening on port 5000...

----------------------------------
Client IP   : 127.0.0.1
Client Port : 33709
Client      : hiServer      : hello
Reply sent to client.

----------------------------------
Client IP   : 127.0.0.1
Client Port : 33709
Client      : how are you?Server      : I am fine.
Reply sent to client.

----------------------------------
Client IP   : 127.0.0.1
Client Port : 33709
Client      : ok you want any helpServer      : no i can manage
Reply sent to client.

----------------------------------
Client IP   : 127.0.0.1
Client Port : 33709
Client      : ok byeServer      : bye
Reply sent to client.

----------------------------------
Client IP   : 127.0.0.1
Client Port : 33709
Client      : byeServer      : exit
Reply sent to client.

----------------------------------
Client IP   : 127.0.0.1
Client Port : 33709
Client      : exitClient disconnected.
[24bcs131@mepcolinux EX6]$cat ex1_client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // Added for close()
#include <sys/socket.h>  // Added for Linux socket API
#include <netinet/in.h>  // Added for sockaddr_in
#include <arpa/inet.h>   // Added for inet_pton

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5000
#define BUFFER_SIZE 1024

int main()
{
    int sockfd; // Changed from SOCKET to standard int

    struct sockaddr_in server_addr;

    char buffer[BUFFER_SIZE];

    socklen_t addr_len; // Changed int to socklen_t for Linux compliance

    /* Create UDP socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) // Linux sockets return -1 on error
    {
        printf("Socket creation failed\n");
        return 1;
    }

    /* Clear server address */
    memset(&server_addr, 0, sizeof(server_addr));

    /* Set server address */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    /* Convert IP address */
    if (inet_pton(AF_INET,
                  SERVER_IP,
                  &server_addr.sin_addr) <= 0)
    {
        printf("Invalid server IP address\n");
        close(sockfd); // Changed closesocket to close
        return 1;
    }

    printf("UDP Chat Client started...\n");

    printf("Connected to server %s:%d\n",
           SERVER_IP,
           SERVER_PORT);

    while (1)
    {
        /* Enter message */
        printf("\nClient: ");

        fgets(buffer, BUFFER_SIZE, stdin);

        /* Remove newline */
        buffer[strcspn(buffer, "\n")] = '\0';

        /* Send message to server */
        int bytes_sent = sendto(
            sockfd,
            buffer,
            strlen(buffer),
            0,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)
        );

        if (bytes_sent < 0)
        {
            printf("sendto failed\n");
            break;
        }

        /* Exit condition */
        if (strcmp(buffer, "exit") == 0)
        {
            printf("Chat ended.\n");
            break;
        }

        /* Receive server reply */
        addr_len = sizeof(server_addr);

        int bytes_received = recvfrom(
            sockfd,
            buffer,
            BUFFER_SIZE - 1,
            0,
            (struct sockaddr *)&server_addr,
            &addr_len
        );

        if (bytes_received < 0)
        {
            printf("recvfrom failed\n");
            break;
        }

        /* Null terminate */
        buffer[bytes_received] = '\0';

        printf("Server: %s\n", buffer);

        /* Check server exit */
        if (strcmp(buffer, "exit") == 0)
        {
            printf("Server ended the chat.\n");
            break;
        }
    }

    /* Close socket */
    close(sockfd); // Changed closesocket to close

    return 0;
}
