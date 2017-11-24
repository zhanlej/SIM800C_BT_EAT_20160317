/********************************************************************
 *                Copyright Simcom(shanghai)co. Ltd.                   *
 *---------------------------------------------------------------------
 * FileName      :   app_demo_spi.c
 * version       :   0.10
 * Description   :   
 * Authors       :   fangshengchang
 * Notes         :
 *---------------------------------------------------------------------
 *
 *    HISTORY OF CHANGES
 *---------------------------------------------------------------------
 *0.10  2012-09-24, fangshengchang, create originally.
 *
 *--------------------------------------------------------------------
 * File Description
 * AT+CEAT=parma1,param2
 * param1 param2 
 *   1      1     lcd display some word and Backlight on
 *   1      2     Backlight off
 *--------------------------------------------------------------------
 ********************************************************************/
/********************************************************************
 * Include Files
 ********************************************************************/
#include <stdio.h>
#include <string.h>
#include "eat_modem.h"
#include "LcdDisplay.h"
#include "eat_interface.h"
#include "eat_uart.h"
#include "eat_periphery.h"
#include "eat_clib_define.h" //only in main.c
/********************************************************************
 * Macros
 ********************************************************************/

/********************************************************************
* Types
 ********************************************************************/
typedef void (*app_user_func)(void*);
/********************************************************************
 * Extern Variables (Extern /Global)
 ********************************************************************/
 
/********************************************************************
 * Local Variables:  STATIC
 ********************************************************************/

/********************************************************************
 * External Functions declaration
 ********************************************************************/
extern void APP_InitRegions(void);

/********************************************************************
 * Local Function declaration
 ********************************************************************/
void app_main(void *data);
void app_func_ext1(void *data);
/********************************************************************
 * Local Function
 ********************************************************************/
#pragma arm section rodata = "APP_CFG"
APP_ENTRY_FLAG 
#pragma arm section rodata

#pragma arm section rodata="APPENTRY"
	const EatEntry_st AppEntry = 
	{
		app_main,
		app_func_ext1,
		(app_user_func)EAT_NULL,//app_user1,
		(app_user_func)EAT_NULL,//app_user2,
		(app_user_func)EAT_NULL,//app_user3,
		(app_user_func)EAT_NULL,//app_user4,
		(app_user_func)EAT_NULL,//app_user5,
		(app_user_func)EAT_NULL,//app_user6,
		(app_user_func)EAT_NULL,//app_user7,
		(app_user_func)EAT_NULL,//app_user8,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL
	};
#pragma arm section rodata

void app_func_ext1(void *data)
{
	/*This function can be called before Task running ,configure the GPIO,uart and etc.
	   Only these api can be used:
		 eat_uart_set_debug: set debug port
		 eat_pin_set_mode: set GPIO mode
		 eat_uart_set_at: set AT port
	*/
	eat_uart_set_debug(EAT_UART_USB);
    eat_uart_set_at_port(EAT_UART_1);// UART1 is as AT PORT
}

eat_bool eat_modem_data_parse(u8* buffer, u16 len, u8* param1, u8* param2)
{
    eat_bool ret_val = EAT_FALSE;
    u8* buf_ptr = NULL;
    /*param:%d,extern_param:%d*/
     buf_ptr = (u8*)strstr((const char *)buffer,"param");
    if( buf_ptr != NULL)
    {
        sscanf((const char *)buf_ptr, "param:%d,extern_param:%d",(int*)param1, (int*)param2);
        eat_trace("data parse param1:%d param2:%d",*param1, *param2);
        ret_val = EAT_TRUE;
    }
    return ret_val;
}


//lcd test
static void lcd_test(eat_bool display)
{    
    if(display)
    {
        eat_trace("LCD test Display on");
        LcdDisplayClear(0, 0, X_PIXELS-1, Y_PIXELS-1, EAT_TRUE);
        eat_sleep(100);
        LcdDisplayStr(0,  0, 127, 15, FONT_TYPE_MAIN, LCD_ALIGN_V_UP | LCD_ALIGN_H_LEFT, "lcd test", EAT_TRUE);
        LcdDisplayStr(0, 16, 127, 31, FONT_TYPE_MAIN, LCD_ALIGN_V_DOWN | LCD_ALIGN_H_RIGHT, "backlight test", EAT_TRUE);
        LcdDisplayStr(0, 32, 127, 47, FONT_TYPE_MAIN, 0, "ABCDEFGH", EAT_TRUE);
        LcdDisplayStr(0, 48, 127, 63, FONT_TYPE_MAIN, 0, "1234567890", EAT_TRUE);  
    }else
    {
        LcdDisplayClear(0, 0, X_PIXELS-1, Y_PIXELS-1, EAT_TRUE);
    }
}

//LCDLED test
void backlight_test(int sw)
{
   if(sw)
   {
      eat_lcd_light_sw(EAT_TRUE, EAT_BL_STEP_24_MA);
   }
   else
   {
      eat_lcd_light_sw(EAT_FALSE, 0);
   }    
}

/****************************************************
 * Timer testing module
 *****************************************************/
eat_bool eat_module_test_spi(u8 param1, u8 param2)
{
    /***************************************
     * example 1
     * used eat_gpio_write test EAT_PIN53_PCM_IN,EAT_PIN57_GPIO4
     ***************************************/
    u8 buf[100] = {0};
    if( 1 == param1 )
    {
        if( 1 == param2 )
        {          
           lcd_init();
           lcd_test(EAT_TRUE);
           backlight_test(EAT_TRUE);
           eat_trace("LCD test 1,1 start");
        }else if( 2 == param2)
        {                   
           lcd_test(EAT_FALSE);
           backlight_test(EAT_FALSE);
           eat_trace("LCD test 1,2 backlight off");
        }else if( 3==param2)
        {
           backlight_test(EAT_FALSE);
           eat_spi_read(buf, 100);
        }
        
    }  
    return EAT_TRUE;
}

u8 buf[2048];
void app_main(void *data)
{
    EatEvent_st event;
    u16 len = 0;

    APP_InitRegions();//Init app RAM
    APP_init_clib();
    eat_trace(" app_main ENTRY");
    eat_modem_write("AT+CNETLIGHT=0\r\n",16);
    eat_module_test_spi(1,1); 
    while(EAT_TRUE)
    {
        eat_get_event(&event);
        eat_trace("MSG id%x", event.event);
        switch(event.event)
        {
            case EAT_EVENT_MDM_READY_RD:
                {
                    u8 param1,param2;
                    len = 0;
                    len = eat_modem_read(buf, 2048);
                    if(len > 0)
                    {
                        //Get the testing parameter
                        if(eat_modem_data_parse(buf,len,&param1,&param2))
                        {
                            //Entry SPI test module
                            eat_module_test_spi(param1, param2);
                        }
                        else
                        {
                            eat_trace("From Mdm:%s",buf);
                        }
                    }

                }
                break;
            case EAT_EVENT_MDM_READY_WR:
            case EAT_EVENT_UART_READY_RD:
                break;
            case EAT_EVENT_UART_SEND_COMPLETE :
                break;
            default:
                break;
        }

    }

}


