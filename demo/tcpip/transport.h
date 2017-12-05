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


/* #####   EXPORTED DATA TYPES   #################################################### */

/* #####   EXPORTED VARIABLES   ##################################################### */

/* #####   EXPORTED FUNCTION DECLARATIONS   ######################################### */
int transport_sendPacketBuffer(int sock, unsigned char* buf, int buflen);
int transport_getdata(unsigned char* buf, int count);
void MQTT_Initial();
void MQTT_Sub0Pub1();



#endif /* __TRANSPORT_H__ */