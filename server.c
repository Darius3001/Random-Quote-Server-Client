#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

char* randomquote(char* filepath);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("arguments (port, filename) required\n");
        return 1;
    }

    //Loading quote

    char *msg = randomquote(argv[2]);

    if (msg == NULL) {
        printf("error: empty quote or file\n");
        return 1;
    }

    //Creating Socket

    int port;
    if ((port = atoi(argv[1])) == 0) {
        printf("argument format error\n");
        return 1;
    }

    int serversock;

    if ((serversock = socket(AF_INET, SOCK_STREAM, 0)) == 1) {
        perror("error on socket()");
        return 1;
    }

    int opt = 1;
    if (setsockopt(serversock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) != 0) {
        perror("error setsockopt()");
        return 1;
    }

    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(serversock, (struct sockaddr*) &address, sizeof(address)) != 0) {
        perror("error on bind()");
        return 1;
    }

    if (listen(serversock, 3) != 0) {
        perror("error on listen()");
        return 1;
    }

    //Connecting to client

    int clientsock;
    struct sockaddr clientaddr;
    socklen_t clienaddrlen;

    if ((clientsock = accept(serversock, &clientaddr, &clienaddrlen)) == -1) {
        perror("error on accept()");
        return 1;
    }

    //Sending Quote

    send(clientsock, msg, strlen(msg)-1, 0);
    printf("message sent\n");

    //Finishing

    free(msg);
    close(serversock);
}

//random quote section

//Count lines in a file
int countlines(FILE* file) {
    int res = 0;
    char buf;

    while ((buf = getc(file)) != EOF) {
        if (buf == '\n') res++;
    }

    rewind(file);

    return res;
}

//random quote
char* randomquote(char* filepath) {
    FILE * file;
    if ((file = fopen(filepath, "r")) == NULL) {
        perror("error on opening file");
        exit(1);
    }

    //Exception for empty file
    fseek(file, 0, SEEK_END);
    if (ftell(file) == 0) {
        fclose(file);
        return NULL;
    }
    fseek(file, 0, SEEK_SET);

    srand(time(NULL));

    int clines = countlines(file);

    if (clines == 0) {
        fclose(file);
        return NULL;
    }

    int randline = rand()%clines;

    while (randline > 1) {
        char temp = getc(file);

        if (temp == '\n') randline--;

        if (temp == EOF) {
            printf("unexpected reading error\n");
            exit(1);
        }
    }

    long offset = ftell(file);

    size_t quotesize = 0;

    char temp;
    while ((temp = getc(file)) != EOF) {
        quotesize++;
        if (temp == '\n' || temp == '\0') break;
    }

    fseek(file, offset, SEEK_SET);

    char* quote = calloc(quotesize+1, sizeof(char));

    fread(quote, sizeof(char), quotesize, file);
    quote[quotesize] = '\0';

    fclose(file);

    if (quote[0] == '\0') {
        free(quote);
        return NULL;
    }

    return quote;
}