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
typedef enum
{
    OPEN = 1,           /* 发送开机信息 */
    RETURN_PUB,     /* 返回接收到的数据 */
    SENDDATA        /* 发送环境数据 */
} mqtt_pub_tpye_enum;

typedef struct _eat_msg_
{
    int id;
    int mqtt_status;
    mqtt_pub_tpye_enum mqtt_pub_tpye;
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
eat_bool mqtt_pingreq();



#endif /* __TRANSPORT_H__ */