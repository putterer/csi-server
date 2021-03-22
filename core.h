#ifndef __CSI_SERVER_CORE_H__
#define __CSI_SERVER_CORE_H__

#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netinet/in.h>

#include "ath_csi_tool.h"
#include "int_csi_tool.h"

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

// deprecated
// sent to a client/subscriber to notify them of CSI for an arriving packet
// struct csi_info {
//     char messageType;//Set to TYPE_CSI_INFO
//     int subscriptionId;
//     int messageId;
//     ath_csi_struct csi_status;
//     ATH_COMPLEX csi_matrix[3][3][114];
// };

void ath_onCSI(unsigned char *data_buf, ath_csi_struct* csi_status, ATH_COMPLEX csi_matrix[3][3][114]);
void int_onCSI(int_csi_notification* notification);
void subscribe(char* buf, int len, struct sockaddr_in* clientAddress, socklen_t addressLength);
void unsubscribe(char* buf, int len, struct sockaddr_in* clientAddress, socklen_t addressLength);
int matchesFilter(ath_csi_struct* csi_status, struct filter_options* options);

void setAtherosStdoutDumpEnabled(bool enabled);

#endif