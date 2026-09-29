#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define SERV_PORT 7034
#define MAXLEN    2048
char generator[64];
int  genLen;
char work[MAXLEN];
char crcRemainder[64];
void modulo2Divide(void)
{
    int i, j, n;

    n = strlen(work);
    for (i = 0; i <= n - genLen; i++)
    {
        if (work[i] == '1')
        {
            for (j = 0; j < genLen; j++)
                work[i + j] = (work[i + j] == generator[j]) ? '0' : '1';
        }
    }
    strcpy(crcRemainder, work + (n - (genLen - 1)));
}

int main(void)
{
    int ls, s;
    int waitSize = 16;
    socklen_t clntAddrLen;
    struct sockaddr_in servAddr, clientAddr;
    int  msgLen;
    char buffer[MAXLEN];
    char reply[64];
    char *sep;
    int  i, allZero;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family      = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port        = htons(SERV_PORT);
   if ((ls = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error: Listen socket failed!");
        exit(1);
    }
    if (bind(ls, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("Error: Binding failed!");
        exit(1);
    }
  if (listen(ls, waitSize) < 0)
    {
        perror("Error: Listening failed!");
        exit(1);
    }

    printf("CRC TCP Server running on port %d...\n", SERV_PORT);
    clntAddrLen = sizeof(clientAddr);
        if ((s = accept(ls, (struct sockaddr *)&clientAddr, &clntAddrLen)) < 0)
        {
            perror("Error: Accept failed!");

        }
       recv(s, &msgLen, sizeof(int), 0);
        recv(s, buffer, msgLen, 0);
        buffer[msgLen] = '\0';

       sep = strchr(buffer, '|');
        if (sep == NULL)
        {
            printf("Malformed message received, ignoring.\n");
            close(s);

        }
        *sep = '\0';
        strcpy(generator, buffer);
        genLen = strlen(generator);
        strcpy(work, sep + 1);

        printf("\n----- CRC Verification -----\n");
        printf("Generator  : %s\n", generator);
        printf("Codeword   : %s\n", work);
       modulo2Divide();
        printf("Remainder  : %s\n", crcRemainder);
        allZero = 1;
        for (i = 0; i < genLen - 1; i++)
        {
            if (crcRemainder[i] != '0')
            {
                allZero = 0;
                break;
            }
        }
        if (allZero)
            strcpy(reply, "No error detected - packet accepted");
        else
            strcpy(reply, "Error detected - packet discarded");

        printf("Result     : %s\n", reply);
        printf("-----------------------------\n");
        send(s, reply, strlen(reply) + 1, 0);
        close(s);
    return 0;
}

[24bcs131@mepcolinux ex5]$cat client3.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define MAXLEN 2048
char generator[64];
int  genLen;
char data[200];
char binaryData[1600];
char work[1700];
char crcRemainder[64];
char codeword[1700];
char message[1800];
void parsePolynomial(const char* polyInput, char* outputBinary)
{
    int  coeffs[64] = {0};
    int  maxDegree = 0;
    char copy[100];
    strncpy(copy, polyInput, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';
    char* token = strtok(copy, "+");
    while (token != NULL)
    {
        int deg = 0;
        char* xPtr = strchr(token, 'x');
        if (!xPtr) xPtr = strchr(token, 'X');

        if (xPtr)
        {
            char* caretPtr = strchr(xPtr, '^');
            if (caretPtr)
                deg = atoi(caretPtr + 1);
            else
                deg = 1;
        }
        else if (strchr(token, '1'))
        {
            deg = 0;
        }
        if (deg >= 0 && deg < 64)
        {
            coeffs[deg] = 1;
            if (deg > maxDegree) maxDegree = deg;
        }
        token = strtok(NULL, "+");
    }
    int idx = 0, i;
    for (i = maxDegree; i >= 0; i--)
        outputBinary[idx++] = coeffs[i] ? '1' : '0';
    outputBinary[idx] = '\0';
}
void textToBinaryString(const char* text, char* output)
{
    output[0] = '\0';
    int i, bit;
    for (i = 0; text[i] != '\0'; i++)
    {
        char ch = text[i];
        for (bit = 7; bit >= 0; bit--)
            strcat(output, ((ch >> bit) & 1) ? "1" : "0");
    }
}
void modulo2Divide(void)
{
    int i, j, n;
    n = strlen(work);
    for (i = 0; i <= n - genLen; i++)
    {
        if (work[i] == '1')
        {
            for (j = 0; j < genLen; j++)
                work[i + j] = (work[i + j] == generator[j]) ? '0' : '1';
        }
    }
    strcpy(crcRemainder, work + (n - (genLen - 1)));
}
int main(int argc, char* argv[])
{
    int  s, i, pos, msgLen;
    char* servName;
    int  servPort;
    struct sockaddr_in servAddr;
    char rawPolyInput[100];
    char fileName[100];
    char choice;
    FILE* fp;
    char reply[64];
    if (argc != 4)
    {
        printf("Error: three arguments are needed! Usage: %s <ServerIP> <Port> <Filename>\n", argv[0]);
        exit(1);
    }
    servName = argv[1];
    servPort = atoi(argv[2]);
    strncpy(fileName, argv[3], sizeof(fileName) - 1);
    fileName[sizeof(fileName) - 1] = '\0';
    printf("Enter generator polynomial (e.g. x^3+x+1): ");
    fgets(rawPolyInput, sizeof(rawPolyInput), stdin);
    rawPolyInput[strcspn(rawPolyInput, "\n")] = '\0';
    parsePolynomial(rawPolyInput, generator);
    genLen = strlen(generator);
    printf("Polynomial Binary : %s (Length: %d)\n\n", generator, genLen);
    fp = fopen(fileName, "r");
    if (fp == NULL)
    {
        perror("Error: could not open input file!");
        exit(1);
    }
    if (fgets(data, sizeof(data), fp) == NULL)
    {
        printf("Error: input file is empty!\n");
        exit(1);
    }
    data[strcspn(data, "\r\n")] = '\0';
    fclose(fp);
    printf("Data              : %s\n", data);
    textToBinaryString(data, binaryData);
    printf("Binary form       : %s\n\n", binaryData);
    strcpy(work, binaryData);
    for (i = 0; i < genLen - 1; i++)
        strcat(work, "0");
    modulo2Divide();
    strcpy(codeword, binaryData);
    strcat(codeword, crcRemainder);
    printf("Generated CRC bits : %s\n", crcRemainder);
    printf("Codeword to send   : %s\n\n", codeword);
    printf("Do you want to introduce an error by flipping a bit? (y/n): ");
    scanf(" %c", &choice);
    if (choice == 'y' || choice == 'Y')
    {
        printf("Enter bit position to flip (1 to %d): ", (int)strlen(codeword));
        scanf("%d", &pos);
        pos = pos - 1;
        if (pos >= 0 && pos < (int)strlen(codeword))
            codeword[pos] = (codeword[pos] == '0') ? '1' : '0';
        printf("Modified codeword  : %s\n\n", codeword);
    }
    strcpy(message, generator);
    strcat(message, "|");
    strcat(message, codeword);
    msgLen = strlen(message);
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    inet_pton(AF_INET, servName, &servAddr.sin_addr);
    servAddr.sin_port = htons(servPort);
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error: socket creation failed!");
        exit(1);
    }
    if (connect(s, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("Error: connection failed!");
        exit(1);
    }
    send(s, &msgLen, sizeof(int), 0);
    send(s, message, msgLen, 0);
    recv(s, reply, sizeof(reply), 0);
    printf("Server response    : %s\n", reply);
    close(s);
    exit(0);
}
