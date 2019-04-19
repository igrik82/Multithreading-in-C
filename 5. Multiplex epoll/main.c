#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <string.h>

#include "include/lib.h"


#define PORT_NUMBER 12345
#define BUFFER_LENGTH 1024
#define MAX_EVENTS 32

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

    // Initializing EPOLL descriptor & adding server socket to Set list
    int ePoll = epoll_create1(0);
    struct epoll_event event;

    // Registration of server socket to EPOLL
    event.data.fd = serverSocket;
    event.events = EPOLLIN;
    epoll_ctl(ePoll, EPOLL_CTL_ADD, serverSocket, &event);

    while(1)
    {
        // Initialize struct for events from clients
        struct epoll_event events[MAX_EVENTS];
        int quantityEvents = epoll_wait(ePoll, events, MAX_EVENTS, -1);

        for(size_t i = 0; i < quantityEvents; ++i)
        {

            // Looking what happened on server's socket.
            // There can only be an event - request to accept the connection
            if(events[i].data.fd == serverSocket)
            {
                int clientSocket = accept(serverSocket, 0, 0);
                if(clientSocket < 0)
                {
                    perror("Accept");
                    exit(EXIT_FAILURE);
                }
                // Set socket to non-blocking mode
                setNonblock(serverSocket);

                //Registration of client socket
                event.data.fd = clientSocket;
                event.events = EPOLLIN;
                epoll_ctl(ePoll, EPOLL_CTL_ADD, clientSocket, &event);

                // Add client socket(node) to linked list.
                addNodes(clientSocket, &clientSockets);

                // Adding nick name to client
                nickNameRequest(clientSocket, &clientSockets);

            }
            else
            {
                static char buffer[BUFFER_LENGTH];
                // MSG_NOSIGNAL (in recv func) working on linux but not on MacOS. Zero working in both cases.
                int recvSize = recv(events[i].data.fd, buffer, BUFFER_LENGTH, MSG_NOSIGNAL);
                // If no message or interruption on signal - close connection
                if((recvSize == 0) && (errno != EAGAIN))
                {
                    shutdown(events[i].data.fd, SHUT_RDWR);
                    close(events[i].data.fd);
                    delNodes(events[i].data.fd, &clientSockets);
                }
                // If anyone sent a message
                else if(recvSize != 0)
                {
                    // Sending message all connected clients except the sender
                    struct linkedList *allConnectedSocket = clientSockets;
                    while (allConnectedSocket)
                    {
                        // Get sender nick
                        char *nickName = getNick(events[i].data.fd, clientSockets);

                        // If sender != connected clients
                        if(events[i].data.fd != allConnectedSocket->val)
                        {
                            send(allConnectedSocket->val, nickName,
                                strlen(nickName), 0);
                            send(allConnectedSocket->val, buffer, recvSize, 0);
                        }
                        allConnectedSocket = allConnectedSocket->next;
                    }
                }
            }
        }
    }
}