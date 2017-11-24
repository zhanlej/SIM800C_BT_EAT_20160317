/********************************************************************
 *                Copyright Simcom(shanghai)co. Ltd.                   *
 *---------------------------------------------------------------------
 * FileName      :   main.c
 * version        :   0.10
 * Description   :   
 * Authors       :   maobin
 * Notes          :
 *---------------------------------------------------------------------
 *
 *    HISTORY OF CHANGES
 *---------------------------------------------------------------------
 *0.10  2012-09-24, maobin, create originally.
 *
 *--------------------------------------------------------------------
 * File Description
 * AT+CEAT=parma1,param2
 * param1 param2 
 *   
 *--------------------------------------------------------------------
 ********************************************************************/
 
/********************************************************************
 * Include Files
 ********************************************************************/
#include "platform.h"
#include "app_at_cmd_envelope.h"
#include "app_custom.h"
#include "eat_clib_define.h" //only in main.c
#include "app_sms.h"

/********************************************************************
 * Macros
 ********************************************************************/
#define EAT_MEM_MAX_SIZE 100*1024 

/********************************************************************
 * Types
 ********************************************************************/
typedef struct 
{
    u16 w;  //write offset
    u16 r;  //read offset
    u8  buf[EAT_UART_RX_BUF_LEN_MAX];
}app_buf_st;

typedef void (*app_user_func)(void*);

/********************************************************************
 * Extern Variables (Extern /Global)
 ********************************************************************/

/********************************************************************
 * Local Variables:  STATIC
 ********************************************************************/
static EatEntryPara_st app_para={0};
static u8 s_memPool[EAT_MEM_MAX_SIZE]; 
static app_buf_st modem_rx = {0};
static app_buf_st uart_rx = {0};

/********************************************************************
 * External Functions declaration
 ********************************************************************/
extern void APP_InitRegions(void);
extern void app_at_cmd_envelope(void* data);
extern void custom_entry(void);
extern void simcom_sms_read_cb(u16 index,u8* number,u8 *msg);

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
    (app_user_func)app_at_cmd_envelope,//app_user1,
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
eat_uart_set_at_port: set AT port
*/
     EatUartConfig_st cfg =
    {
        EAT_UART_BAUD_115200,
        EAT_UART_DATA_BITS_8,
        EAT_UART_STOP_BITS_1,
        EAT_UART_PARITY_NONE
    };
    eat_uart_set_debug(app_uart_debug);
    eat_uart_set_debug_config(EAT_UART_DEBUG_MODE_UART, &cfg);
    
    eat_uart_set_at_port(app_uart_at);// UART1 is as AT PORT
    eat_modem_set_poweron_urc_dir(EAT_USER_1);
}    

static eat_bool app_mem_init(void)
{
    eat_bool ret = EAT_FALSE;
    ret = eat_mem_init(s_memPool,EAT_MEM_MAX_SIZE);
    if (!ret)
        eat_trace("ERROR: eat memory initial error!");
        
    return ret;
}

void app_main(void *data)
{
    EatEntryPara_st *para;

    APP_InitRegions();//Init app RAM, first step
    APP_init_clib();  //C library initialize, second step

    para = (EatEntryPara_st*)data;

    memcpy(&app_para, para, sizeof(EatEntryPara_st));
    eat_trace("App Main ENTRY update:%d result:%d", app_para.is_update_app,app_para.update_app_result);
    if(app_para.is_update_app && app_para.update_app_result)
    {
        eat_update_app_ok();
    }

    custom_entry();
}

/*Read the data from UART */
static void uart_rx_proc(const EatEvent_st* event)
{
    u16 len;
    EatUart_enum uart = event->data.uart.uart;
    app_buf_st* rx_p = (app_buf_st*)&uart_rx;

    do
    {
        if(rx_p->w == EAT_UART_RX_BUF_LEN_MAX)
        {
             rx_p->w = 0;
             rx_p->r = 0;
        } 
        len = eat_uart_read(uart, &rx_p->buf[rx_p->w], EAT_UART_RX_BUF_LEN_MAX-rx_p->w);
        if(len>0)
        {
            rx_p->w += len;  
        }
    }while(len > 0);

    eat_trace("Uart buf w=%d r=%d",rx_p->w, rx_p->r);
}

/*Read the data from Modem*/
static void mdm_rx_proc(void)
{
    u16 len;
    u16 w_len;
    app_buf_st* rx_p = (app_buf_st*)&modem_rx;
    do
    {
        len = eat_modem_read(&rx_p->buf[rx_p->w], EAT_UART_RX_BUF_LEN_MAX-rx_p->w);
        if(len==0)
        {
            break;
        }
        
        if(len>0)
        {
            rx_p->w += len;
            len = rx_p->w - rx_p->r;
            w_len = eat_uart_write(eat_uart_app, &rx_p->buf[rx_p->r], len);
            //The buffer is full, the remainder data in Modem will be process in EVENT EAT_EVENT_UART_READY_WR
            if(w_len<len)
            {
                rx_p->r += w_len;
               break;
            }else
            {
                rx_p->r = 0;
                rx_p->w = 0;
            }
        }
    }while(1 );
}

/*process the EAT_EVENT_UART_READY_WR, continues to write the data of Modem to UART*/
static void uart_ready_wr_proc(void)
{
    u16 len;
    u16 w_len;
    app_buf_st* rx_p = (app_buf_st*)&modem_rx;
    
    len = rx_p->w-rx_p->r;
    w_len = eat_uart_write(eat_uart_app, &rx_p->buf[rx_p->r], len);
    if( w_len < len)
    {
        rx_p->r += len;
        return;
    }else
    {
        rx_p->r = 0;
        rx_p->w = 0;
    }
    
    do
    {
        len = eat_modem_read(&rx_p->buf[rx_p->w], EAT_UART_RX_BUF_LEN_MAX-rx_p->w);
        if(len==0)
        {
            break;
        }
        if(len>0)
        {
            rx_p->w += len;
            len = rx_p->w - rx_p->r;
            w_len = eat_uart_write(eat_uart_app, &rx_p->buf[rx_p->r], len);
            if(w_len<len)
            {
               rx_p->r += w_len;
               //The eat uart buffer is full, the remainder data in Modem will be process in EVENT EAT_EVENT_UART_READY_WR
               break;
            }else
            {
                rx_p->r = 0;
                rx_p->w = 0;
            }
        }
    }while(1);
}

eat_bool app_uart_init(void)
{
    eat_bool result = EAT_FALSE;
    EatUartConfig_st uart_config;
    if(eat_uart_open(eat_uart_app ) == TRUE)
    {
        if( EAT_UART_USB != eat_uart_app )//usb port not need to config
        {
            uart_config.baud = EAT_UART_BAUD_115200;
            uart_config.dataBits = EAT_UART_DATA_BITS_8;
            uart_config.parity = EAT_UART_PARITY_NONE;
            uart_config.stopBits = EAT_UART_STOP_BITS_1;
            if(EAT_TRUE == eat_uart_set_config(eat_uart_app, &uart_config))
            {
                result = EAT_TRUE;
            }else
            {
                eat_trace("[%s] uart(%d) set config fail!", __FUNCTION__, eat_uart_app);
            }
            //eat_uart_set_send_complete_event(eat_uart_app, EAT_TRUE);
        }
    }else
    {
        eat_trace("[%s] uart(%d) open fail!", __FUNCTION__, eat_uart_app);
    }
    
    return result;
}

void custom_entry(void)
{
    EatEvent_st event;
 
    app_mem_init();  //Memory
    app_uart_init(); //UART
    
    simcom_gsm_init("1234",GsmInitCallback);
    simcom_sms_init();

    eat_uart_write(eat_uart_app,"\r\nAPP entry!\r\nVersion:1\r\n",25);
    while(EAT_TRUE)
    {
        eat_get_event(&event);
        eat_trace("MSG id%x", event.event);
        switch(event.event)
        {
            case EAT_EVENT_TIMER :
                break;
            case EAT_EVENT_MDM_READY_RD:
                {
                    mdm_rx_proc();
                }
                break;
            case EAT_EVENT_MDM_READY_WR:
                break;
            case EAT_EVENT_UART_READY_RD:
                uart_rx_proc(&event);
                break;
            case EAT_EVENT_UART_READY_WR:
                uart_ready_wr_proc();
                break;
            case EAT_EVENT_UART_SEND_COMPLETE :
                break;
            case  EAT_EVENT_INT :
               break;
            default:
                break;
        }

    }
}
