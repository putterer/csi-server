#include <string.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <stdio.h>

#include "base64.h"

#include "logger.h"
#include "core.h"
#include "server.h"
#include "util.h"

/**
 * Manages subscriptions and forwarding CSI messages to subscribers
 */


struct subscription* subscriptions[100];
int subscriptionsLength = 0;
char buffer[20000];

// ID of the message, incremented with each received CSI info and included in each notification
int messageId;

bool atherosStdoutDumpEnabled = false;

/**
 * Packages csi matrix and status into a buffer to be sent to a subscriber
 * @param buffer the buffer to write to
 * @param csi_status the csi status
 * @param csi_matrix the csi matrix
 * @returns the length of the packaged data
 */
int ath_packageCSIInfoMessage(char* buffer, ath_csi_struct* csi_status, ATH_COMPLEX csi_matrix[3][3][114]) {
    int index = 0;
    putByte(buffer, &index, TYPE_ATH_CSI_INFO);
    putInt(buffer, &index, messageId);
    putLong(buffer, &index, csi_status->tstamp);
    putShort(buffer, &index, csi_status->channel);
    putByte(buffer, &index, csi_status->chanBW);
    putByte(buffer, &index, csi_status->rate);
    putByte(buffer, &index, csi_status->nr);
    putByte(buffer, &index, csi_status->nc);
    putByte(buffer, &index, csi_status->num_tones);
    putByte(buffer, &index, csi_status->noise);
    putByte(buffer, &index, csi_status->phyerr);
    putByte(buffer, &index, csi_status->rssi);
    putByte(buffer, &index, csi_status->rssi_0);
    putByte(buffer, &index, csi_status->rssi_1);
    putByte(buffer, &index, csi_status->rssi_2);
    putShort(buffer, &index, csi_status->payload_len);
    putShort(buffer, &index, csi_status->csi_len);
    putShort(buffer, &index, csi_status->buf_len);

    if(csi_status->csi_len > 0) {
        for(int i1 = 0;i1 < 3;i1++) {
			for(int i2 = 0;i2 < 3;i2++) {
				for(int i3 = 0;i3 < 114;i3++) {
					putInt(buffer, &index, csi_matrix[i1][i2][i3].real);
                    putInt(buffer, &index, csi_matrix[i1][i2][i3].imag);
				}
			}
		}
    } else {
        memset((buffer + index), 0, sizeof(ATH_COMPLEX) * 3 * 3 * 114);
        index += sizeof(ATH_COMPLEX) * 3 * 3 * 114;
    }

    return index;
}

void ath_dumpCSIInfoMessageToStdout(char* buffer, ath_csi_struct* csi_status, ATH_COMPLEX csi_matrix[3][3][114]) {
    printf("<athCSI>");
    // printf("<msgId>%d</msgId>", messageId);
    // printf("<tstamp>%d</tstamp>", csi_status->tstamp);
    // printf("<channel>%d</channel>", csi_status->channel);
    // printf("<chanBW>%d</chanBW>", csi_status->chanBW);
    // printf("<rate>%d</rate>", csi_status->rate);
    // printf("<nr>%d</nr>", csi_status->nr);
    // printf("<nc>%d</nc>", csi_status->nc);
    // printf("<num_tones>%d</num_tones>", csi_status->num_tones);
    // printf("<noise>%d</noise>", csi_status->noise);
    // printf("<phyerr>%d</phyerr>", csi_status->phyerr);
    // printf("<rssi>%d</rssi>", csi_status->rssi);
    // printf("<rssi_0>%d</rssi_0>", csi_status->rssi_0);
    // printf("<rssi_1>%d</rssi_1>", csi_status->rssi_1);
    // printf("<rssi_2>%d</rssi_2>", csi_status->rssi_2);
    // printf("<payload_len>%d</payload_len>", csi_status->payload_len);
    // printf("<csi_len>%d</csi_len>", csi_status->csi_len);
    // printf("<buf_len>%d</buf_len>", csi_status->buf_len);
    
    int len = ath_packageCSIInfoMessage(buffer, csi_status, csi_matrix);
    // for(int i = 0;i < len;i++) {
    //     // printf("%x", buffer[i]);

    // }

    size_t encodedLength = 0;
    char* base64EncodedBuffer = base64_encode((unsigned char*)buffer, len, &encodedLength); // null terminated, malloc'd buffer
    printf("%s", base64EncodedBuffer);
    free(base64EncodedBuffer);

    printf("</athCSI>\n");
    fflush(stdout);
}

/**
 * Packages csi matrix and status into a buffer to be sent to a subscriber
 * @param buffer the buffer to write to
 * @param notification the csi notification
 * @returns the length of the packaged data
 */
int int_packageCSIInfoMessage(char* buffer, int_csi_notification* notification) {
    int index = 0;

    // send as longer type to prevent conversion to signed at receiver
    uint16_t rssi_a = notification->rssi_a;
    uint16_t rssi_b = notification->rssi_b;
    uint16_t rssi_c = notification->rssi_c;
    uint16_t noise = notification->noise;
    uint16_t agc = notification->agc;

    putByte(buffer, &index, TYPE_INT_CSI_INFO);
    putInt(buffer, &index, messageId);
    putInt(buffer, &index, notification->timestamp_low);
    putShort(buffer, &index, notification->bfee_count);
    putByte(buffer, &index, notification->Nrx);
    putByte(buffer, &index, notification->Ntx);
    putShort(buffer, &index, rssi_a);
    putShort(buffer, &index, rssi_b);
    putShort(buffer, &index, rssi_c);
    putShort(buffer, &index, noise);
    putShort(buffer, &index, agc);
    putByte(buffer, &index, notification->antenna_sel);
    putByte(buffer, &index, notification->perm[0]);
    putByte(buffer, &index, notification->perm[1]);
    putByte(buffer, &index, notification->perm[2]);
    putShort(buffer, &index, notification->len);
    putShort(buffer, &index, notification->fake_rate_n_flags);
    

    // format: [tx][rx][carrier]   !!!!!   (permutation of rx ant <-> rf chains is already applied)
    int csi_mat_entries = notification->Ntx * notification->Nrx * 30;
    for(int i = 0;i < csi_mat_entries;i++) {
        putDouble(buffer, &index, notification->csi_matrix[i].real);
        putDouble(buffer, &index, notification->csi_matrix[i].imag);
    }

    return index;
}

/**
 * Called on receiving CSI from the kernel, checks filters and forwards to subscribers 
 * @param data_buf the data from the kernel
 * @param csi_status the csi status from the kernel
 * @param csi_matrix the csi matrix from the kernel
 */
void ath_onCSI(unsigned char *data_buf, ath_csi_struct* csi_status, ATH_COMPLEX csi_matrix[3][3][114]) {
    printf("Broadcasting to subscribers\n");

    if(atherosStdoutDumpEnabled) {
        ath_dumpCSIInfoMessageToStdout(buffer, csi_status, csi_matrix);
    }

    for(int i = 0;i < subscriptionsLength;i++) {
        struct subscription* sub = subscriptions[i];
        if(matchesFilter(csi_status, &(sub->options.filter_options))) {
            int len = ath_packageCSIInfoMessage(buffer, csi_status, csi_matrix);
            sendData(sub->address, sub->addressLength, buffer, len);
        }
    }

    messageId++;
}

/**
 * Called on receiving CSI from the kernel, does not yet filter and forwards to subscribers 
 * @param notification the notification constructed from the data received from the kernel
 */
void int_onCSI(int_csi_notification* notification) {
    for(int i = 0;i < subscriptionsLength;i++) {
        struct subscription* sub = subscriptions[i];
        // TODO: check / add filter options
        // if(matchesFilter(csi_status, &(sub->options.filter_options))) {
            int len = int_packageCSIInfoMessage(buffer, notification);
            sendData(sub->address, sub->addressLength, buffer, len);
        // }
    }
    messageId++;
}

/**
 * Adds a subscription for a client
 * @param buf the received subscription message
 * @param len length of the subscription message
 * @param clientAddress the address of the subscriber to be added
 * @param addressLength the length of the subscriber's address
 */
void subscribe(char* buf, int len, struct sockaddr_in* clientAddress, socklen_t addressLength) {
    if(len != sizeof(struct subscription_options)) {
        log(LEVEL_WARNING, "Subscription message has wrong length");
        return;
    }

    unsubscribe(NULL, 0, clientAddress, addressLength);

    struct subscription* subscription = malloc(sizeof(struct subscription));
    memcpy(&(subscription->options), buf, sizeof(struct subscription_options));

    subscription->address = malloc(sizeof(struct sockaddr_in));
    memcpy(subscription->address, clientAddress, sizeof(struct sockaddr_in));

    subscription->addressLength = addressLength;

    subscriptions[subscriptionsLength] = subscription;
    subscriptionsLength++;

    log(LEVEL_INFO, "Subscription from %s", inet_ntoa(clientAddress->sin_addr));

    char confirmation[] = {TYPE_CONFIRM_SUBSCRIPTION};
    sendData(clientAddress, addressLength, confirmation, 1);
}

//TODO: maybe just allow one client?
/**
 * Removes a subscriber
 * @param buf the unsubscription message
 * @param len the length of the unsubscription message
 * @param clientAddress the address of the client
 * @param addressLength the length of the client's address
 */
void unsubscribe(char* buf, int len, struct sockaddr_in* clientAddress, socklen_t addressLength) {
    //char confirmation[] = {TYPE_CONFIRM_UNSUBSCRIPTION};
    //sendData(clientAddress, addressLength, confirmation, 1);

    int i = 0;
    for(;i < subscriptionsLength;i++) {
        struct sockaddr_in* otherAddr = subscriptions[i]->address;
        if(clientAddress->sin_addr.s_addr == otherAddr->sin_addr.s_addr
            && clientAddress->sin_port == otherAddr->sin_port) {
            goto found; // This the only goto I have ever and will ever use in my life (and it's probably one too much)
        }
    }

    return;

    found:
    log(LEVEL_INFO, "Removing %s", inet_ntoa(clientAddress->sin_addr));
    free(subscriptions[i]->address);
    free(subscriptions[i]);

    i++;
    for(;i < subscriptionsLength;i++) {
        subscriptions[i - 1] = subscriptions[i];
    }
    subscriptionsLength--;
}


/**
 * Checks if the current csi info matches a client's subscription options, can be expanded
 * @param csi_status
 * @param options the client's filter options
 * @returns 1 if matches client's filter, 0 else
 */
int matchesFilter(ath_csi_struct* csi_status, struct filter_options* options) {
    if(options->payload_size != 0 && csi_status->payload_len != options->payload_size) {
       return 0;
    }

    //built this way to accomodate more filters

    return 1;
}

void setAtherosStdoutDumpEnabled(bool enabled) {
    atherosStdoutDumpEnabled = enabled;
    if(atherosStdoutDumpEnabled) {
        log(LEVEL_INFO, "Dumping (atheros) csi data to stdout ");
    }
}