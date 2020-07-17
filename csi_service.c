#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "logger.h"
#include "core.h"
#include "csi_types.h"
#include "int_csi_tool.h"
#include "ath_csi_tool.h"

#define RECV_BUFSIZE 4096

int mode = -1;

unsigned char receiveBuffer[RECV_BUFSIZE];

// ---------------------------
// ATHEROS TOOL
// ---------------------------
// https://wands.sg/research/wifi/AtherosCSI/
// M x N x subcarriers   where M is transmitting antenna, N is receiving antenna
ATH_COMPLEX       csiMatrix[3][3][114];
ath_csi_struct*   ath_csiStatus;
unsigned char ath_dataBuffer[1500];
int           ath_csiDevice;// device file descriptor

// ---------------------------
// ATHEROS TOOL
// ---------------------------


int initCSI() {    
    ath_csiDevice = ath_open_csi_device();
    if(ath_csiDevice < 0) {
        log(LEVEL_WARNING, "Could not open atheros csi device, are you running as root? is /dev/CSI_dev present?");
        log(LEVEL_INFO, "Assuming intel chip, reading csi from stdin...");
        log(LEVEL_WARNING, "RUN AS SUDO!");
        
        mode = MODE_INT_TOOL;
    } else {
        log(LEVEL_DEBUG, "Opened csi device, fd: %d", ath_csiDevice);

        mode = MODE_ATH_TOOL;
        ath_csiStatus = (ath_csi_struct*) malloc(sizeof(ath_csi_struct));
    }

    return 0;
}

int closeCSI() {
    
    if(mode == MODE_ATH_TOOL) {
        ath_close_csi_device(ath_csiDevice);
        free(ath_csiStatus);
        log(LEVEL_DEBUG, "Closing csi device");
    }

    return 0;
}

void processCSI(unsigned char *data_buf, ath_csi_struct* csi_status, ATH_COMPLEX csi_matrix[3][3][114]) {

    onCSI(data_buf, csi_status, csi_matrix);

    // ATHEROS debugging code
    /* if(1 || csi_status->payload_len == 140) {
        printf("Recv msg with rate: 0x%02x | payload len: %d\n",csi_status->rate,csi_status->payload_len);
        printf("\n");
        printf("RSSI: %d\n", csi_status->rssi);
        printf("Subcarriers: %d\n", csi_status->num_tones);
        printf("RX antenna: %d\n", csi_status->nr);
        printf("TX antenna: %d\n", csi_status->nc);
        
        //ATH_COMPLEX* mat = csi_matrix[csi_status->nc][csi_status->nr];

        if(csi_status->csi_len == 0) {
            printf("No CSI in message (probably valid but no hw_upload/type)\n\n\n");
            return;
        }

        for(int tx = 0;tx < 3;tx++) {
            for(int rx = 0;rx < 3;rx++) {
                printf("\nTX: %d, RX: %d\n", tx, rx);
                for(int i = 0;i < csi_status->num_tones;i++) {
                   printf("R%dI%d ", csi_matrix[tx][rx][i].real, csi_matrix[csi_status->nc][csi_status->nr][i].imag);
                }
            }
        }
        printf("\n\n");

        printf("Above: msg with rate: 0x%02x | payload len: %d\n",csi_status->rate,csi_status->payload_len);

        //printf("\n");
        //for(int i = 0;i < csi_status->buf_len;i++) {
        //    printf("0x%02x\t", data_buf[i]);
        //}
        //printf("\n\n");
    } */

    
}

/**
 * Attempts to read csi from the csi device
 * @returns 1 if csi was read and processed, 0 otherwise
 */
int readCSI() {

    if(mode == MODE_ATH_TOOL) {
        int length = ath_read_csi_buf(receiveBuffer, ath_csiDevice, RECV_BUFSIZE);
        if(length > 0) {
            log(LEVEL_TRACE, "Received %d bytes (a message) from csi device", length);

            ath_record_status(receiveBuffer, length, ath_csiStatus);
            ath_record_csi_payload(receiveBuffer, ath_csiStatus, ath_dataBuffer, csiMatrix);

            processCSI(ath_dataBuffer, ath_csiStatus, csiMatrix);
            return 1;
        } else {
            return 0;
        }
    }

    if(mode == MODE_INT_TOOL) {
        //read 2 only, then read skip
        printf("reading\n");
        memset(receiveBuffer, 0, RECV_BUFSIZE);
        int bytesReceived = read(fileno(stdin), receiveBuffer, 3);
        printf("firstRead %d\n", bytesReceived);

        if(! bytesReceived) {
            return 0;
        }

        //TODO: test using file
        
        unsigned short *len = (unsigned short*)receiveBuffer;
        unsigned char *code = (unsigned char*)(receiveBuffer + 2);
        printf("Code: %d, Len: %d\n", *code, *len);

        while(bytesReceived < (*len + 2)) {
            bytesReceived += read(fileno(stdin), receiveBuffer + bytesReceived, (*len + 2) - bytesReceived);
            printf("totalRead %d / %d\n", bytesReceived, *len);
        }

        if(*code == 0xBB) {
            int_csi_notification csi_notification;
            printf("Found csi of length %d\n", *len);

            int_read_bfee(receiveBuffer + 3, &csi_notification);

            int_free_notification(&csi_notification);
        } else {
            printf("Found unknown field of code %d\n", *code);
        }

        return 0;
    }

    return 0;
}
