[24bcs131@mepcolinux ex5]$cat server1.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int sockfd, clientfd;
struct sockaddr_in server, client;
char data[100];

int createServer()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Socket creation failed\n");
        return -1;
    }
    printf("Server socket created\n");
    return 0;
}

int bindServer(int port)
{
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Bind failed\n");
        return -1;
    }

    printf("Server bind successful on port %d\n", port);
    return 0;
}

int waitClient()
{
    if (listen(sockfd, 5) < 0)
    {
        printf("Listen failed\n");
        return -1;
    }

    printf("Waiting for client...\n");

    socklen_t len = sizeof(client);
    clientfd = accept(sockfd, (struct sockaddr *)&client, &len);

    if (clientfd < 0)
    {
        printf("Client connection failed\n");
        return -1;
    }

    printf("Client connected\n");
    return 0;
}

void echoData()
{
    memset(data, 0, sizeof(data));
    int bytes_received = recv(clientfd, data, sizeof(data) - 1, 0);

    if (bytes_received <= 0)
    {
        printf("Client disconnected or error occurred\n");
        return;
    }

    printf("Received from client: %s\n", data);

    send(clientfd, data, strlen(data), 0);
    printf("String sent back to client\n");
}

void closeServer()
{
    if (clientfd >= 0) close(clientfd);
    if (sockfd >= 0) close(sockfd);
    printf("Connection closed\n");
}

int main(int argc, char *argv[])
{
    // Expecting: ./server <Port>
    if (argc != 2)
    {
        printf("Usage: %s <Port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]); // Convert port string to integer

    if (createServer() < 0) return 1;
    if (bindServer(port) < 0) return 1;
    if (waitClient() < 0) return 1;

    echoData();
    closeServer();

    return 0;
}

[24bcs131@mepcolinux ex5]$cat client1.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int sockfd;
struct sockaddr_in server;
char data[100];
char reply[100];

int createClient()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Socket creation failed\n");
        return -1;
    }
    printf("Client socket created\n");
    return 0;
}

int connectServer(const char *ip, int port)
{
    server.sin_family = AF_INET;

    // Convert IP from text to binary format
    if (inet_pton(AF_INET, ip, &server.sin_addr) <= 0)
    {
        printf("Invalid IP address format\n");
        return -1;
    }

    server.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Connection failed\n");
        return -1;
    }

    printf("Connected to server at %s:%d\n", ip, port);
    return 0;
}

void sendData()
{
    printf("Enter a string: ");
    fgets(data, sizeof(data), stdin);
    data[strcspn(data, "\n")] = '\0';

    send(sockfd, data, strlen(data), 0);
}

void receiveData()
{
    memset(reply, 0, sizeof(reply));
    int bytes_received = recv(sockfd, reply, sizeof(reply) - 1, 0);

    if (bytes_received > 0)
    {
        printf("Echo from server: %s\n", reply);
    }
    else
    {
        printf("Server closed connection or error occurred\n");
    }
}

void closeClient()
{
    close(sockfd);
    printf("Connection closed\n");
}

int main(int argc, char *argv[])
{
    // Expecting: ./client <Server_IP> <Port>
    if (argc != 3)
    {
        printf("Usage: %s <Server_IP> <Port>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]); // Convert port string to integer

    if (createClient() < 0) return 1;
    if (connectServer(server_ip, port) < 0) return 1;

    sendData();
    receiveData();
    closeClient();

    return 0;
}
