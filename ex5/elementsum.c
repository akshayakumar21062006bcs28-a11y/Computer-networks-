[24bcs131@mepcolinux ex5]$cat server2.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int sockfd, clientfd;
struct sockaddr_in server, client;

int a[20];
int n,i;
int sum;

/* Create server socket */
void createServer()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        printf("Socket creation failed\n");
        exit(1);
    }

    printf("Server socket created\n");
}

/* Bind server using port from argument */
void bindServer(int port)
{
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Bind failed\n");
        exit(1);
    }

    printf("Server bind successful on port %d\n", port);
}

/* Wait for client */
void waitClient()
{
    listen(sockfd, 5);

    printf("Waiting for client...\n");

    socklen_t len = sizeof(client);

    clientfd = accept(sockfd, (struct sockaddr *)&client, &len);

    if (clientfd < 0)
    {
        printf("Client connection failed\n");
        exit(1);
    }

    printf("Client connected\n");
}

/* Receive array */
void receiveData()
{
    recv(clientfd, &n, sizeof(n), 0);
    recv(clientfd, a, sizeof(a), 0);

    printf("\nReceived array:\n");

    for (i = 0; i < n; i++)
    {
        printf("%d ", a[i]);
    }

    printf("\n");
}

/* Find sum */
void findSum()
{
    sum = 0;

    for (i = 0; i < n; i++)
    {
        sum = sum + a[i];
    }

    printf("Sum = %d\n", sum);
}

/* Send sum */
void sendResult()
{
    send(clientfd, &sum, sizeof(sum), 0);

    printf("Sum sent to client\n");
}

/* Close connection */
void closeServer()
{
    close(clientfd);
    close(sockfd);

    printf("Connection closed\n");
}

int main(int argc, char *argv[])
{
    // Check if the port argument is provided
    if (argc != 2)
    {
        printf("Usage: %s <Port_Number>\n", argv[0]);
        printf("Example: %s 8080\n", argv[0]);
        return 1;
    }

    // Convert string argument to integer port number
    int port = atoi(argv[1]);

    createServer();
    bindServer(port);
    waitClient();
    receiveData();
    findSum();
    sendResult();
    closeServer();

    return 0;
}

[24bcs131@mepcolinux ex5]$cat client2.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int sockfd;
struct sockaddr_in server;

int a[20];
int n,i;
int sum;

/* Create client socket */
void createClient()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        printf("Socket creation failed\n");
        exit(1);
    }

    printf("Client socket created\n");
}

/* Connect to server using IP and Port from arguments */
void connectServer(char *ip_address, int port)
{
    server.sin_family = AF_INET;

    // Use the dynamic IP address passed from the command line
    if (inet_pton(AF_INET, ip_address, &server.sin_addr) <= 0)
    {
        printf("Invalid IP address format\n");
        exit(1);
    }

    server.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Connection failed\n");
        exit(1);
    }

    printf("Connected to server %s on port %d\n", ip_address, port);
}

/* Get array */
void getData()
{
    printf("Enter number of elements: ");
    scanf("%d", &n);

    printf("Enter %d elements:\n", n);

    for (i = 0; i < n; i++)
    {
        scanf("%d", &a[i]);
    }
}

/* Send array */
void sendData()
{
    send(sockfd, &n, sizeof(n), 0);
    send(sockfd, a, sizeof(a), 0);

    printf("Array sent to server\n");
}

/* Receive sum */
void receiveResult()
{
    recv(sockfd, &sum, sizeof(sum), 0);

    printf("Sum received from server = %d\n", sum);
}

/* Close connection */
void closeClient()
{
    close(sockfd);

    printf("Connection closed\n");
}

int main(int argc, char *argv[])
{
    // Check if both Server IP and Port arguments are provided
    if (argc != 3)
    {
        printf("Usage: %s <Server_IP> <Port_Number>\n", argv[0]);
        printf("Example: %s 127.0.0.1 8080\n", argv[0]);
        return 1;
    }

    char *ip_address = argv[1];
    int port = atoi(argv[2]);

    createClient();
    connectServer(ip_address, port);
    getData();
    sendData();
    receiveResult();
    closeClient();

    return 0;
}
