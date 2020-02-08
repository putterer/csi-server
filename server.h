#include <netinet/in.h>

// Types of server messages
#define TYPE_SUBSCRIBE 10
#define TYPE_UNSUBSCRIBE 11
#define TYPE_CONFIRM_SUBSCRIPTION 12
#define TYPE_CONFIRM_UNSUBSCRIPTION 13
#define TYPE_CSI_INFO 14

#define PORT 9380
#define MAX_MESSAGE_LENGTH 65507 // MTU might be lower, this is the the maximum size with IP header already deducted

int readServer();
int initServer();
int closeServer();
void sendData(struct sockaddr_in* dest, socklen_t addrLen, char* buf, int len);