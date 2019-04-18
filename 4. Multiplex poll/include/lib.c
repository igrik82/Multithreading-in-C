#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
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
            free(tempNode->nickName);
            tempNode->nickName = NULL;
            free(tempNode);
            tempNode = NULL;
        }
        else if(socketNode == nextNode->val)
        {
            tempNode = nextNode;
            currentNode->next = nextNode->next;
            free(tempNode->nickName);
            tempNode->nickName = NULL;
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

// Nickname request and add to node
int nickNameRequest(int clientSocket, struct linkedList **List)
{
    // set request to enter nickname
    char *nickNameRequest = "Enter you nickname:\n> ";
    int strLength = strlen(nickNameRequest);
    send(clientSocket, nickNameRequest, strLength , 0);

    // get nickname
    char buffer[50] = {'\0'};
    recv(clientSocket, buffer, 50, 0);

    // if it's empty closing the connection
    if(strncmp(buffer, "\r\n", 2) == 0)
    {
        send(clientSocket, "You have been kicked off.\n", 26 , 0);
        shutdown(clientSocket, SHUT_RDWR);
        close(clientSocket);
        delNodes(clientSocket, List);
        return -1;
    }

    //get rid of \r\n symbols
    for(size_t i = 0; buffer[i] != '\0'; ++i)
    {
        if(buffer[i] == '\r')
        {
            buffer[i] = ':';
        }
        else if(buffer[i] == '\n')
        {
            buffer[i] = ' ';
        }
    }

    // coping nickname from buffer
    strLength = strlen(buffer);
    char *nickname = (char*)malloc((sizeof(char) * strLength) + 1 );
    strcpy(nickname, buffer);

    // finding client socket from node list
    struct linkedList *tempNode = (*List);
    while (tempNode)
    {
        // If node founded setting up a nickname
        if(tempNode->val == clientSocket)
        {
            char *nickAccept = "Ok. Nickname accepted.\n";
            tempNode->nickName = nickname;
            send(tempNode->val, nickAccept, strlen(nickAccept), 0);
            return 0;
        }
        else
        {
            tempNode = tempNode->next;
        }
    }
    return 0;
}