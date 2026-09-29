 ftpc.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int client_socket;
    struct sockaddr_in server_address;

    char server_ip[50];
    char file_name[256];
    char buffer[BUFFER_SIZE];

    FILE *file;
    int bytes_received;
    long total_bytes = 0;

    /* Create socket */
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    printf("====================================\n");
    printf("       TCP FILE CLIENT\n");
    printf("====================================\n");

    /* Get server IP */
    printf("Enter Server IP Address: ");
    scanf("%49s", server_ip);

    /* Get requested file */
    printf("Enter file name to download: ");
    scanf("%255s", file_name);

    /* Server address */
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr(server_ip);

    /* Check IP */
    if (server_address.sin_addr.s_addr == INADDR_NONE)
    {
        printf("Invalid IP address\n");
        close(client_socket);
        return 1;
    }

    /* Connect to server */
    if (connect(client_socket,
                (struct sockaddr *)&server_address,
                sizeof(server_address)) < 0)
    {
        printf("Connection failed\n");
        close(client_socket);
        return 1;
    }

    printf("Connected to server.\n");

    /* Send requested file name */
    send(client_socket,
         file_name,
         strlen(file_name),
         0);

    /* Receive response from server */
    memset(buffer, 0, BUFFER_SIZE);

    bytes_received = recv(client_socket,
                          buffer,
                          BUFFER_SIZE - 1,
                          0);

    if (bytes_received <= 0)
    {
        printf("Server disconnected.\n");
        close(client_socket);
        return 1;
    }

    buffer[bytes_received] = '\0';

    /* Check whether file is available */
    if (strcmp(buffer, "FILE_NOT_FOUND") == 0)
    {
        printf("File is not available on server.\n");

        close(client_socket);
        return 0;
    }

    /* File exists */
    printf("File found. Downloading %s...\n", file_name);

    file = fopen(file_name, "wb");

    if (file == NULL)
    {
        printf("Cannot create file.\n");
        close(client_socket);
        return 1;
    }

    /* The first response contains FILE_FOUND */
    /* Receive actual file data */
    while ((bytes_received = recv(client_socket,
                                  buffer,
                                  BUFFER_SIZE,
                                  0)) > 0)
    {
        fwrite(buffer, 1, bytes_received, file);
        total_bytes += bytes_received;
    }

    fclose(file);

    printf("File downloaded successfully.\n");
    printf("File name : %s\n", file_name);
    printf("File size : %ld bytes\n", total_bytes);

    close(client_socket);

    printf("Connection closed.\n");

    return 0;
}
[24bcs131@mepcolinux ex7]$cat ftps.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int server_socket;

/* Stop server */
void stop_server(int signal_number)
{
    printf("\nServer stopped.\n");

    close(server_socket);

    exit(0);
}

int main()
{
    int client_socket;
    int process_id;

    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    socklen_t client_length;

    char file_name[256];
    char buffer[BUFFER_SIZE];

    signal(SIGINT, stop_server);
    signal(SIGTSTP, stop_server);

    /* Create socket */
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    /* Reuse port */
    int option = 1;

    setsockopt(server_socket,
               SOL_SOCKET,
               SO_REUSEADDR,
               &option,
               sizeof(option));

    /* Server address */
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    /* Bind */
    if (bind(server_socket,
             (struct sockaddr *)&server_address,
             sizeof(server_address)) < 0)
    {
        printf("Bind failed\n");

        close(server_socket);
        return 1;
    }

    /* Listen */
    if (listen(server_socket, 10) < 0)
    {
        printf("Listen failed\n");

        close(server_socket);
        return 1;
    }

    printf("====================================\n");
    printf("       TCP FILE SERVER\n");
    printf("====================================\n");

    printf("Server started...\n");
    printf("Port : %d\n", PORT);
    printf("Waiting for clients...\n\n");

    while (1)
    {
        client_length = sizeof(client_address);

        /* Accept client */
        client_socket = accept(
            server_socket,
            (struct sockaddr *)&client_address,
            &client_length
        );

        if (client_socket < 0)
        {
            printf("Accept failed\n");
            continue;
        }

        printf("Client connected: %s\n",
               inet_ntoa(client_address.sin_addr));

        /* Create child process */
        process_id = fork();

        if (process_id < 0)
        {
            printf("Fork failed\n");

            close(client_socket);
            continue;
        }

        /* Child process */
        if (process_id == 0)
        {
            FILE *file;

            int bytes_received;
            int bytes_read;
            long total_bytes = 0;

            close(server_socket);

            /* Receive requested file name */
            memset(file_name, 0, sizeof(file_name));

            bytes_received = recv(
                client_socket,
                file_name,
                sizeof(file_name) - 1,
                0
            );

            if (bytes_received <= 0)
            {
                printf("File request failed\n");

                close(client_socket);
                exit(0);
            }

            file_name[bytes_received] = '\0';

            printf("Client requested: %s\n", file_name);

            /* Check if file exists */
            file = fopen(file_name, "rb");

            if (file == NULL)
            {
                printf("File not available.\n");

                send(client_socket,
                     "FILE_NOT_FOUND",
                     14,
                     0);

                close(client_socket);
                exit(0);
            }

            /* File exists */
            printf("File found. Sending file...\n");

            send(client_socket,
                 "FILE_FOUND",
                 10,
                 0);

            /* Send file */
            while ((bytes_read = fread(
                        buffer,
                        1,
                        BUFFER_SIZE,
                        file)) > 0)
            {
                send(client_socket,
                     buffer,
                     bytes_read,
                     0);

                total_bytes += bytes_read;
            }

            fclose(file);

            printf("File sent successfully.\n");
            printf("File name : %s\n", file_name);
            printf("File size : %ld bytes\n", total_bytes);

            /* Finish sending */
            shutdown(client_socket, SHUT_WR);

            close(client_socket);

            exit(0);
        }

        /* Parent process */
        else
        {
            close(client_socket);

            printf("Request handled by child process.\n");
        }
    }

    close(server_socket);

    return 0;
}
