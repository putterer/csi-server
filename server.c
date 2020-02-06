#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <errno.h>

#include "server.h"
#include "logger.h"
#include "core.h"

int socketFD;
char buffer[MAX_MESSAGE_LENGTH];


void onDatagram(int len, struct sockaddr_in* clientAddress, int addressLength) {
    int type = buffer[0];
    char* buf = buffer + 1;
    len = len - 1;
    switch(type) {
        case TYPE_SUBSCRIBE: subscribe(buf, len, clientAddress, addressLength);break;
        case TYPE_UNSUBSCRIBE: unsubscribe(buf, len, clientAddress, addressLength);break;
    }
}

int readServer() {
    struct sockaddr_in clientAddress;
    memset(&clientAddress, 0, sizeof(clientAddress)); 

    //TODO: poll interval is only every 5 secs????? why?
    socklen_t addressLength = 100000;
    int length = recvfrom(socketFD, buffer, MAX_MESSAGE_LENGTH, MSG_DONTWAIT, (struct sockaddr*) &clientAddress, &addressLength);
    
    //No data
    if(length == 0 || (length == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))) {
        return 0;
    }

    log(LEVEL_DEBUG, "Received datagram from %s:%d", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

    onDatagram(length, &clientAddress, addressLength);

    return 1;
}

void sendData(struct sockaddr_in* dest, socklen_t addrLen, char* buf, int len) {
    int sent = sendto(socketFD, buf, len, 0, (struct sockaddr*) dest, addrLen);
    if(sent == -1) {
        log(LEVEL_ERROR, "Could not send to %s:%d", inet_ntoa(dest->sin_addr), ntohs(dest->sin_port));
        perror("");
    }
}

int initServer() {
    struct sockaddr_in serverAddress;

    socketFD = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketFD < 0) {
        log(LEVEL_ERROR, "Could not create socket");
        perror("");
        return 1;
    }

    memset(&serverAddress, 0, sizeof(serverAddress)); 

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    int status = bind(socketFD, (const struct sockaddr *) &serverAddress, sizeof(serverAddress));
    if(status < 0) {
        log(LEVEL_ERROR, "Could not bind socket");
        perror("");
        return 1;
    }

    return 0;
}

int closeServer() {
    close(socketFD);
    return 0;
}