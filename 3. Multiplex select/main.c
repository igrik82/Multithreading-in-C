#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "include/lib.h"

#define PORT_NUMBER 12345
#define BUFFER_LENGTH 1024

int main(int argc, char **argv)
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket < 0)
    {
        perror("Socket");
        exit(1);
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
        exit(2);
    }


    listen(serverSocket, SOMAXCONN);

    // Initializing linked lists
    struct linkedList *clientSockets = NULL;
    struct linkedList *tempSocket = NULL;

    while(1)
    {
        // set Set list
        fd_set Set;
        FD_ZERO(&Set);

        // Adding server socket to Set list
        FD_SET(serverSocket, &Set);

        // Adding clients socket to Set list
        tempSocket = clientSockets;
        while(tempSocket != NULL)
        {
            FD_SET(tempSocket->val, &Set);
            tempSocket = tempSocket->next;
        }

        // Getting max socket index for defining end Bit in Set list. Otherwise, need to go thru whole Set list(1024 bits).
        int maxSocketIndex = maxValue(serverSocket, getMaxInClientSocket(clientSockets));

        // Reading Set list. When we got new event in Set list
        if(select(maxSocketIndex + 1, &Set, NULL, NULL, NULL) <=0)
        {
            perror("Select");
            exit(3);
        }

        // Looking what happened on client's sockets
        tempSocket = clientSockets;
        while (tempSocket != NULL)
        {
            // If new client's socket in Set
            if(FD_ISSET(tempSocket->val, &Set))
            {
                static char buffer[BUFFER_LENGTH];
                // MSG_NOSIGNAL (in recv func) working on linux but not on MacOS. Zero in both cases.
                int resvSize = recv(tempSocket->val, buffer, BUFFER_LENGTH, 0);
                // If no message or interruption on signal - close connection
                if((resvSize == 0) && (errno != EAGAIN))
                {
                    shutdown(tempSocket->val, SHUT_RDWR);
                    close(tempSocket->val);
                    delNodes(tempSocket->val, &clientSockets);
                }
                // If anyone sent a message
                else if(resvSize != 0)
                {
                    // Sending message all connected clients except the sender
                    struct linkedList *allConnectedSocket = clientSockets;
                    while (allConnectedSocket)
                    {
                        // If sender != connected clients
                        if(tempSocket->val != allConnectedSocket->val)
                        {
                            send(allConnectedSocket->val, buffer, resvSize, 0);
                        }
                        allConnectedSocket = allConnectedSocket->next;
                    }
                }
            }
            tempSocket = tempSocket->next;
        }

        // Looking what happened on server's socket.
        // There can only be an event - request to accept the connection
        if(FD_ISSET(serverSocket, &Set))
        {
            int clientSocket = accept(serverSocket, NULL, NULL);
            if(clientSocket < 0)
            {
                perror("Accept");
                exit(4);
            }
            // Set socket to non-blocking mode and add client socket(node) to linked list.
            setNonblock(serverSocket);
            addNodes(clientSocket, &clientSockets);
        }
    }
}