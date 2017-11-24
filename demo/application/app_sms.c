/********************************************************************
 *                Copyright Simcom(shanghai)co. Ltd.                   *
 *---------------------------------------------------------------------
 * FileName      :   app_sms.c
 * version       :   0.10
 * Description   :   
 * Authors       :   heweijiang
 * Notes         :
 *---------------------------------------------------------------------
 *
 *    HISTORY OF CHANGES
 *
 *0.10  2014-06-11, heweijiang, create originally.
 *---------------------------------------------------------------------
 *
 *1. Before using the SMS service,please call simcom_sms_init().
 *2. Use eat_get_sms_ready_state() to confirm SMS service is available or not.
 *3. About SMS APT useage,please refer to the API description in eat_sms.h
 *
 ********************************************************************/
/********************************************************************
 * Include Files
 ********************************************************************/
 
#include <string.h>
#include "eat_sms.h"
#include "eat_interface.h"
#include "platform.h"
#include "app_custom.h"

/********************************************************************
* Macros
 ********************************************************************/

/********************************************************************
* Types
 ********************************************************************/

/********************************************************************
* Extern Variables (Extern /Global)
 ********************************************************************/
extern SIMCOM_SMS_INFO g_sms_info;
/********************************************************************
* Local Variables:  STATIC
 ********************************************************************/

/********************************************************************
* External Functions declaration
 ********************************************************************/

/********************************************************************
* Local Function declaration
 ********************************************************************/
eat_bool simcom_sms_msg_read(u16 index);
void simcom_sms_setting(void);
 
/********************************************************************
* Local Function
 ********************************************************************/

/*new sms message callback function*/
static eat_sms_new_message_cb(EatSmsNewMessageInd_st smsNewMessage)
{
    eat_trace("eat_sms_new_message_cb, storage=%d,index=%d",smsNewMessage.storage,smsNewMessage.index);

    simcom_sms_msg_read(smsNewMessage.index);  
}

/*receive flash sms message callback function*/
static eat_sms_flash_message_cb(EatSmsReadCnf_st smsFlashMessage)
{
    u8 format =0;

    eat_get_sms_format(&format);
    eat_trace("eat_sms_flash_message_cb, format=%d",format);
    if(1 == format)//TEXT mode
    {
        eat_trace("eat_sms_read_cb, msg=%s",smsFlashMessage.data);
        eat_trace("eat_sms_read_cb, datetime=%s",smsFlashMessage.datetime);
        eat_trace("eat_sms_read_cb, name=%s",smsFlashMessage.name);
        eat_trace("eat_sms_read_cb, status=%d",smsFlashMessage.status);
        eat_trace("eat_sms_read_cb, len=%d",smsFlashMessage.len);
        eat_trace("eat_sms_read_cb, number=%s",smsFlashMessage.number);
    }
    else//PDU mode
    {
        eat_trace("eat_sms_read_cb, msg=%s",smsFlashMessage.data);
        eat_trace("eat_sms_read_cb, len=%d",smsFlashMessage.len);
    }
}

/*send sms message callback function*/
static void eat_sms_send_cb(eat_bool result)
{
    eat_trace("eat_sms_send_cb, result=%d",result);
}


/*sms ready callback function*/
static void eat_sms_ready_cb(eat_bool result)
{

    if (TRUE == result)
    {
        //SMS Ready, SMS example entry
        eat_trace("SMS Ready");
        simcom_sms_setting();
    }    
    else
    {
        eat_trace("SMS Not Ready");
    }
}

/*SMS service init, register callback function*/
void simcom_sms_init(void)
{
    eat_set_sms_operation_mode(EAT_TRUE);
    eat_sms_register_new_message_callback(eat_sms_new_message_cb);
    eat_sms_register_flash_message_callback(eat_sms_flash_message_cb);
    eat_sms_register_send_completed_callback(eat_sms_send_cb);
    eat_sms_register_sms_ready_callback(eat_sms_ready_cb);
}

/********************************************************************
 * SMS PROCESS FUNCTION 
 *******************************************************************/
eat_bool simcom_sms_send(u8* number, u8* msg, u16 msgLen)
{
    
    if(NULL == number){
        return EAT_FALSE;
    }

    if(msgLen > 160){
        return EAT_FALSE;
    }

    eat_trace("simcom_sms_msg_send number=%s, msg=%s", number,msg);

    eat_set_sms_format(g_sms_info.formatType);  //text format
    eat_send_text_sms(number,msg);

    return EAT_TRUE;
}

eat_bool simcom_sms_sc_set(u8* number)
{
    if(NULL == number){
        return FALSE;
    }

    eat_trace("simcom_sms_sc_set Number=%s",number);

    return eat_set_sms_sc(number);
}

eat_bool simcom_sms_format_set(u8 nFormatType)
{
    eat_trace("simcom_sms_format_set FormatType=%d", nFormatType);

    return eat_set_sms_format(nFormatType);
}

eat_bool simcom_sms_cnmi_set(u8 mt)
{
    eat_trace("simcom_sms_cnmi_set %d", mt);
    
    return eat_set_sms_cnmi(2,mt,0,0,0);
}

static void eat_sms_read_cb(EatSmsReadCnf_st  smsReadCnfContent)
{
    u8 format =0;

    eat_get_sms_format(&format);
    eat_trace("eat_sms_read_cb, format=%d",format);
    if(1 == format)//TEXT mode
    {
        eat_trace("eat_sms_read_cb, msg=%s",smsReadCnfContent.data);
        eat_trace("eat_sms_read_cb, datetime=%s",smsReadCnfContent.datetime);
        eat_trace("eat_sms_read_cb, name=%s",smsReadCnfContent.name);
        eat_trace("eat_sms_read_cb, status=%d",smsReadCnfContent.status);
        eat_trace("eat_sms_read_cb, len=%d",smsReadCnfContent.len);
        eat_trace("eat_sms_read_cb, number=%s",smsReadCnfContent.number);
    }
    else//PDU mode
    {
        eat_trace("eat_sms_read_cb, msg=%s",smsReadCnfContent.data);
        eat_trace("eat_sms_read_cb, name=%s",smsReadCnfContent.name);
        eat_trace("eat_sms_read_cb, status=%d",smsReadCnfContent.status);
        eat_trace("eat_sms_read_cb, len=%d",smsReadCnfContent.len);
    }
}

eat_bool simcom_sms_msg_read(u16 index)
{    
    eat_trace("simcom_sms_msg_read index = %d", index);

    return eat_read_sms(index,eat_sms_read_cb);
}

static void eat_sms_delete_cb(eat_bool result)
{
    eat_trace("eat_sms_delete_cb, result=%d",result);
}

eat_bool simcom_sms_msg_delete(u16 index)
{
    eat_trace("simcom_sms_msg_delete index = %d", index);

    return eat_delete_sms(index,eat_sms_delete_cb);
}

void simcom_sms_setting(void)
{    
    if(TRUE == eat_get_sms_ready_state())
    {   
        simcom_sms_format_set(g_sms_info.formatType);
        simcom_sms_cnmi_set(g_sms_info.mt);
        simcom_sms_send((u8 *)g_sms_info.phone_number,(u8 *)g_sms_info.sms_string,g_sms_info.msg_len);
    }
    else
    {
        eat_trace("SMS Not Ready");
    }
}
