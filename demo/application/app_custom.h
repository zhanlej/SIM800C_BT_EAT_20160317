#ifndef _APP_CUSTOM_H_
#define _APP_CUSTOM_H_

#include "platform.h"

/**************Pin define begin***********************/

/**************Pin define end************************/

/**************Timer define begin*********************/

/**************Timer define end**********************/

/**************UART function define begin**************/
#define eat_uart_app EAT_UART_1
#define app_uart_at EAT_UART_NULL
#define app_uart_debug EAT_UART_1
/**************UART function define end***************/

/**************Modem configuration define begin*********/
#define APN_NAME_LEN						20
#define APN_USER_NAME_LEN					20
#define APN_PASSWORD_LEN					20
#define SERVER_IP_LEN				        20
#define FTP_SERVER_USER_NAME_LEN			20
#define FTP_PASSWORD_LEN					20
#define FTP_FILENAME_LEN					20
#define FTP_FILEPATH_LEN					20
#define FTP_PORT_LEN						5

#define MAX_SMS_STRING_LEN          160 
#define MASTER_MOBILE_NUMBER_LEN    24

typedef struct ModemConfigContextTag
{
	ascii apnName[APN_NAME_LEN+1];
	ascii apnUserName[APN_USER_NAME_LEN+1];
	ascii apnPassword[APN_PASSWORD_LEN+1];
	ascii FTPServerIP[SERVER_IP_LEN+1];
	ascii ftpUserName[FTP_SERVER_USER_NAME_LEN+1];
	ascii ftpPassword[FTP_PASSWORD_LEN+1];
  	ascii ftpFileName[FTP_FILENAME_LEN+1];
	ascii ftpFilePath[FTP_FILEPATH_LEN+1];
	u16 ftpPort;
}ModemConfigContext;

typedef enum
{
    PDU,        
    TEXT
} SMS_FORMAT;

typedef struct
{
    SMS_FORMAT formatType;
	ascii sc_number[MASTER_MOBILE_NUMBER_LEN+1];
    u8 phone_number[MASTER_MOBILE_NUMBER_LEN+1];
    u8 sms_string[MAX_SMS_STRING_LEN];    
    u8 msg_len;
    u8 mt;      //mt = 2, flash message
} SIMCOM_SMS_INFO;

extern ModemConfigContext g_modemConfig;
/**************Modem configuration define end**********/

void GsmInitCallback(eat_bool result);

#endif //_APP_CUSTOM_H_
