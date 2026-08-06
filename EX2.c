/*=========================================================
                        PART 1
    Structures, default table, global variables, constants
=========================================================*/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define MAXDATA 5000
#define FRAMESIZE 32
#define MAXFRAME 100

struct table {
    char url[50];
    char ip[20];
    char mac[25];
};

struct table route[] = {
    {"www.google.com","192.168.10.1","00:11:22:33:44:55"},
    {"www.amazon.in","192.168.10.2","66:77:88:99:AA:BB"},
    {"www.mepco.edu","192.168.10.3","10:20:30:40:50:60"},
    {"www.yahoo.com","192.168.10.4","AA:BB:CC:DD:EE:FF"},
    {"www.microsoft.com","192.168.10.5","11:22:33:44:55:66"}
};
#define NUM_ROUTES (sizeof(route) / sizeof(route[0]))

struct frame {
    char soh[10];
    int frame_no;
    int length;
    char src_url[50];
    char dest_url[50];
    char src_ip[20];
    char dest_ip[20];
    char src_mac[25];
    char dest_mac[25];
    char data[FRAMESIZE + 1];
    char augmented[FRAMESIZE + 50];
    char crc[50];
    char codeword[FRAMESIZE + 50];
    char quotient[100]; // To store quotient for display
};

struct table src;
struct table dest;
struct frame f[MAXFRAME];

char srcurl[50];
char desturl[50];
char message[500];
char binary[MAXDATA];
char dataword[MAXDATA];
char generator[100];
char generator_binary[100];

int total_frames;
int binary_length;

// Function Declarations
void loadTable();
void getInput();
void searchAddress();
void readMessage();
void messageToBinary();
void polynomialToBinary();
void createFrames();
void sender();
void receiver();

/*=========================================================
                        PART 2
    Get source/destination URL, search table, validate URLs
=========================================================*/
void loadTable() 
{
    printf("\n============================================================\n");
    printf("                       ROUTING TABLE\n");
    printf("============================================================\n");
    printf("%-20s %-20s %-20s\n", "URL", "IP ADDRESS", "MAC ADDRESS");
    printf("------------------------------------------------------------\n");
    for(int i = 0; i < NUM_ROUTES; i++) {
        printf("%-20s %-20s %-20s\n", route[i].url, route[i].ip, route[i].mac);
    }
    printf("============================================================\n");
}

void getInput() 
{
    printf("\nEnter Source URL : ");
    scanf("%s", srcurl);
    
    printf("Enter Destination URL : ");
    scanf("%s", desturl);
    
    printf("Enter Generator Polynomial : ");
    scanf(" %[^\n]", generator);
}

void searchAddress() 
{
    int i, f1 = 0, f2 = 0;

    // Validate Source URL
    while(1) {
        f1 = 0;
        for(i = 0; i < NUM_ROUTES; i++) {
            if(strcmp(route[i].url, srcurl) == 0) {
                src = route[i];
                f1 = 1;
                break;
            }
        }
        if(f1 == 1) break;
        printf("\nSource URL Not Found. Enter Valid Source URL : ");
        scanf("%s", srcurl);
    }

    // Validate Destination URL
    while(1) {
        f2 = 0;
        for(i = 0; i < NUM_ROUTES; i++) {
            if(strcmp(route[i].url, desturl) == 0) {
                dest = route[i];
                f2 = 1;
                break;
            }
        }
        if(f2 == 1) break;
        printf("\nDestination URL Not Found. Enter Valid Destination URL : ");
        scanf("%s", desturl);
    }
}

/*=========================================================
                        PART 3
    Read message from file, ASCII to binary, decimal to binary
=========================================================*/
void readMessage() 
{
    int ch;
    FILE *fp = fopen("message.txt", "r");

    if(fp == NULL) {
        printf("\nError: message.txt Not Found.");
        exit(0);
    }

    message[0] = '\0';
    while((ch = fgetc(fp)) != EOF) {
        int len = strlen(message);
        message[len] = ch;
        message[len + 1] = '\0';
    }
    fclose(fp);
    printf("\nMessage Read Successfully: %s\n", message);
}

void decimalToBinary(int num) 
{
    int arr[8] = {0,0,0,0,0,0,0,0}; // Pre-fill with zeros
    int i = 0, j;
    int rem;
    char temp[2];

    // Manual paper logic as requested
    while(num > 0) {
        rem = num % 2;
        arr[i] = rem;
        num = num / 2;
        i++;
    }

    // Read array backwards to get correct binary order
    for(j = 7; j >= 0; j--) {
        temp[0] = arr[j] + '0';
        temp[1] = '\0';
        strcat(binary, temp);
    }
}

void messageToBinary() 
{
    int i;
    binary[0] = '\0';

    for(i = 0; message[i] != '\0'; i++) {
        decimalToBinary((int)message[i]);
    }

    strcpy(dataword, binary);
    binary_length = strlen(dataword);
}

/*=========================================================
                        PART 4
    Polynomial to binary (manual power checking)
=========================================================*/
void polynomialToBinary() 
{
    int power[20] = {0};
    int i = 0, max = 0, num = 0;

    while(generator[i] != '\0') {
        if(generator[i] == 'x' || generator[i] == 'X') {
            if(generator[i+1] == '^') {
                i += 2; num = 0;
                while(generator[i] >= '0' && generator[i] <= '9') {
                    num = num * 10 + (generator[i] - '0');
                    i++;
                }
                power[num] = 1;
                if(num > max) max = num;
            } else {
                power[1] = 1;
                if(max < 1) max = 1;
                i++;
            }
        }
        else if(generator[i] == '1') { power[0] = 1; i++; }
        else { i++; }
    }

    generator_binary[0] = '\0';
    for(i = max; i >= 0; i--) {
        if(power[i] == 1) strcat(generator_binary,"1");
        else strcat(generator_binary,"0");
    }
}

/*=========================================================
                        PART 5
    Create DDCMP-style frames, generate CRC using XOR
=========================================================*/
void crc_division(char *dividend, char *divisor, char *remainder, char *quotient) 
{
    int d_len = strlen(dividend);
    int g_len = strlen(divisor);
    char temp[100];
    int i, j;

    strncpy(temp, dividend, g_len);
    temp[g_len] = '\0';
    quotient[0] = '\0';

    for (i = g_len - 1; i < d_len; i++) {
        if (i > g_len - 1) {
            int t_len = strlen(temp);
            temp[t_len] = dividend[i];
            temp[t_len + 1] = '\0';
        }

        if (temp[0] == '1') {
            strcat(quotient, "1"); 
            for (j = 1; j < g_len; j++) {
                temp[j] = (temp[j] == divisor[j]) ? '0' : '1';
            }
        } else {
            strcat(quotient, "0"); 
            for (j = 1; j < g_len; j++) {
                temp[j] = (temp[j] == '0') ? '0' : '1'; 
            }
        }

        for (j = 0; j < g_len - 1; j++) {
            temp[j] = temp[j + 1];
        }
        temp[g_len - 1] = '\0';
    }
    strcpy(remainder, temp);
}

void createFrames() 
{
    int i, j, k = 0;
    int len = strlen(dataword);

    total_frames = len / FRAMESIZE;
    if(len % FRAMESIZE != 0) total_frames++;

    for(i = 0; i < total_frames; i++) 
    {
        strcpy(f[i].soh, "01111110");
        f[i].frame_no = i + 1;
        strcpy(f[i].src_url, src.url);
        strcpy(f[i].src_ip, src.ip);
        strcpy(f[i].src_mac, src.mac);
        strcpy(f[i].dest_url, dest.url);
        strcpy(f[i].dest_ip, dest.ip);
        strcpy(f[i].dest_mac, dest.mac);

        int remaining = len - k;
        f[i].length = (remaining >= FRAMESIZE) ? FRAMESIZE : remaining;

        j = 0;
        while(j < FRAMESIZE) {
            if(k < len) {
                f[i].data[j] = dataword[k];
                k++;
            } else {
                f[i].data[j] = '0';
            }
            j++;
        }
        f[i].data[FRAMESIZE] = '\0';

        // Perform CRC Math inside Frame Creation
        int glen = strlen(generator_binary);
        strcpy(f[i].augmented, f[i].data);
        for(j = 0; j < glen - 1; j++) {
            strcat(f[i].augmented, "0");
        }

        crc_division(f[i].augmented, generator_binary, f[i].crc, f[i].quotient);

        strcpy(f[i].codeword, f[i].data);
        strcat(f[i].codeword, f[i].crc);
    }
}

/*=========================================================
                        PART 6
    Sender display, receiver verification, final main()
=========================================================*/
void sender() 
{
    printf("\n=======================================\n");
    printf("APPLICATION LAYER\n");
    printf("=======================================\n");
    printf("Application Layer Processing Completed.\n");

    printf("\n=======================================\n");
    printf("TRANSPORT LAYER\n");
    printf("=======================================\n");
    printf("Message          : %s\n", message);
    printf("Source Port      : 5000\n");
    printf("Destination Port : 8080\n");
    printf("Transport Layer Processing Completed.\n");

    printf("\n=======================================\n");
    printf("NETWORK LAYER\n");
    printf("=======================================\n");
    printf("Packets Created : %d\n", total_frames);
    printf("Network Layer Processing Completed.\n");

    printf("\n=======================================\n");
    printf("DATA LINK LAYER\n");
    printf("=======================================\n");
    printf("\n========== CRC GENERATION ==========\n");

    for(int i = 0; i < total_frames; i++) {
        printf("\n--- Frame %d ---\n", f[i].frame_no);
        printf("Dataword       : %s\n", f[i].data);
        printf("Augmented Data : %s\n", f[i].augmented);
        printf("Quotient       : %s\n", f[i].quotient);
        printf("CRC Remainder  : %s\n", f[i].crc);
    }

    printf("\n========== FRAMES ==========\n");
    for(int i = 0; i < total_frames; i++) {
        printf("\nFrame %d\n", f[i].frame_no);
        printf("---------------------------------------\n");
        printf("SRC IP:%s\n", f[i].src_ip);
        printf("DEST IP:%s\n", f[i].dest_ip);
        printf("SRC MAC:%s\n", f[i].src_mac);
        printf("DEST MAC:%s\n", f[i].dest_mac);
        printf("PAYLOAD : %s\n", f[i].data);
        printf("TRAILER : %s\n", f[i].crc);
        printf("---------------------------------------\n");
    }
    printf("\nData Link Layer Completed.\n");

    printf("\n=======================================\n");
    printf("SUMMARY\n");
    printf("=======================================\n");
    printf("Packets Created : %d\n", total_frames);
    printf("Frames Created  : %d\n", total_frames);
}

void receiver()
{
    int i, j;
    char remainder[50], quotient[100];
    int error_flag;
    char received_data[MAXDATA] = "";
    char ascii_char[9];
    int decimal_val;

    printf("\n=======================================\n");
    printf("       RECEIVER PROCESSING\n");
    printf("=======================================\n");

    for(i = 0; i < total_frames; i++) {
        printf("\n--- Receiving Frame %d ---\n", f[i].frame_no);
        printf("Received Codeword : %s\n", f[i].codeword);
        
        crc_division(f[i].codeword, generator_binary, remainder, quotient);
        
        printf("Decoder Quotient  : %s\n", quotient);
        printf("Syndrome (Rem)    : %s\n", remainder);

        error_flag = 0;
        for(j = 0; j < strlen(remainder); j++) {
            if(remainder[j] == '1') {
                error_flag = 1;
                break;
            }
        }

        if(error_flag == 1) {
            printf("Status: ERROR DETECTED. Frame %d discarded.\n", f[i].frame_no);
        } else {
            printf("Status: NO ERROR (Zero syndrome). Frame %d accepted.\n", f[i].frame_no);
            strncat(received_data, f[i].data, f[i].length); 
        }
    }

    printf("\nExtracted Binary Data:\n%s\n", received_data);
    printf("\nDecoded Message: ");

    for(i = 0; i < strlen(received_data); i += 8) {
        strncpy(ascii_char, &received_data[i], 8);
        ascii_char[8] = '\0';
        decimal_val = 0;
        for(j = 0; j < 8; j++) {
            decimal_val = decimal_val * 2 + (ascii_char[j] - '0');
        }
        printf("%c", decimal_val);
    }
    printf("\n\n");
}

int main()
{
    loadTable();
    getInput();
    searchAddress();
    readMessage();
    messageToBinary();
    polynomialToBinary();
    createFrames();
    sender();
    receiver();
    
    return 0;
}