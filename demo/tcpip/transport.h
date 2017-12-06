#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__

/*
 * =====================================================================================
 *
 *       Filename:  transport.h
 *
 *    Description:  the head file
 *
 *        Version:  1.0
 *        Created:  2017-12-5 15:00:20
 *       Revision:  None
 *       Compiler:  gcc
 *
 *         Author:  Zhanglejia, zhanlej@qq.com
 *   Organization:  None
 *
 * =====================================================================================
 */

/* #####   HEADER FILE INCLUDES   ################################################### */

/* #####   EXPORTED MACROS   ######################################################## */


/* #####   EXPORTED TYPE DEFINITIONS   ############################################## */
typedef struct _eat_msg_
{
    int id;
    int mqtt_status;
}eat_msg_t;

/* #####   EXPORTED DATA TYPES   #################################################### */

/* #####   EXPORTED VARIABLES   ##################################################### */
extern u32 mqtt_ack_flag;

/* #####   EXPORTED FUNCTION DECLARATIONS   ######################################### */
int transport_open(char* addr, int port);
int transport_sendPacketBuffer(int sock, unsigned char* buf, int buflen);
int transport_getdata(unsigned char* buf, int count);
eat_bool mqtt_send_handle(eat_msg_t msg);
eat_bool mqtt_receive_handle(eat_msg_t msg);

void MQTT_Initial();
void MQTT_Sub0Pub1();



#endif /* __TRANSPORT_H__ */