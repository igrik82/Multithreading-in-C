#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "lib.h"

// Set socket to non-blocking mode
int setNonblock(int fileDescriptor)
{
    int flags;
#if defined(O_NONBLOCK)
    if (-1 == (flags = fcntl(fileDescriptor, F_GETFL, 0)))
        flags = 0;
    return fcntl(fileDescriptor, F_SETFL, O_NONBLOCK);
#else
    flags = 1;
    return ioctl(fileDescriptor, FIONBIO, &flags);
#endif
}

// delete node from list
void delNodes(int socketNode, struct linkedList **List)
{
    if(*List)
    {
        struct linkedList *tempNode = NULL;
        struct linkedList *currentNode = NULL;
        struct linkedList *nextNode = NULL;

        currentNode = (*List);
        nextNode = (*List)->next;
        while ((nextNode) && (currentNode->val != socketNode) && (nextNode->val != socketNode))
        {
            currentNode = nextNode;
            nextNode = nextNode->next;
        }
        if(socketNode == currentNode->val)
        {
            tempNode = currentNode;
            (*List) = nextNode;
            free(tempNode);
            tempNode = NULL;
        }
        else if(socketNode == nextNode->val)
        {
            tempNode = nextNode;
            currentNode->next = nextNode->next;
            free(tempNode);
            tempNode = NULL;
        }
    }
}

// Add nodes to list
void addNodes(int socketNode, struct linkedList **List)
{
    if((*List) == NULL)
    {
        (*List) = (struct linkedList *)malloc(sizeof(struct linkedList));
        (*List)->val = socketNode;
        (*List)->next = NULL;
    }
    else
    {
        struct linkedList *currentNode = NULL;
        struct linkedList *nextNode = NULL;
        struct linkedList *addedNode = (struct linkedList*) malloc(sizeof(struct linkedList));
        addedNode->val = socketNode;
        addedNode->next = NULL;
        currentNode = (*List);
        nextNode = (*List)->next;

        while(nextNode)
        {
            currentNode = nextNode;
            nextNode = nextNode->next;
        }
        currentNode->next = addedNode;
    }
}

// Getting maxValue from slave nodes
int getMaxInClientSocket(struct linkedList *List){
    if(List == NULL)
    {
        return -1;
    }
    else
    {
        int max = List->val;
        while (List)
        {
            if(max < List->val)
            {
                max = List->val;
            }
            List = List->next;
        }
        return max;
    }
}

// Getting max of two numbers
int maxValue(int serverSocket, int clientsSocket)
{
    return(serverSocket > clientsSocket) ? serverSocket : clientsSocket;
}