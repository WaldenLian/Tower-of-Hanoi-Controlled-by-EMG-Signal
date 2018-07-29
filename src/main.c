/**
 * Stack Implementation based on LinkedList
 */

#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    float value;
    struct Node *next;
} ListNode;

typedef struct StackHead {
    int size;
    ListNode *top;
} Stack;


Stack *initStack();
void freeStack(Stack *rod);
void pushElement(Stack *rod, ListNode *disk);
ListNode *popElement(Stack *rod);
int getSize(Stack *rod);
int isEmpty(Stack *rod);
ListNode *initNode(float value);
void printStack(Stack *rod);

void printStack(Stack *rod) {
    printf("==========Stack Content==========\n");
    ListNode *it = rod->top;
    while (it) {
        printf("%f ", it->value);
        it = it->next;
    }
    printf("\n=================================\n");
}

ListNode *initNode(float value) {
    ListNode *result = (ListNode *) malloc(sizeof(ListNode));
    result->value = value;
    result->next = NULL;
    return result;
}

Stack *initStack() {
    Stack *result = (Stack *) malloc(sizeof(Stack));
    result->size = 0;
    result->top = NULL;
    return result;
}

void freeStack(Stack *rod) {
    while (rod->size > 0) {
        ListNode *topNode = popElement(rod);
        free(topNode);
    }
    free(rod);
}

void pushElement(Stack *rod, ListNode *disk) {
    rod->size++;
    disk->next = rod->top;
    rod->top = disk;
}

ListNode *popElement(Stack *rod) {
    if (isEmpty(rod) == 1) {
        return NULL;
    } else {
        rod->size--;
        ListNode *topNode = rod->top;
        rod->top = rod->top->next;
        return topNode;
    }
}

int getSize(Stack *rod) {
    return rod->size;
}

int isEmpty(Stack *rod) {
    if (rod->size == 0) {
        return 1;
    } else {
        return 0;
    }
}


int main()
{
    Stack *rod = initStack();
    ListNode *n1 = initNode(1.2);
    ListNode *n2 = initNode(2.4);
    ListNode *n3 = initNode(0.88);

    pushElement(rod, n1);
    pushElement(rod, n2);
    pushElement(rod, n3);
    printStack(rod);

    free(rod);
}