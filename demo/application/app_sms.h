/*
 * =====================================================================================
 *
 *       Filename:  app_sms.h
 *
 *    Description:  the head file
 *
 *        Version:  1.0
 *        Created:  2014-6-11 15:58:20
 *       Revision:  none
 *       Compiler:  
 *
 *         Author:  weijiang.he@sim.com
 *   Organization:  www.sim.com
 *
 * =====================================================================================
 */

#if !defined APP_SMS_H
#define APP_SMS_H

/* #####   HEADER FILE INCLUDES   ################################################### */

/* #####   EXPORTED MACROS   ######################################################## */

/* #####   EXPORTED TYPE DEFINITIONS   ############################################## */


/* #####   EXPORTED DATA TYPES   #################################################### */

/* #####   EXPORTED VARIABLES   ##################################################### */

/* #####   EXPORTED FUNCTION DECLARATIONS   ######################################### */
extern void simcom_sms_init(void);
extern void simcom_sms_test(void);
eat_bool simcom_sms_send(u8* number, u8* msg, u16 msgLen);
eat_bool simcom_sms_msg_read(u16 index);
eat_bool simcom_sms_sc_set(u8* number);
eat_bool simcom_sms_format_set(u8 nFormatType);
eat_bool simcom_sms_msg_delete(u16 index);
#endif
