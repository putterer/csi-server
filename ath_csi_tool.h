/*
 * =====================================================================================
 *       Filename:  csi_fun.h
 * this file is published under GPL v2 here: https://github.com/xieyaxiongfly/Atheros_CSI_tool_OpenWRT_src/tree/master/package/recvCSI
 *
 *    Description:  head file for csi processing fucntion  
 *        Version:  1.0
 *
 *         Author:  Yaxiong Xie  
 *         Email :  <xieyaxiongfly@gmail.com>
 *   Organization:  WANDS group @ Nanyang Technological University
 *
 *   Copyright (c)  WANDS group @ Nanyang Technological University
 * =====================================================================================
 */
#ifndef __CSI_SERVER_ATH_TOOL_H__
#define __CSI_SERVER_ATH_TOOL_H__

#include <stdbool.h>
#include <sys/types.h>

#define Kernel_CSI_ST_LEN 23 
typedef struct
{
    int real;
    int imag;
} ATH_COMPLEX;

typedef struct
{
    u_int64_t tstamp;         /* h/w assigned time stamp */
    
    u_int16_t channel;        /* wireless channel (represented in Hz)*/
    u_int8_t  chanBW;         /* channel bandwidth (0->20MHz,1->40MHz)*/

    u_int8_t  rate;           /* transmission rate*/
    u_int8_t  nr;             /* number of receiving antenna*/
    u_int8_t  nc;             /* number of transmitting antenna*/
    u_int8_t  num_tones;      /* number of tones (subcarriers) */
    u_int8_t  noise;          /* noise floor (to be updated)*/

    u_int8_t  phyerr;          /* phy error code (set to 0 if correct)*/

    u_int8_t    rssi;         /*  rx frame RSSI */
    u_int8_t    rssi_0;       /*  rx frame RSSI [ctl, chain 0] */
    u_int8_t    rssi_1;       /*  rx frame RSSI [ctl, chain 1] */
    u_int8_t    rssi_2;       /*  rx frame RSSI [ctl, chain 2] */

    u_int16_t   payload_len;  /*  payload length (bytes) */
    u_int16_t   csi_len;      /*  csi data length (bytes) */
    u_int16_t   buf_len;      /*  data length in buffer */
} ath_csi_struct;

bool  ath_is_big_endian();
int   ath_open_csi_device();
void  ath_close_csi_device(int fd);
int   ath_read_csi_buf(unsigned char* buf_addr,int fd, int BUFSIZE);
void  ath_record_status(unsigned char* buf_addr, int cnt, ath_csi_struct* csi_status);
void  ath_record_csi_payload(unsigned char* buf_addr, ath_csi_struct* csi_status,unsigned char* data_buf, ATH_COMPLEX(* csi_buf)[3][114]);


#endif