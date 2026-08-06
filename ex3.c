#include <stdio.h>
#include <math.h>

#define MAX 100

char data[MAX];
char hamming[MAX];
char received[MAX];
char original[MAX];

int dataBits;
int parityBits;
int totalBits;

void getData();
void calculateParityBits();
void placeDataBits();
void calculateParityBitsValue();
void displaySender();

void receiveCode();
void detectAndCorrect();
void extractOriginalData();
void displayReceiver();

int isPowerOfTwo(int n);

int main()
{
    printf("\n========================================");
    printf("\n        HAMMING CODE PROGRAM");
    printf("\n========================================\n");

    getData();

    calculateParityBits();

    placeDataBits();

    calculateParityBitsValue();

    displaySender();

    receiveCode();

    detectAndCorrect();

    extractOriginalData();

    displayReceiver();

    return 0;
}

void getData()
{
    printf("\nEnter Binary Data : ");
    scanf("%s", data);

    dataBits = 0;

    while(data[dataBits] != '\0')
        dataBits++;

    printf("\nNumber of Data Bits : %d\n", dataBits);
}

void calculateParityBits()
{
    parityBits = 0;

    while(pow(2, parityBits) < (dataBits + parityBits + 1))
    {
        parityBits++;
    }

    totalBits = dataBits + parityBits;

    printf("\nParity Bits Required : %d", parityBits);
    printf("\nTotal Hamming Bits   : %d\n", totalBits);
}

int isPowerOfTwo(int n)
{
    if(n == 1)
        return 1;

    while(n > 1)
    {
        if(n % 2 != 0)
            return 0;

        n /= 2;
    }

    return 1;
}
void placeDataBits()
{
    int i, j = 0;

    printf("\nPlacing Data and Parity Bits:\n");

    for(i = 1; i <= totalBits; i++)
    {
        if(isPowerOfTwo(i))
        {
            hamming[i] = '0';      // Temporary parity bit
            printf("Position %2d -> P\n", i);
        }
        else
        {
            hamming[i] = data[j++];
            printf("Position %2d -> %c\n", i, hamming[i]);
        }
    }
}

void calculateParityBitsValue()
{
    int i, j, k;
    int parity;

    printf("\nCalculating Even Parity Bits...\n");

    for(i = 0; i < parityBits; i++)
    {
        int position = (int)pow(2, i);
        parity = 0;

        for(j = position; j <= totalBits; j += 2 * position)
        {
            for(k = j; k < j + position && k <= totalBits; k++)
            {
                if(k != position)
                {
                    if(hamming[k] == '1')
                        parity++;
                }
            }
        }

        if(parity % 2 == 0)
            hamming[position] = '0';
        else
            hamming[position] = '1';

        printf("P%d = %c\n", position, hamming[position]);
    }
}

void displaySender()
{
    int i;

    printf("\n====================================");
    printf("\n         HAMMING SENDER");
    printf("\n====================================");

    printf("\nOriginal Data : ");

    for(i = 0; i < dataBits; i++)
        printf("%c", data[i]);

    printf("\n");

    printf("\nGenerated Hamming Code : ");

    for(i = 1; i <= totalBits; i++)
        printf("%c", hamming[i]);

    printf("\n");
}
void receiveCode()
{
    int i;

    printf("\n====================================");
    printf("\n        HAMMING RECEIVER");
    printf("\n====================================");

    printf("\nEnter Received Hamming Code : ");

    scanf("%s",&received[1]);

    received[totalBits+1]='\0';
}

void detectAndCorrect()
{
    int i,j,k;
    int parity;
    int errorPosition=0;

    printf("\n\nChecking Parity Bits...\n");

    for(i=0;i<parityBits;i++)
    {
        int position=(int)pow(2,i);

        parity=0;

        for(j=position;j<=totalBits;j=j+2*position)
        {
            for(k=j;k<j+position && k<=totalBits;k++)
            {
                if(received[k]=='1')
                    parity++;
            }
        }

        if(parity%2!=0)
        {
            errorPosition+=position;
            printf("Parity Bit P%d : ERROR\n",position);
        }
        else
        {
            printf("Parity Bit P%d : OK\n",position);
        }
    }

    if(errorPosition==0)
    {
        printf("\n\nNo Error Detected.\n");
    }
    else
    {
        printf("\nError Detected at Bit Position : %d\n",errorPosition);

        if(received[errorPosition]=='0')
            received[errorPosition]='1';
        else
            received[errorPosition]='0';

        printf("Error Corrected Successfully.\n");
    }
}

void extractOriginalData()
{
    int i;
    int j=0;

    for(i=1;i<=totalBits;i++)
    {
        if(!isPowerOfTwo(i))
        {
            original[j]=received[i];
            j++;
        }
    }

    original[j]='\0';
}

void displayReceiver()
{
    int i;

    printf("\n====================================");
    printf("\n          FINAL OUTPUT");
    printf("\n====================================");

    printf("\nCorrected Hamming Code : ");

    for(i=1;i<=totalBits;i++)
        printf("%c",received[i]);

    printf("\n");

    printf("\nOriginal Data : %s\n",original);
}