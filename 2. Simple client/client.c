#include <stdio.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_ADDRES "10.0.0.1"
#define SERVER_PORT 12345

int main(int argv, char **argc)
{
    // Socket descriptor
    int socketClient = socket(
        AF_INET,
        SOCK_STREAM,
        0
        );
    //TODO: make error exceptions with errno.h
    if(socketClient == -1)
        return puts("Something wrong with socket.");

    // Server address
    struct sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_ADDRES, &sockAddr.sin_addr); // Function for address server
    sockAddr.sin_port = htons(SERVER_PORT);

    // Connect
    connect(socketClient, (struct sockaddr *)&sockAddr, sizeof(sockAddr));

    // Send message
    char buf[] = "BADABUM!";
    char BUF[80];
    recv(socketClient, BUF, sizeof(BUF), 0); //for linux MSG_NOSIGNAL; for mac 0
    puts(BUF);
    send(socketClient, buf, sizeof(buf), 0);
    recv(socketClient, BUF, sizeof(BUF), 0);
    puts(BUF);

    // Close connection
    shutdown(socketClient, SHUT_RDWR);
    close(socketClient);

}