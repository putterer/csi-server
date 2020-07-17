#include <stdio.h>
#include "int_csi_tool.h"

/*
 * Based on the linux csi tool and code by
 * (c) 2008-2011 Daniel Halperin <dhalperi@cs.washington.edu>
 */


// Reads a beamforming notification not containing the length (2 bytes) and code (1 byte)
void int_read_bfee(unsigned char* data, int_csi_notification* notification) {
    notification->timestamp_low = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
	notification->bfee_count = data[4] + (data[5] << 8);
	notification->Nrx = data[8];
	notification->Ntx = data[9];
	notification->rssi_a = data[10];
	notification->rssi_b = data[11];
	notification->rssi_c = data[12];
	notification->noise = data[13];
	notification->agc = data[14];
	notification->antenna_sel = data[15];
	notification->len = data[16] + (data[17] << 8);
	notification->fake_rate_n_flags = data[18] + (data[19] << 8);

    unsigned int csi_mat_target_len = (30 * (notification->Nrx * notification->Ntx * 8 * 2 + 3) + 7) / 8;

    if(notification->len != csi_mat_target_len) {
        printf("CSI Matrix length (%d) doesn't equal expected length (%d)\n", notification->len, csi_mat_target_len);
    }

    unsigned char* payload = (unsigned char*)(data + 20);

    notification->csi_matrix = (INT_COMPLEX*) malloc(notification->Ntx * notification->Nrx * 30 * sizeof(INT_COMPLEX));
    INT_COMPLEX *ptr = notification->csi_matrix;


    int index = 0, remainder;
    char tmp;
    /* Compute CSI from all this crap :) */
	for (int i = 0; i < 30; ++i) {
		index += 3;
		remainder = index % 8;
		for (int j = 0; j < notification->Nrx * notification->Ntx; ++j) {
			tmp = (payload[index / 8] >> remainder) |
				(payload[index/8+1] << (8-remainder));
			ptr->real = (double) tmp;
			tmp = (payload[index / 8+1] >> remainder) |
				(payload[index/8+2] << (8-remainder));
			ptr->imag = (double) tmp;

            ptr++;
			index += 16;
		}
	}

    /* Compute the permutation array */
	notification->perm[0] = ((notification->antenna_sel) & 0x3) + 1;
	notification->perm[1] = ((notification->antenna_sel >> 2) & 0x3) + 1;
	notification->perm[2] = ((notification->antenna_sel >> 4) & 0x3) + 1;
}

void int_free_notification(int_csi_notification* notification) {
    free(notification->csi_matrix);
}