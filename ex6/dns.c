#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // Added for close()
#include <sys/socket.h>  // Added for Linux socket API
#include <netinet/in.h>  // Added for sockaddr_in structures
#include <arpa/inet.h>   // Added for network byte order functions

#define PORT 7000
#define BUFFER_SIZE 1024

struct dns_record
{
    char domain[100];
    char ip[50];
};

int main()
{
    int sockfd; // Changed from SOCKET to standard int

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int i;
    socklen_t addr_len; // Changed int to socklen_t for Linux compliance

    /* DNS Records */
    struct dns_record dns_table[] =
    {
        {"google.com", "142.250.195.14"},
        {"youtube.com", "142.250.72.14"},
        {"facebook.com", "157.240.241.35"},
        {"example.com", "93.184.216.34"}
    };

    int total_records = 4;

    /* Create UDP socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) // Linux sockets return -1 on error
    {
        printf("Socket creation failed\n");
        return 1;
    }

    // Force reuse port to prevent "Bind failed" errors if port 8080 is stuck
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* Clear server address */
    memset(&server_addr, 0, sizeof(server_addr));

    /* Server address */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    /* Bind socket */
    if (bind(sockfd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) // Evaluated against < 0
    {
        printf("Bind failed\n");
        close(sockfd); // Changed closesocket to close
        return 1;
    }

    printf("UDP DNS Server started...\n");
    printf("Listening on port %d...\n", PORT);

    /* Iterative DNS Communication */
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        addr_len = sizeof(client_addr);

        /* Receive domain name from client */
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

        printf("\nClient requested: %s\n", buffer);

        /* Exit condition */
        if (strcmp(buffer, "exit") == 0)
        {
            strcpy(response, "DNS Server stopped");

            sendto(
                sockfd,
                response,
                strlen(response),
                0,
                (struct sockaddr *)&client_addr,
                addr_len
            );

            break;
        }

        /* Search DNS table */
        int found = 0;

        for (i = 0; i < total_records; i++)
        {
            if (strcmp(buffer, dns_table[i].domain) == 0)
            {
                strcpy(response, dns_table[i].ip);
                found = 1;
                break;
            }
        }

        /* Domain not found */
        if (!found)
        {
            strcpy(response, "Domain not found");
        }

        /* Send IP address to client */
        int bytes_sent = sendto(
            sockfd,
            response,
            strlen(response),
            0,
            (struct sockaddr *)&client_addr,
            addr_len
        );

        if (bytes_sent < 0)
        {
            printf("sendto failed\n");
            continue;
        }

        printf("Response sent: %s\n", response);
    }

    /* Close socket */
    close(sockfd); // Changed closesocket to close

    return 0;
}
[24bcs131@mepcolinux EX6]$cc ex3_server.c
[24bcs131@mepcolinux EX6]$./a.out
UDP DNS Server started...
Listening on port 7000...

Client requested: google.com
Response sent: 142.250.195.14

Client requested: youtube.com
Response sent: 142.250.72.14

Client requested: yahoo.com
Response sent: Domain not found

Client requested: exit
[24bcs131@mepcolinux EX6]$cat exx3_client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // Added for close()
#include <sys/socket.h>  // Added for Linux socket API
#include <netinet/in.h>  // Added for sockaddr_in structures
#include <arpa/inet.h>   // Added for inet_pton

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 7000
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

    /* Server address */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    /* Convert server IP */
    if (inet_pton(AF_INET,
                  SERVER_IP,
                  &server_addr.sin_addr) <= 0)
    {
        printf("Invalid server IP\n");
        close(sockfd); // Changed closesocket to close
        return 1;
    }

    printf("UDP DNS Client started...\n");
    printf("Server: %s:%d\n",
           SERVER_IP,
           SERVER_PORT);

    while (1)
    {
        /* Enter domain name */
        printf("\nEnter domain name: ");

        fgets(buffer, BUFFER_SIZE, stdin);

        /* Remove newline */
        buffer[strcspn(buffer, "\n")] = '\0';

        /* Send domain to DNS server */
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

        /* Exit */
        if (strcmp(buffer, "exit") == 0)
        {
            break;
        }

        /* Receive DNS response */
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

        buffer[bytes_received] = '\0';

        printf("IP Address: %s\n", buffer);
    }

    /* Close socket */
    close(sockfd); // Changed closesocket to close

    return 0;
}
