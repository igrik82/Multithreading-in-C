#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <string.h>

#include "include/lib.h"


#define PORT_NUMBER 12345
#define BUFFER_LENGTH 1024
#define POLL_SIZE 3000

int main(int argc, char **argv)
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket < 0)
    {
        perror("Socket");
        exit(EXIT_FAILURE);
    }

    // Set socket to non-blocking mode
    setNonblock(serverSocket);

    struct sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(PORT_NUMBER);
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if((bind(serverSocket, (struct sockaddr*) &sockAddr, sizeof(sockAddr))) < 0)
    {
        perror("Bind");
        exit(EXIT_FAILURE);
    }

    listen(serverSocket, SOMAXCONN);

    // Initializing linked lists
    struct linkedList *clientSockets = NULL;
    struct linkedList *tempSocket = NULL;

    while(1)
    {
        // Initializing POLL SET & adding server socket to Set list
        struct pollfd Set[POLL_SIZE];
        Set[0].fd = serverSocket;
        Set[0].events = POLLIN;

        // Adding clients socket to Set list
        unsigned int counter = 1;
        tempSocket = clientSockets;
        while(tempSocket != NULL)
        {
            Set[counter].fd = tempSocket->val;
            Set[counter++].events = POLLIN;
            // If nickname empty
            if(tempSocket->nickName == NULL)
            {
                // If we not get nickname then set counter down
                if(nickNameRequest(tempSocket->val, &clientSockets) < 0)
                {
                    counter--;
                }
            }
            tempSocket = tempSocket->next;
        }

        // Waiting for any events on a file descriptor
        if(poll(Set, counter, -1) < 0)
        {
            perror("POLL");
            exit(EXIT_FAILURE);
        }

        // Looking what happened on client's sockets
        tempSocket = clientSockets;
        int newCounter = 1;
        while ((tempSocket != NULL))
        {
            // If new client's socket in Set
            if(Set[newCounter++].revents && POLLIN)
            {
                static char buffer[BUFFER_LENGTH];
                // MSG_NOSIGNAL (in recv func) working on linux but not on MacOS. Zero in both cases.
                int recvSize = recv(tempSocket->val, buffer, BUFFER_LENGTH, 0);
                // If no message or interruption on signal - close connection
                if((recvSize == 0) && (errno != EAGAIN))
                {
                    shutdown(tempSocket->val, SHUT_RDWR);
                    close(tempSocket->val);
                    delNodes(tempSocket->val, &clientSockets);
                }
                // If anyone sent a message
                else if(recvSize != 0)
                {
                    // Sending message all connected clients except the sender
                    struct linkedList *allConnectedSocket = clientSockets;
                    while (allConnectedSocket)
                    {
                        // If sender != connected clients
                        if(tempSocket->val != allConnectedSocket->val)
                        {
                            send(allConnectedSocket->val, tempSocket->nickName , strlen(tempSocket->nickName), 0);
                            send(allConnectedSocket->val, buffer, recvSize, 0);
                        }
                        allConnectedSocket = allConnectedSocket->next;
                    }
                }
            }
            tempSocket = tempSocket->next;
        }

        // Looking what happened on server's socket.
        // There can only be an event - request to accept the connection
        if(Set[0].revents && POLLIN)
        {
            int clientSocket = accept(serverSocket, NULL, NULL);
            if(clientSocket < 0)
            {
                perror("Accept");
                exit(EXIT_FAILURE);
            }
            // Set socket to non-blocking mode and add client socket(node) to linked list.
            setNonblock(serverSocket);
            addNodes(clientSocket, &clientSockets);
        }
    }
}