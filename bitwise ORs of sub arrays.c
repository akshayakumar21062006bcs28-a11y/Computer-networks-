#include <stdlib.h>
#include <string.h>

#define HASH_SIZE 262144

typedef struct Node {
    int val;
    struct Node *next;
} Node;

int hashExists(Node **table, int val) {
    int h = ((unsigned int)val) % HASH_SIZE;
    Node *p = table[h];
    while (p) {
        if (p->val == val)
            return 1;
        p = p->next;
    }
    return 0;
}

void hashInsert(Node **table, int val) {
    int h = ((unsigned int)val) % HASH_SIZE;
    Node *node = (Node *)malloc(sizeof(Node));
    node->val = val;
    node->next = table[h];
    table[h] = node;
}

int subarrayBitwiseORs(int* arr, int arrSize) {
    Node *global[HASH_SIZE];
    memset(global, 0, sizeof(global));

    int prev[32], prevSize = 0;
    int answer = 0;

    for (int i = 0; i < arrSize; i++) {
        int curr[32];
        int currSize = 0;

        curr[currSize++] = arr[i];

        for (int j = 0; j < prevSize; j++) {
            int x = prev[j] | arr[i];
            if (curr[currSize - 1] != x)
                curr[currSize++] = x;
        }

        prevSize = 0;
        for (int j = 0; j < currSize; j++) {
            prev[prevSize++] = curr[j];

            if (!hashExists(global, curr[j])) {
                hashInsert(global, curr[j]);
                answer++;
            }
        }
    }

    return answer;
}