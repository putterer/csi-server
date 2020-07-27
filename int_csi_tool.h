#ifndef __CSI_SERVER_INT_TOOL_H__
#define __CSI_SERVER_INT_TOOL_H__

#include <stdlib.h>

typedef struct
{
    double real;
    double imag;
} INT_COMPLEX;

typedef struct {
	unsigned int timestamp_low;
	unsigned short bfee_count;
	unsigned char Nrx, Ntx;
	unsigned char rssi_a, rssi_b, rssi_c;
	char noise;
	unsigned char agc, antenna_sel;
	unsigned short len;
	unsigned short fake_rate_n_flags;
	unsigned char perm[3]; // antenna permutation 0 based
	INT_COMPLEX* csi_matrix;  // NON PERMUTED!!!, outer loop is subcarrier
} int_csi_notification;



void int_read_bfee(unsigned char* data, int_csi_notification* notification);
void int_free_notification(int_csi_notification* notification);

#endif

// file format: (read by read_bf_file.m)
// split into fields
// 2 bytes length entry, 1 byte code entry,  FIELD LENGTH CONTAINS the code
// code for bfee notification: 187 (0xBB)
// --> read fieldLen - 1 bytes for current entry
// CALLS INTO read_bfee.c


// from iwlagn_bfee_notif in lib.c
/*
	 * Each subcarrier uses Ntx * Nrx * 2 * 8 bits for matrix
	 * (2 signed 8-bit I/Q vals) plus 3 bits for SNR. I think the hardware
	 * always gives 0 for these 3 bits. See 802.11n spec section 7.3.1.28.
	 */
// IWL_DEBUG_RX(priv, "BFEE NOTIFICATION, Nrx=%u Ntx=%u "
// 			"len=%u calc_len=%u\n",
// 			Nrx, Ntx, len, (30*(3+2*Nrx*Ntx*8)+7)/8);



// no permutation in kernel

// BFEE notification as sent from kernel
// (inside /linux-80211n-csitool/drivers/net/wireless/iwlwifi/commands.h)

// struct iwl_bfee_notif {
// 	__le32 timestamp_low;
// 	__le16 bfee_count;
// 	__le16 reserved1;
// 	u8 Nrx, Ntx;
// 	u8 rssiA, rssiB, rssiC;
// 	s8 noise;
// 	u8 agc, antenna_sel;
// 	__le16 len;
// 	__le16 fake_rate_n_flags;
// 	u8 payload[0];
// } __attribute__ ((packed));




// MATLAB CSI Reader


// %% Initialize variables
// ret = cell(ceil(len/95),1);     % Holds the return values - 1x1 CSI is 95 bytes big, so this should be upper bound
// cur = 0;                        % Current offset into file
// count = 0;                      % Number of records output
// broken_perm = 0;                % Flag marking whether we've encountered a broken CSI yet
// triangle = [1 3 6];             % What perm should sum to for 1,2,3 antennas

// %% Process all entries in file
// % Need 3 bytes -- 2 byte size field and 1 byte code
// while cur < (len - 3)
//     % Read size and code
//     field_len = fread(f, 1, 'uint16', 0, 'ieee-be');
//     code = fread(f,1);
//     cur = cur+3;
    
//     % If unhandled code, skip (seek over) the record and continue
//     if (code == 187) % get beamforming or phy data
//         bytes = fread(f, field_len-1, 'uint8=>uint8');
//         cur = cur + field_len - 1;
//         if (length(bytes) ~= field_len-1)
//             fclose(f);
//             return;
//         end
//     else % skip all other info
//         fseek(f, field_len - 1, 'cof');
//         cur = cur + field_len - 1;
//         continue;
//     end
    
//     if (code == 187) %hex2dec('bb')) Beamforming matrix -- output a record
//         count = count + 1;
//         ret{count} = read_bfee(bytes);
        
//         perm = ret{count}.perm;
//         Nrx = ret{count}.Nrx;
//         if Nrx == 1 % No permuting needed for only 1 antenna
//             continue;
//         end
//         if sum(perm) ~= triangle(Nrx) % matrix does not contain default values
//             if broken_perm == 0
//                 broken_perm = 1;
//                 fprintf('WARN ONCE: Found CSI (%s) with Nrx=%d and invalid perm=[%s]\n', filename, Nrx, int2str(perm));
//             end
//         else
//             ret{count}.csi(:,perm(1:Nrx),:) = ret{count}.csi(:,1:Nrx,:);
//         end
//     end
// end
// ret = ret(1:count);

// %% Close file
// fclose(f);
// end



/*
 * (c) 2008-2011 Daniel Halperin <dhalperi@cs.washington.edu>
 */
// #include "mex.h"

// /* The computational routine */
// void read_bfee(unsigned char *inBytes, mxArray *outCell)
// {
// 	unsigned int timestamp_low = inBytes[0] + (inBytes[1] << 8) +
// 		(inBytes[2] << 16) + (inBytes[3] << 24);
// 	unsigned short bfee_count = inBytes[4] + (inBytes[5] << 8);
// 	unsigned int Nrx = inBytes[8];
// 	unsigned int Ntx = inBytes[9];
// 	unsigned int rssi_a = inBytes[10];
// 	unsigned int rssi_b = inBytes[11];
// 	unsigned int rssi_c = inBytes[12];
// 	char noise = inBytes[13];
// 	unsigned int agc = inBytes[14];
// 	unsigned int antenna_sel = inBytes[15];
// 	unsigned int len = inBytes[16] + (inBytes[17] << 8);
// 	unsigned int fake_rate_n_flags = inBytes[18] + (inBytes[19] << 8);
// 	unsigned int calc_len = (30 * (Nrx * Ntx * 8 * 2 + 3) + 7) / 8;
// 	unsigned int i, j;
// 	unsigned int index = 0, remainder;
// 	unsigned char *payload = &inBytes[20];
// 	char tmp;
// 	mwSize size[] = {Ntx, Nrx, 30};
// 	mxArray *csi = mxCreateNumericArray(3, size, mxDOUBLE_CLASS, mxCOMPLEX);
// 	mwSize perm_size[] = {1, 3};
// 	mxArray *perm = mxCreateNumericArray(2, perm_size, mxDOUBLE_CLASS, mxREAL);
// 	double* ptrR = (double *)mxGetPr(csi);
// 	double* ptrI = (double *)mxGetPi(csi);

// 	/* Check that length matches what it should */
// 	if (len != calc_len)
// 		mexErrMsgIdAndTxt("MIMOToolbox:read_bfee_new:size","Wrong beamforming matrix size.");

// 	/* Compute CSI from all this crap :) */
// 	for (i = 0; i < 30; ++i)
// 	{
// 		index += 3;
// 		remainder = index % 8;
// 		for (j = 0; j < Nrx * Ntx; ++j)
// 		{
// 			tmp = (payload[index / 8] >> remainder) |
// 				(payload[index/8+1] << (8-remainder));
// 			//printf("%d\n", tmp);
// 			*ptrR = (double) tmp;
// 			++ptrR;
// 			tmp = (payload[index / 8+1] >> remainder) |
// 				(payload[index/8+2] << (8-remainder));
// 			*ptrI = (double) tmp;
// 			++ptrI;
// 			index += 16;
// 		}
// 	}

// 	/* Compute the permutation array */
// 	ptrR = (double *)mxGetPr(perm);
// 	ptrR[0] = ((antenna_sel) & 0x3) + 1;
// 	ptrR[1] = ((antenna_sel >> 2) & 0x3) + 1;
// 	ptrR[2] = ((antenna_sel >> 4) & 0x3) + 1;

// 	mxDestroyArray(mxGetField(outCell, 0, "timestamp_low"));
// 	mxDestroyArray(mxGetField(outCell, 0, "bfee_count"));
// 	mxDestroyArray(mxGetField(outCell, 0, "Nrx"));
// 	mxDestroyArray(mxGetField(outCell, 0, "Ntx"));
// 	mxDestroyArray(mxGetField(outCell, 0, "rssi_a"));
// 	mxDestroyArray(mxGetField(outCell, 0, "rssi_b"));
// 	mxDestroyArray(mxGetField(outCell, 0, "rssi_c"));
// 	mxDestroyArray(mxGetField(outCell, 0, "noise"));
// 	mxDestroyArray(mxGetField(outCell, 0, "agc"));
// 	mxDestroyArray(mxGetField(outCell, 0, "perm"));
// 	mxDestroyArray(mxGetField(outCell, 0, "rate"));
// 	mxDestroyArray(mxGetField(outCell, 0, "csi"));
// 	mxSetField(outCell, 0, "timestamp_low", mxCreateDoubleScalar((double)timestamp_low));
// 	mxSetField(outCell, 0, "bfee_count", mxCreateDoubleScalar((double)bfee_count));
// 	mxSetField(outCell, 0, "Nrx", mxCreateDoubleScalar((double)Nrx));
// 	mxSetField(outCell, 0, "Ntx", mxCreateDoubleScalar((double)Ntx));
// 	mxSetField(outCell, 0, "rssi_a", mxCreateDoubleScalar((double)rssi_a));
// 	mxSetField(outCell, 0, "rssi_b", mxCreateDoubleScalar((double)rssi_b));
// 	mxSetField(outCell, 0, "rssi_c", mxCreateDoubleScalar((double)rssi_c));
// 	mxSetField(outCell, 0, "noise", mxCreateDoubleScalar((double)noise));
// 	mxSetField(outCell, 0, "agc", mxCreateDoubleScalar((double)agc));
// 	mxSetField(outCell, 0, "perm", perm);
// 	mxSetField(outCell, 0, "rate", mxCreateDoubleScalar((double)fake_rate_n_flags));
// 	mxSetField(outCell, 0, "csi", csi);

// 	//printf("Nrx: %u Ntx: %u len: %u calc_len: %u\n", Nrx, Ntx, len, calc_len);
// }

// /* The gateway function */
// void mexFunction(int nlhs, mxArray *plhs[],
// 	         int nrhs, const mxArray *prhs[])
// {
// 	const char* fieldnames[] = {"timestamp_low",
// 		"bfee_count",
// 		"Nrx", "Ntx",
// 		"rssi_a", "rssi_b", "rssi_c",
// 		"noise",
// 		"agc",
// 		"perm",
// 		"rate",
// 		"csi"};
// 	unsigned char *inBytes;		/* A beamforming matrix */
// 	mxArray *outCell;		/* The cellular output */

// 	/* check for proper number of arguments */
// 	if(nrhs!=1) {
// 		mexErrMsgIdAndTxt("MIMOToolbox:read_bfee_new:nrhs","One input required.");
// 	}
// 	if(nlhs!=1) {
// 		mexErrMsgIdAndTxt("MIMOToolbox:read_bfee_new:nlhs","One output required.");
// 	}
// 	/* make sure the input argument is a char array */
// 	if (!mxIsClass(prhs[0], "uint8")) {
// 		mexErrMsgIdAndTxt("MIMOToolbox:read_bfee_new:notBytes","Input must be a char array");
// 	}

// 	/* create a pointer to the real data in the input matrix */
// 	inBytes = mxGetData(prhs[0]);

// 	/* create the output matrix */
// 	outCell = mxCreateStructMatrix(1, 1, 12, fieldnames);


// 	/* call the computational routine */
// 	read_bfee(inBytes,outCell);

// 	/* */
// 	plhs[0] = outCell;
// }
