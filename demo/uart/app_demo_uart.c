/********************************************************************
 *                Copyright Simcom(shanghai)co. Ltd.                   *
 *---------------------------------------------------------------------
 * FileName      :   app_demo_uart.c
 * version       :   0.10
 * Description   :   
 * Authors       :   Maobin
 * Notes         :
 *---------------------------------------------------------------------
 *
 *    HISTORY OF CHANGES
 *---------------------------------------------------------------------
 *0.10  2012-09-24, Maobin, create originally.
 *
 *--------------------------------------------------------------------
 *  Description:
 *   UART1: app channel. 
 *   UART2: Debug port
 *   UART3: AT command channel
 *          In this example,APP gets AT cmd data from UART3,and sends it to Modem,
 *          then gets response message from Modem and sends to UART3.
 *  Data flow graph:
 *         "AT" 
 *   UART1 --->APP ---> Modem AT
 *     |             "OK" |
 *      <---- APP <-------
 *
 *            "AT"
 *   UART3 <=======> Modem AT
 *            "OK"
 **************************************************************************/

/******************************************************************************
 *  Include Files
 ******************************************************************************/
#include "eat_modem.h"
#include "eat_interface.h"
#include "eat_uart.h"
#include "eat_clib_define.h" //only in main.c

/********************************************************************
 * Macros
 ********************************************************************/
#define EAT_AT_TEST
//#define EAT_DEBUG_STRING_INFO_MODE //output debug string info to debug port

#define EAT_UART_RX_BUF_LEN_MAX 2000
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
static u8 rx_buf[EAT_UART_RX_BUF_LEN_MAX + 1] = {0};
static u16 wr_uart_offset = 0; //用来标记uart未写完的数据
static u16 wr_uart_len = 0; //下次需要往串口写的数据长度

static const EatUart_enum eat_uart_app= EAT_UART_1;
static const EatUart_enum eat_uart_debug = EAT_UART_USB;
static const EatUart_enum eat_uart_at = EAT_UART_2;

/********************************************************************
 * External Functions declaration
 ********************************************************************/
extern void APP_InitRegions(void);

/********************************************************************
 * Local Function declaration
 ********************************************************************/
void app_main(void *data);
void app_user1(void *data);
void app_func_ext1(void *data);

static void mdm_rx_proc(const EatEvent_st* event);
static void uart_rx_proc(const EatEvent_st* event);
static void uart_send_complete_proc(const EatEvent_st* event);
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
		(app_user_func)app_user1,//app_user1,
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

/***************************************************************************
 * Local Functions
 ***************************************************************************/
static void app_func_ext1(void *data)
{
#ifdef EAT_DEBUG_STRING_INFO_MODE
    EatUartConfig_st cfg =
    {
        EAT_UART_BAUD_115200,
        EAT_UART_DATA_BITS_8,
        EAT_UART_STOP_BITS_1,
        EAT_UART_PARITY_NONE
    };
#endif
    eat_uart_set_at_port(eat_uart_at);

    eat_uart_set_debug(eat_uart_debug);
    //set debug string info to debug port
#ifdef EAT_DEBUG_STRING_INFO_MODE
    if( EAT_UART_USB == eat_uart_debug)
    {
        eat_uart_set_debug_config(EAT_UART_DEBUG_MODE_UART, NULL);
    }else
    {
        eat_uart_set_debug_config(EAT_UART_DEBUG_MODE_UART, &cfg);
    }
#endif
    //eat_pin_set_mode(EAT_PIN24_COL4, EAT_PIN_MODE_GPIO);

}

static void uart_rx_proc(const EatEvent_st* event)
{
    u16 len;
    EatUart_enum uart = event->data.uart.uart;

    len = eat_uart_read(uart, rx_buf, EAT_UART_RX_BUF_LEN_MAX);
    if(len != 0)
    {
        rx_buf[len] = '\0';
        eat_trace("[%s] uart(%d) rx: %s", __FUNCTION__, uart, rx_buf);

#if defined(EAT_AT_TEST)
        if (uart == eat_uart_app)
        {
            eat_modem_write(rx_buf, len);
        }
        else
#endif
        {
            eat_uart_write(uart, rx_buf, len);
        }
    }
}

static void mdm_rx_proc(const EatEvent_st* event)
{
    u16 len;

    do
    {
        len = eat_modem_read(rx_buf, EAT_UART_RX_BUF_LEN_MAX );
        rx_buf[len] = '\0';
        //eat_trace("[%s] uart(%d) rx from modem: %s", __FUNCTION__, eat_uart_app, rx_buf);
#if defined(EAT_AT_TEST)
        wr_uart_offset = eat_uart_write(eat_uart_app, rx_buf, len);
        //表示UART buffer已满，则剩余的数据要在 EAT_EVENT_UART_READY_WR消息中继续处理
        if(wr_uart_offset<len)
        {
            wr_uart_len = len - wr_uart_offset;
            break;
        }
#endif
    }while(len==EAT_UART_RX_BUF_LEN_MAX );//从modem过来数据未读完
}

static void uart_ready_wr_proc(void)
{
    u16 len;
    #if defined(EAT_AT_TEST)
    len = eat_uart_write(eat_uart_app, &rx_buf[wr_uart_offset], wr_uart_len);
    if( len < wr_uart_len)
    {
        wr_uart_offset += len;
        wr_uart_len -= len;
        return;
    }
    do
    {
        len = eat_modem_read(rx_buf, EAT_UART_RX_BUF_LEN_MAX );
        if(len>0)
        {
            rx_buf[len] = '\0';
            //eat_trace("[%s] uart(%d) rx from modem: %s", __FUNCTION__, eat_uart_app, rx_buf);
            wr_uart_offset = eat_uart_write(eat_uart_app, rx_buf, len);
            if(wr_uart_offset<len)
            {
                wr_uart_len = len - wr_uart_offset;
                break;
            }
        }
    }while(len==EAT_UART_RX_BUF_LEN_MAX );
    #endif
}

static void uart_send_complete_proc(const EatEvent_st* event)
{
    eat_trace("[%s] uart(%d) send complete", __FUNCTION__, event->data.uart.uart);	
}

void uart_set(eat_bool on_off)
{
    EatUartConfig_st uart_config;
    if(on_off)
    {
        if(eat_uart_open(eat_uart_app ) == EAT_FALSE)
        {
            eat_trace("[%s] uart(%d) open fail!", __FUNCTION__, eat_uart_app);
        }else
        {
            if( EAT_UART_USB != eat_uart_app )//usb port not need to config
            {
                uart_config.baud = EAT_UART_BAUD_115200;
                uart_config.dataBits = EAT_UART_DATA_BITS_8;
                uart_config.parity = EAT_UART_PARITY_NONE;
                uart_config.stopBits = EAT_UART_STOP_BITS_1;
                if(EAT_FALSE == eat_uart_set_config(eat_uart_app, &uart_config))
                {
                    eat_trace("[%s] uart(%d) set config fail!", __FUNCTION__, eat_uart_app);
                }
                //eat_uart_set_send_complete_event(eat_uart_app, EAT_TRUE);
            }
        }
    }else
    {
        eat_uart_close(eat_uart_app );
    }
}
static void app_main(void *data)
{
	EatEvent_st event;
    u32 counter = 0;


    APP_InitRegions();//Init app RAM, first step
    APP_init_clib(); //C library initialize, second step
    uart_set(EAT_TRUE);
    
    while(1)
    {
        eat_get_event(&event);
        eat_trace("[%s] event_id=%d", __FUNCTION__, event.event);

		if (event.event == EAT_EVENT_NULL || event.event >= EAT_EVENT_NUM)
		{
			eat_trace("[%s] invalid event type", __FUNCTION__, event.event);
			continue;
		}

		switch (event.event)
		{
			case EAT_EVENT_MDM_READY_RD:
				mdm_rx_proc(&event);
				break;
			case EAT_EVENT_MDM_READY_WR:
				eat_trace("[%s] modem event :%s", __FUNCTION__, "EAT_EVENT_MDM_READY_WR");
				break;
			case EAT_EVENT_UART_READY_RD:
				uart_rx_proc(&event);
				break;
			case EAT_EVENT_UART_READY_WR:
				eat_trace("[%s] uart(%d) event :%s", __FUNCTION__, event.data.uart, "EAT_EVENT_UART_READY_WR");
				uart_ready_wr_proc();
				break;
			case EAT_EVENT_UART_SEND_COMPLETE:
				uart_send_complete_proc(&event);
				break;

			default:
				break;
		}
    }	
}

void test_handler_int_level(EatInt_st *interrupt)
{
    eat_trace("test_handler_int_level interrupt->pin [%d ],interrupt->level=%d",interrupt->pin,interrupt->level);
    if(!(interrupt->level))
    {
        //high level trigger,and set low level trigger
        eat_int_set_trigger(interrupt->pin, EAT_INT_TRIGGER_HIGH_LEVEL);
       eat_trace("System interrupt by interrupt->pin [%d ]TRIGGER_LOW_LEVEL",interrupt->pin);
    }
    else
    {
        //low level trigger
        eat_int_set_trigger(interrupt->pin, EAT_INT_TRIGGER_LOW_LEVEL);
        eat_trace("System interrupt by interrupt->pin [%d ] TRIGGER_HIGH_LEVEL",interrupt->pin);        
    }
}
void app_user1(void *data)
{
    u32 num,num1;
    EatEvent_st event;
    eat_trace("%s-%d: app_user1 ENTRY", __FILE__, __LINE__);

    eat_modem_write("at+creg?\r",9);
    while(1)
    {
        num1= eat_get_event_num_for_user(EAT_USER_1);
        eat_get_event_for_user(EAT_USER_1, &event);
        num= eat_get_event_num_for_user(EAT_USER_1);
        eat_trace("user1: event num =%d,%d",num1,num);
        
        if(event.event == EAT_EVENT_USER_MSG)
        {
            eat_trace("user8: %d %d %d %s %x", event.data.user_msg.src, event.data.user_msg.use_point, 
                event.data.user_msg.len, event.data.user_msg.data, event.data.user_msg.data_p);  
                       
        }else if(event.event == EAT_EVENT_INT )
        {
            test_handler_int_level(&event.data.interrupt);
        }else if(event.event == EAT_EVENT_MDM_READY_RD)
        {
            num1 = eat_modem_read(rx_buf, 200);
            if(num1 > 0)
            {
                rx_buf[num1] = 0;
                eat_trace("Task1 from modem:%s",rx_buf);
            }
        }
    }
}
