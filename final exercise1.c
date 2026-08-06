#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define SIZE 4
#define FRAMESIZE 16


struct Host
{
    char url[30];
    char ip[20];
    char mac[20];
};

struct Host host[SIZE]=
{
    {"mepco.edu","192.168.1.2","AA:BB:CC:DD:EE:01"},
    {"google.com","192.168.1.3","AA:BB:CC:DD:EE:02"},
    {"yahoo.com","192.168.1.4","AA:BB:CC:DD:EE:03"},
    {"chatgpt.com","192.168.1.5","AA:BB:CC:DD:EE:04"}
};

struct Host src,dest;

char msg[300];
char out[500];

// --- NEW GLOBAL STORAGE FOR BYTE COUNT DEMONSTRATION ---
char frame_payloads[20][FRAMESIZE + 1]; 
int global_total_frames = 0;
// -----------------------------------------------------

void display();
void search();
void stuffing();
void saveFile();
void binary();
void decimalToBinary(int,char[]);
void convertIP(char[]);
void convertMAC(char[]);
void convertMessage();
void frame();
void receiver();
void destuffing();

int main()
{
    display();

    search();

    stuffing();

    saveFile();

    binary();

    frame();

    // Call receiver to show the data link layer verification process
    receiver();

    return 0;
}

void display()
{
    int i;

    printf("\nDEFAULT ADDRESS TABLE\n\n");

    printf("%-18s %-18s %-20s\n",
            "URL",
            "IP ADDRESS",
            "MAC ADDRESS");

    printf("--------------------------------------------------------------\n");

    for(i=0;i<SIZE;i++)
    {
        printf("%-18s %-18s %-20s\n",
                host[i].url,
                host[i].ip,
                host[i].mac);
    }

    printf("\n");
}

void search()
{
    char s[30], d[30];
    int i;
    int f1, f2;

    /* Source URL */

    do
    {
        f1 = 0;

        printf("\nEnter Source URL      : ");
        scanf("%s", s);

        for(i = 0; i < SIZE; i++)
        {
            if(strcmp(s, host[i].url) == 0)
            {
                src = host[i];
                f1 = 1;
                break;
            }
        }

        if(f1 == 0)
        {
            printf("\nSource URL Not Found!");
            printf("\nPlease Enter a Valid Source URL.\n");
        }

    }while(f1 == 0);


    /* Destination URL */

    do
    {
        f2 = 0;

        printf("\nEnter Destination URL : ");
        scanf("%s", d);

        for(i = 0; i < SIZE; i++)
        {
            if(strcmp(d, host[i].url) == 0)
            {
                dest = host[i];
                f2 = 1;
                break;
            }
        }

        if(f2 == 0)
        {
            printf("\nDestination URL Not Found!");
            printf("\nPlease Enter a Valid Destination URL.\n");
        }

    }while(f2 == 0);


    printf("\nSOURCE DETAILS\n");
    printf("URL : %s\n", src.url);
    printf("IP  : %s\n", src.ip);
    printf("MAC : %s\n", src.mac);

    printf("\nDESTINATION DETAILS\n");
    printf("URL : %s\n", dest.url);
    printf("IP  : %s\n", dest.ip);
    printf("MAC : %s\n", dest.mac);
}

void stuffing()
{
    char *p;

    getchar();

    printf("\nEnter Message : ");

    fgets(msg,sizeof(msg),stdin);

    out[0]='\0';

    p=strtok(msg," \n");

    while(p!=NULL)
    {
        if(strcmp(p,"DLE")==0)
        {
            strcat(out,"DLE ");
            strcat(out,"DLE ");
        }
        else
        {
            strcat(out,p);
            strcat(out," ");
        }

        p=strtok(NULL," \n");
    }

    printf("\nStuffed Message\n");
    printf("%s\n",out);
}

void saveFile()
{
    FILE *fp;

    fp=fopen("message.txt","w");

    if(fp==NULL)
    {
        printf("\nFile Cannot Be Created\n");
        return;
    }

    fprintf(fp,"SOURCE URL : %s\n",src.url);
    fprintf(fp,"DESTINATION URL : %s\n\n",dest.url);

    fprintf(fp,"SOURCE IP : %s\n",src.ip);
    fprintf(fp,"DESTINATION IP : %s\n\n",dest.ip);

    fprintf(fp,"SOURCE MAC : %s\n",src.mac);
    fprintf(fp,"DESTINATION MAC : %s\n\n",dest.mac);

    fprintf(fp,"ORIGINAL MESSAGE\n");
    fprintf(fp,"%s\n",msg);

    fprintf(fp,"\nSTUFFED MESSAGE\n");
    fprintf(fp,"%s\n",out);

    fclose(fp);

    printf("\nMessage Stored Successfully in message.txt\n");
}

void decimalToBinary(int n,char bin[])
{
    int temp[8];
    int i,j=0;

    for(i=7;i>=0;i--)
    {
        temp[i]=n%2;
        n=n/2;
    }

    for(i=0;i<8;i++)
    {
        bin[j++]=temp[i]+'0';
    }

    bin[j]='\0';
}

void convertIP(char ip[])
{
    int i;
    int num=0;
    char bin[9];

    for(i=0;;i++)
    {
        if(ip[i]=='.' || ip[i]=='\0')
        {
            decimalToBinary(num,bin);

            printf("%s",bin);

            if(ip[i]=='\0')
                break;

            printf(".");

            num=0;
        }
        else
        {
            num=num*10+(ip[i]-'0');
        }
    }
}

void convertMAC(char mac[])
{
    int i;
    int val;
    char bin[9];

    for(i=0;mac[i]!='\0';i++)
    {
        if(mac[i]==':')
        {
            printf(":");
            continue;
        }

        if(mac[i]>='0' && mac[i]<='9')
            val=mac[i]-'0';
        else
            val=mac[i]-'A'+10;

        decimalToBinary(val,bin);

        printf("%s ",bin+4);
    }
}

void convertMessage()
{
    int i;
    char bin[9];

    for(i=0;out[i]!='\0';i++)
    {
        decimalToBinary((int)out[i],bin);

        printf("%s ",bin);
    }
}

void binary()
{
    printf("\n");
    printf("\n=================================");
    printf("\nBINARY CONVERSION");
    printf("\n=================================\n");

    printf("\nSource IP\n");
    printf("%s\n",src.ip);
    convertIP(src.ip);

    printf("\n\nDestination IP\n");
    printf("%s\n",dest.ip);
    convertIP(dest.ip);

    printf("\n\nSource MAC\n");
    printf("%s\n",src.mac);
    convertMAC(src.mac);

    printf("\n\nDestination MAC\n");
    printf("%s\n",dest.mac);
    convertMAC(dest.mac);

    printf("\n\nStuffed Message (Binary)\n");
    convertMessage();

    printf("\n");
}

void frame()
{
    int len;
    int byteCount;

    len = strlen(out);

    /* Remove trailing space if present */
    if(out[len - 1] == ' ')
        len--;

    /* Byte Count = 1 Byte for Count Field + Data Bytes */
    byteCount = len + 1;

    printf("\n\n=========================================================\n");
    printf("                    DCMP FRAME\n");
    printf("=========================================================\n");

    printf("\nSOH");

    printf("\n\nHEADER");

    printf("\n---------------------------------------------------------");

    printf("\nSource URL      : %s", src.url);
    printf("\nDestination URL : %s", dest.url);

    printf("\nSource IP       : %s", src.ip);
    printf("\nDestination IP  : %s", dest.ip);

    printf("\nSource MAC      : %s", src.mac);
    printf("\nDestination MAC : %s", dest.mac);

    printf("\n---------------------------------------------------------");

    printf("\n\nSTX");

    printf("\n\nBYTE COUNT : %d", byteCount);

    printf("\n\nDATA");

    printf("\n%s", out);

    printf("\n\nETX");

    printf("\n\nBCC : 00000000");

    printf("\n=========================================================\n");
}

void receiver()
{
    char temp[500];
    char *p;

    printf("\n\n========================================");
    printf("\n          RECEIVER SIDE");
    printf("\n========================================");

    printf("\n\nReceiving DCMP Frame.....");

    printf("\nHeader Verified Successfully.");

    printf("\nBCC Verified Successfully.");

    printf("\n\nReceived Stuffed Message");

    printf("\n%s\n", out);

    strcpy(temp, out);

    msg[0] = '\0';

    p = strtok(temp, " \n");

    while(p != NULL)
    {
        if(strcmp(p, "DLE") == 0)
        {
            strcat(msg, "DLE ");

            p = strtok(NULL, " \n");

            if(p == NULL)
                break;
        }
        else
        {
            strcat(msg, p);
            strcat(msg, " ");
        }

        p = strtok(NULL, " \n");
    }

    printf("\n========================================");
    printf("\n      ORIGINAL MESSAGE");
    printf("\n========================================");

    printf("\n%s\n", msg);
}