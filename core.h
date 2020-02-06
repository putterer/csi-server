#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netinet/in.h>

#include "csi_tool.h"

struct filter_options {
    int payload_size; // Filter messages by payload size, to ignore set to 0
};

struct subscription_options {
    struct filter_options filter_options;
};

struct subscription {
    struct sockaddr_in* address;
    socklen_t addressLength;
    struct subscription_options options;
};

struct csi_info {
    char messageType;//Set to TYPE_CSI_INFO
    int subscriptionId;
    int messageId;
    csi_struct csi_status;
    COMPLEX csi_matrix[3][3][114];
};

void onCSI(unsigned char *data_buf, csi_struct* csi_status, COMPLEX csi_matrix[3][3][114]);
void subscribe(char* buf, int len, struct sockaddr_in* clientAddress, socklen_t addressLength);
void unsubscribe(char* buf, int len, struct sockaddr_in* clientAddress, socklen_t addressLength);
int matchesFilter(csi_struct* csi_status, struct filter_options* options);
