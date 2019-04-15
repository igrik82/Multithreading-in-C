#include <stdio.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main (int argv, char **argc)
{
    // Doing socket descriptor
    int masterSocket = socket(
        AF_INET, //IPv4
        SOCK_STREAM, //TCP
        0 //Auto
        );

    //TODO: make error exceptions with errno.h
    if(masterSocket == -1)
        return puts("Something wrong with socket.");

    // Bind socket
    struct sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(12345);
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY); //for all available addresses

    int bindSocket = bind(masterSocket, (struct sockaddr*)(&sockAddr), sizeof(sockAddr));
    //TODO: make error exceptions with errno.h
    if(bindSocket == -1)
        return puts("Something wrong with socket.");

    // Listen socket
    int listenSocket = listen(masterSocket, SOMAXCONN);

    while(1)
    {
        int slaveSocket = accept(masterSocket, 0 , 0);

        send(slaveSocket,"Server made by igrik is running...\n", 36, MSG_NOSIGNAL);
        char Buffer[20] = {0};
        recv(slaveSocket, Buffer, 19, MSG_NOSIGNAL);
        send(slaveSocket,"FUCK OFF!!!\n", 13, MSG_NOSIGNAL);
        shutdown(slaveSocket, SHUT_RDWR);
        close(slaveSocket);
    }
}