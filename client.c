#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

int main(int argc , char *argv[]) {
    if (argc < 3) {
        printf("arguments (quoteserver, port) required\n");
        return 1;
    }

    //Translating

    struct addrinfo *serverinfo, hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(argv[1], argv[2], &hints, &serverinfo) != 0) {
        perror("error on getaddrinfo()");
        return 1;
    }

    //Setting up Sockets
    int serversock;
    if ((serversock = socket(serverinfo->ai_family, SOCK_STREAM, 0)) == -1) {
        perror("error on socket()");
        return 1;
    }
    int clientsock;

    //Connecting

    struct addrinfo *temp = serverinfo;
    while (temp != NULL) {
        if ((clientsock = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol)) == -1)
            continue;

        if (connect(clientsock, temp->ai_addr, temp->ai_addrlen) == 0)
            break;

        close(clientsock);
        temp = temp->ai_next;
    }

    if (temp == NULL) {
        printf("error: no server found.\n");
        return 1;
    }

    //Recieving + printing

    char buf[1];

    while (1) {
        int readrv = read(clientsock, buf, 1);
        if (readrv == -1) {
            perror("error on read");
            return 1;
        }
        if (readrv == 0)
            break;
        printf("%c", *buf);
    }
    //printf("\n");

    //Finishing

    close(serversock);
    freeaddrinfo(serverinfo);
    return 0;
}