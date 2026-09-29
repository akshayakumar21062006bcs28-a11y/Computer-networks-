#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // Added for close()
#include <sys/socket.h>  // Added for Linux socket API
#include <netinet/in.h>  // Added for sockaddr_in structures
#include <arpa/inet.h>   // Added for inet_ntoa, inet_pton, inet_ntop

#define PORT 10240
#define BUFFER_SIZE 1024
#define MAX_SUBNETS 256

int main()
{
    int sockfd; // Changed from SOCKET to standard int

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    socklen_t addr_len; // Changed int to socklen_t for Linux compliance

    char base_ip[20];

    int prefix;
    int num_subnets;
    int required_hosts;
    int i;
    int host_bits;
    int new_prefix;

    unsigned int subnet_mask;
    unsigned int base_ip_num;

    int next_ip[MAX_SUBNETS];

    printf("========== UDP DHCP SERVER ==========\n");

    /* Step 1: Base IP Address */
    printf("Enter Base IP Address: ");
    scanf("%19s", base_ip);

    /* Step 2: Prefix */
    printf("Enter Prefix: /");
    scanf("%d", &prefix);

    /* Step 3: Number of Subnets */
    printf("Enter Number of Subnets: ");
    scanf("%d", &num_subnets);

    /* Step 4: Required Hosts */
    printf("Enter Required Hosts per Subnet: ");
    scanf("%d", &required_hosts);

    /*
     * Step 5: Calculate Host Bits
     */
    host_bits = 0;

    while ((1 << host_bits) - 2 < required_hosts)
    {
        host_bits++;
    }

    /*
     * Step 6: Calculate New Prefix
     */
    new_prefix = 32 - host_bits;

    /*
     * Step 7: Calculate Subnet Mask
     */
    subnet_mask = 0xFFFFFFFF;

    subnet_mask = subnet_mask << host_bits;

    printf("\n========== NETWORK DETAILS ==========\n");

    printf("Base IP Address     : %s\n", base_ip);
    printf("Original Prefix     : /%d\n", prefix);
    printf("Number of Subnets   : %d\n", num_subnets);
    printf("Required Hosts      : %d\n", required_hosts);
    printf("Host Bits           : %d\n", host_bits);
    printf("New Prefix          : /%d\n", new_prefix);

    /*
     * Display subnet mask
     */
    struct in_addr mask_addr;

    mask_addr.s_addr = htonl(subnet_mask);

    printf("Subnet Mask         : %s\n",
           inet_ntoa(mask_addr));

    /*
     * Convert Base IP to integer
     */
    struct in_addr temp_addr;

    if (inet_pton(AF_INET, base_ip, &temp_addr) != 1)
    {
        printf("Invalid Base IP Address\n");
        return 1;
    }

    base_ip_num = ntohl(temp_addr.s_addr);

    /*
     * Initialize IP allocation
     */
    for ( i = 0; i < MAX_SUBNETS; i++)
    {
        next_ip[i] = 1;
    }

    /*
     * Create UDP Socket
     */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) // Linux sockets return -1 on error
    {
        printf("Socket creation failed\n");
        return 1;
    }

    /*
     * Clear server address
     */
    memset(&server_addr, 0, sizeof(server_addr));

    /*
     * Server Address
     */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    /*
     * Bind
     */
    if (bind(sockfd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) // Evaluated against < 0
    {
        printf("Bind failed\n");
        close(sockfd); // Changed closesocket to close
        return 1;
    }

    printf("\nDHCP UDP Server started...\n");
    printf("Listening on Port %d...\n", PORT);

    /*
     * Step 9 onwards:
     * Iterative UDP Communication
     */

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        addr_len = sizeof(client_addr);

        /*
         * Step 10:
         * Receive Client Request
         */

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

        printf("\n====================================\n");

        printf("Request received from Client\n");

        printf("Client IP   : %s\n",
               inet_ntoa(client_addr.sin_addr));

        printf("Client Port : %d\n",
               ntohs(client_addr.sin_port));

        printf("Request     : %s\n", buffer);

        /*
         * Client sends:
         * SUBNET 0
         * SUBNET 1
         * SUBNET 2
         */

        int requested_subnet;

        if (sscanf(buffer,
                   "SUBNET %d",
                   &requested_subnet) != 1)
        {
            strcpy(response,
                   "ERROR: Invalid Request");

            sendto(
                sockfd,
                response,
                strlen(response),
                0,
                (struct sockaddr *)&client_addr,
                addr_len
            );

            continue;
        }

        /*
         * Step 11:
         * Select Requested Subnet
         */

        if (requested_subnet < 0 ||
            requested_subnet >= num_subnets)
        {
            strcpy(response,
                   "ERROR: Invalid Subnet");

            sendto(
                sockfd,
                response,
                strlen(response),
                0,
                (struct sockaddr *)&client_addr,
                addr_len
            );

            continue;
        }

        /*
         * Calculate subnet size
         */

        int subnet_size = 1 << host_bits;

        /*
         * Calculate selected subnet network
         */

        unsigned int subnet_network =
            base_ip_num +
            (requested_subnet * subnet_size);

        /*
         * Check available IP
         */

        if (next_ip[requested_subnet] >=
            subnet_size - 1)
        {
            strcpy(response,
                   "ERROR: No IP Available");

            sendto(
                sockfd,
                response,
                strlen(response),
                0,
                (struct sockaddr *)&client_addr,
                addr_len
            );

            continue;
        }

        /*
         * Step 12:
         * Assign IP Address
         */

        unsigned int assigned_ip =
            subnet_network +
            next_ip[requested_subnet];

        next_ip[requested_subnet]++;

        /*
         * Convert assigned IP to string
         */

        struct in_addr assigned_addr;

        assigned_addr.s_addr =
            htonl(assigned_ip);

        char assigned_ip_string[INET_ADDRSTRLEN];

        inet_ntop(
            AF_INET,
            &assigned_addr,
            assigned_ip_string,
            INET_ADDRSTRLEN
        );

        /*
         * Convert network address to string
         */

        struct in_addr network_addr;

        network_addr.s_addr =
            htonl(subnet_network);

        char network_ip_string[INET_ADDRSTRLEN];

        inet_ntop(
            AF_INET,
            &network_addr,
            network_ip_string,
            INET_ADDRSTRLEN
        );

        /*
         * Prepare Response
         */

        sprintf(
            response,
            "IP ASSIGNED: %s | SUBNET: %s/%d",
            assigned_ip_string,
            network_ip_string,
            new_prefix
        );

        printf("Selected Subnet : %d\n",
               requested_subnet);

        printf("Network Address : %s/%d\n",
               network_ip_string,
               new_prefix);

        printf("Assigned IP     : %s\n",
               assigned_ip_string);

        /*
         * Step 13:
         * Send UDP Response
         */

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
        }
        else
        {
            printf("Response sent to Client.\n");
        }
    }

    /*
     * Close socket
     */
    close(sockfd); // Changed closesocket to close
    return 0;
}
[24bcs131@mepcolinux EX6]$cc ex2_server.c
[24bcs131@mepcolinux EX6]$./a.out
========== UDP DHCP SERVER ==========
Enter Base IP Address: 192.168.1.0
Enter Prefix: /24
Enter Number of Subnets: 4
Enter Required Hosts per Subnet: 4

========== NETWORK DETAILS ==========
Base IP Address     : 192.168.1.0
Original Prefix     : /24
Number of Subnets   : 4
Required Hosts      : 4
Host Bits           : 3
New Prefix          : /29
Subnet Mask         : 255.255.255.248

DHCP UDP Server started...
Listening on Port 10240...

====================================
Request received from Client
Client IP   : 127.0.0.1
Client Port : 53486
Request     : SUBNET 0
Selected Subnet : 0
Network Address : 192.168.1.0/29
Assigned IP     : 192.168.1.1
Response sent to Client.

====================================
Request received from Client
Client IP   : 127.0.0.1
Client Port : 43278
Request     : SUBNET 1
Selected Subnet : 1
Network Address : 192.168.1.8/29
Assigned IP     : 192.168.1.9
Response sent to Client.

====================================
Request received from Client
Client IP   : 127.0.0.1
Client Port : 34200
Request     : SUBNET 4
[24bcs131@mepcolinux EX6]$cat ex2_client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // Added for close()
#include <sys/socket.h>  // Added for Linux socket API
#include <netinet/in.h>  // Added for sockaddr_in structures
#include <arpa/inet.h>   // Added for inet_pton

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 10240
#define BUFFER_SIZE 1024

int main()
{
    int sockfd; // Changed from SOCKET to standard int

    struct sockaddr_in server_addr;

    char buffer[BUFFER_SIZE];

    socklen_t addr_len; // Changed int to socklen_t for Linux compliance

    int subnet_number;

    /*
     * Create UDP Socket
     */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) // Linux sockets return -1 on error
    {
        printf("Socket creation failed\n");
        return 1;
    }

    /*
     * Clear Server Address
     */
    memset(
        &server_addr,
        0,
        sizeof(server_addr)
    );

    /*
     * Server Address
     */
    server_addr.sin_family = AF_INET;

    server_addr.sin_port =
        htons(SERVER_PORT);

    /*
     * Convert Server IP
     */
    if (inet_pton(
            AF_INET,
            SERVER_IP,
            &server_addr.sin_addr
        ) <= 0)
    {
        printf("Invalid Server IP\n");
        close(sockfd); // Changed closesocket to close
        return 1;
    }

    printf("========== DHCP UDP CLIENT ==========\n");

    printf(
        "Connected to DHCP Server %s:%d\n",
        SERVER_IP,
        SERVER_PORT
    );

    /*
     * Request IP
     */
    printf("\nEnter Required Subnet Number: ");

    scanf("%d", &subnet_number);

    /*
     * Create Request
     */
    sprintf(
        buffer,
        "SUBNET %d",
        subnet_number
    );

    printf(
        "Sending Request: %s\n",
        buffer
    );

    /*
     * Step 10:
     * Send Client Request
     */
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
        close(sockfd); // Changed closesocket to close
        return 1;
    }

    /*
     * Receive UDP Response
     */
    addr_len =
        sizeof(server_addr);

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
        close(sockfd); // Changed closesocket to close
        return 1;
    }

    buffer[bytes_received] = '\0';

    /*
     * Display Assigned IP
     */
    printf(
        "\nServer Response:\n%s\n",
        buffer
    );

    /*
     * Close Socket
     */
    close(sockfd); // Changed closesocket to close

    return 0;
}
