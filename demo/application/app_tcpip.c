/********************************************************************
 *                Copyright Simcom(shanghai)co. Ltd.                   *
 *---------------------------------------------------------------------
 * FileName     :   app_tcpip.c
 * version       :   0.10
 * Description  :   
 * Authors      :   heweijiang
 * Notes         :
 *---------------------------------------------------------------------
 *
 *    HISTORY OF CHANGES
 *
 *0.10  2014-06-11, heweijiang, create originally.
 *---------------------------------------------------------------------
 *
 *About socket APT useage,please refer to the API description in eat_socket.h
 *
 *
 ********************************************************************/
/********************************************************************
 * Include Files
 ********************************************************************/
#include <stdio.h>
#include <string.h>
#include "eat_type.h"
#include "eat_socket.h"
#include "eat_interface.h"
#include "app_custom.h"
#include "app_at_cmd_envelope.h"

/********************************************************************
* Macros
 ********************************************************************/
 
/********************************************************************
* Types
 ********************************************************************/

sockaddr_struct g_server_address =
{
    SOC_SOCK_STREAM,
    4,
    5107,              //server port
    {116,247,119,165}  //server IP  
};

/********************************************************************
* Extern Variables (Extern /Global)
 ********************************************************************/
 
/********************************************************************
* Local Variables:  STATIC
 ********************************************************************/
u8 *SOC_EVENT[]={
    "SOC_READ",
    "SOC_WRITE",  
    "SOC_ACCEPT", 
    "SOC_CONNECT",
    "SOC_CLOSE", 
    "SOC_ACKED"
};

u8 *BEARER_STATE[]={
    "DEACTIVATED",
    "ACTIVATING",
    "ACTIVATED",
    "DEACTIVATING",
    "CSD_AUTO_DISC_TIMEOUT",
    "GPRS_AUTO_DISC_TIMEOUT",
    "NWK_NEG_QOS_MODIFY",
    "CBM_WIFI_STA_INFO_MODIF",
};

/********************************************************************
* External Functions declaration
 ********************************************************************/
eat_bool simcom_fota_update(u8 *username, u8 *pwd, u8* getName,u8* getPath, u8* serv, u8 port);

/********************************************************************
* Local Function declaration
 ********************************************************************/
s8 simcom_connect_server(sockaddr_struct *addr);
s32 simcom_send_to_server(s8 sid, const void *buf, s32 len);
 
/********************************************************************
* Local Function
 ********************************************************************/ 

/**************************************************************************************
 * call app_update() to update new app bin
 *************************************************************************************/
ResultNotifyCb ftpgettofs_final_cb(eat_bool result)
{
    u8 buffer[128] = {0};

    eat_trace("ftpgettofs_cb final result = %d\r\n", result);

    if (result)
    {        
        eat_uart_write(eat_uart_app,"\r\nTEST:APP UPDATE...\r\n",22);
        app_update("c:\\User\\Ftp\\app.bin");
    }
    else
    {
        //Do something
        eat_trace("FTP download fail");
    }
}

ResultNotifyCb ftpgettofs_cb(eat_bool result)
{
    u8 buffer[128] = {0};

    eat_trace("ftpgettofs_cb result = %d\r\n", result);
    
    if(!result)
    {        
       //Do something       
       eat_trace("ftpgettofs fail");
    }
}

/*****************************************************************************
* Function :   soc_notify_cb
* Description: call back function of soc_create()
* Parameters :
*     s - the socket ID
*     event - the notify event about current socket, please refer to soc_event_enum
*     result - if event is SOC_CONNECT, this paramter indicate the result of connect
*     ack_size - if event is  SOC_ACKED, this parameter is valid, it indicate the acked 
* Returns:
*****************************************************************************/
eat_bool simcom_fota_update(u8 *username, u8 *pwd, u8* getName,u8* getPath, u8* serv, u8 port)
{
    eat_uart_write(eat_uart_app,"\r\nTEST:APP DOWNLOAD...\r\n",24);

    simcom_ftp_down_file(username, pwd, getName, getPath, 
                        serv, port, "app.bin", ftpgettofs_cb, ftpgettofs_final_cb);
}

/*****************************************************************************
* Function :   soc_notify_cb
* Description: call back function of soc_create()
* Parameters :
*     s - the socket ID
*     event - the notify event about current socket, please refer to soc_event_enum
*     result - if event is SOC_CONNECT, this paramter indicate the result of connect
*     ack_size - if event is  SOC_ACKED, this parameter is valid, it indicate the acked 
* Returns:
*****************************************************************************/
void soc_notify_cb(s8 s,soc_event_enum event,eat_bool result, u16 ack_size)
{
    u8 buffer[128] = {0};
    u8 id = 0;    
    static u8 connect_count = 0;

    eat_trace("soc_notify_cb()");
    
    if(event & SOC_READ)
    {
        s16 len = 0;
        u8  buf[2000]={0};
        id = 0;

        while(1)
        {
            len = eat_soc_recv(s,buf,sizeof(buf));
            if (len>0)
            {
                //Read from Server.TODO something
                eat_trace("Recv data len=%d, %s",len, buf);
            }
            else
                break;
        }
    }
    else if (event&SOC_WRITE) id = 1;
    else if (event&SOC_ACCEPT) id = 2;
    else if (event&SOC_CONNECT)
    {
        id = 3;

        if (EAT_TRUE == result)
        {
            //Connect Server Successful,TODO something
            eat_trace("Conncet Successful");
            simcom_send_to_server(s,"Hello", 6);
        }
        else
        {       
            eat_trace("Socket connect error,result=%d",result);
            if (connect_count < 3)
            {   
                eat_trace("connect again %d", connect_count);
                eat_soc_close(s);
                simcom_connect_server(&g_server_address);
                connect_count++;
            }
        }
    }
    else if (event&SOC_CLOSE){ 
        id = 4;
        //eat_soc_close(s);        
        eat_trace("----SOC_CLOSE---");        
        simcom_connect_server(&g_server_address);
    }
    else if (event&SOC_ACKED) id = 5;

    if (id == 5)
        sprintf(buffer,"SOC_NOTIFY:%d,%s,%d\r\n",s,SOC_EVENT[id],ack_size);
    else 
        sprintf(buffer,"SOC_NOTIFY:%d,%s,%d\r\n",s,SOC_EVENT[id],result);
    eat_trace(buffer,strlen(buffer));

    if(SOC_ACCEPT==event){
        u8 val = 0;
        s8 ret = 0;
        sockaddr_struct clientAddr={0};
        s8 newsocket = eat_soc_accept(s,&clientAddr);
        if (newsocket < 0){
            eat_trace("eat_soc_accept return error :%d",newsocket);
        }
        else{
            sprintf(buffer,"client accept:%s,%d:%d:%d:%d\r\n",clientAddr.addr[0],clientAddr.addr[1],clientAddr.addr[2],clientAddr.addr[3]);
        }

        val = TRUE;
        ret = eat_soc_setsockopt(s, SOC_NODELAY, &val, sizeof(val));
        if (ret != SOC_SUCCESS)
            eat_trace("eat_soc_setsockopt SOC_NODELAY return error :%d",ret);

    }
}

/*****************************************************************************
* Function :   simcom_create_server
* Description: Creat a server
* Parameters :
*     port   - the server port
* Returns:
*     socket id
*****************************************************************************/
s8 simcom_create_server(u16 port)
{
    u8 val;    
    s8 ret;
    s8 server_socket;    
    sockaddr_struct address={0};

    eat_soc_notify_register(soc_notify_cb);
    server_socket = eat_soc_create(SOC_SOCK_STREAM,0);
    if(server_socket < 0)
        eat_trace("eat_soc_create() return error :%d",server_socket);
    
    val = TRUE;
    ret = eat_soc_setsockopt(server_socket, SOC_NBIO, &val, sizeof(val));
    if (ret != SOC_SUCCESS)
        eat_trace("eat_soc_setsockopt() return error :%d",ret);
    
    val = (SOC_READ | SOC_WRITE | SOC_CLOSE | SOC_CONNECT|SOC_ACCEPT);
    ret = eat_soc_setsockopt(server_socket,SOC_ASYNC,&val,sizeof(val));
    if (ret != SOC_SUCCESS)
        eat_trace("eat_soc_setsockopt() return error :%d",ret);

    address.port = port;
    eat_soc_bind(server_socket,&address);
    eat_soc_listen(server_socket,1);

    return server_socket;
}

/*****************************************************************************
* Function :   bear_notify_cb
* Description: The callback function of eat_gprs_bearer_open
* Parameters :
*     state   - the state of bearer open 
*     ip_addr - 
* Returns:
*     
*****************************************************************************/
void bear_notify_cb(cbm_bearer_state_enum state,u8 ip_addr[4])
{
    u8 buffer[128] = {0};
    u8 id = 0;
    u8 sid;

    eat_trace("bear_notify_cb()");

    if (state & CBM_DEACTIVATED) id = 0;
    else if (state & CBM_ACTIVATING) id = 1;
    else if (state & CBM_ACTIVATED) id = 2;
    else if (state & CBM_DEACTIVATING) id = 3;
    else if (state & CBM_CSD_AUTO_DISC_TIMEOUT) id = 4;
    else if (state & CBM_GPRS_AUTO_DISC_TIMEOUT) id = 5;
    else if (state & CBM_NWK_NEG_QOS_MODIFY) id = 6;
    else if (state & CBM_WIFI_STA_INFO_MODIFY) id = 7;

    if (id == 2)
    {
        sprintf(buffer,"BEAR_NOTIFY:%s,%d:%d:%d:%d\r\n",BEARER_STATE[id],ip_addr[0],ip_addr[1],ip_addr[2],ip_addr[3]);
        //eat_uart_write(EAT_UART_2,buffer,strlen(buffer));
    }
    else
    {
        sprintf(buffer,"BEAR_NOTIFY:%s\r\n",BEARER_STATE[id]);
    }

    eat_trace(buffer,strlen(buffer));

    if (id == 2){
        //Connect Successful, TODO something.
        sid = simcom_connect_server(&g_server_address);
        if(sid < 0)
            eat_trace("simcom_connect_server return error :%d",sid);
    }

}

void simcom_gprs_start(u8* apn,u8* userName,u8* password)
{
   eat_gprs_bearer_open(apn, userName, password,bear_notify_cb);
}

/*****************************************************************************
* Function :   simcom_connect_server
* Description: Connect to server
* Parameters :
*     addr -  pointer of server address 
* Returns:
*     s - socket id
*****************************************************************************/
s8 simcom_connect_server(sockaddr_struct *addr)
{
    u8 val = 0;
    s8 ret = 0;
    s8 sid;

    eat_soc_notify_register(soc_notify_cb); //register socket callback
    
    sid = eat_soc_create(SOC_SOCK_STREAM,0);
    if(sid < 0)
        eat_trace("eat_soc_create return error :%d",sid);
    
    val = (SOC_READ | SOC_WRITE | SOC_CLOSE | SOC_CONNECT|SOC_ACCEPT);
    ret = eat_soc_setsockopt(sid,SOC_ASYNC,&val,sizeof(val));
    if (ret != SOC_SUCCESS)
        eat_trace("eat_soc_setsockopt 1 return error :%d",ret);
    
    val = TRUE;
    ret = eat_soc_setsockopt(sid, SOC_NBIO, &val, sizeof(val));
    if (ret != SOC_SUCCESS)
        eat_trace("eat_soc_setsockopt 2 return error :%d",ret);
    
    val = TRUE;
    ret = eat_soc_setsockopt(sid, SOC_NODELAY, &val, sizeof(val));
    if (ret != SOC_SUCCESS)
        eat_trace("eat_soc_setsockopt 3 return error :%d",ret);
        
    ret = eat_soc_connect(sid,addr); 
    if(ret >= 0){
        eat_trace("NEW Connection ID is :%d",ret);
    }
    else if (ret == SOC_WOULDBLOCK) {
        eat_trace("Connection is in progressing");
    }
    else {
        eat_trace("Connect return error:%d",ret);
    }

    return sid;
}

/*****************************************************************************
* Function :   simcom_send_to_server
* Description: Send data to server,please refer to the eat_soc_send() description in eat_socket.h
* Parameters :
*     sid  - socket id
*     buf - buffer for sending data
*     len -  buffer size
* Returns:
*     >=0 : SUCCESS
*     SOC_INVALID_SOCKET : invalid socket id
*     SOC_INVAL : buf is NULL or len equals to zero
*     SOC_WOULDBLOCK : buffer not available or bearer is establishing
*     SOC_BEARER_FAIL : bearer broken
*     SOC_NOTCONN : socket is not connected in case of TCP
*     SOC_PIPE : socket is already been shutdown
*     SOC_MSGSIZE : message is too long
*     SOC_ERROR : unspecified error
*     SOC_NOTBIND : in case of sending ICMP Echo Request, shall bind before
*****************************************************************************/
s32 simcom_send_to_server(s8 sid, const void *buf, s32 len)
{
    s32 ret = 0;
    ret = eat_soc_send(sid,buf,len);
    if (ret < 0)
        eat_trace("eat_soc_send return error :%d",ret);
    else
        eat_trace("eat_soc_send success :%d",ret);

    return ret;
}

/*****************************************************************************
* Function :   simcom_recv_from_server
* Description: Receive data from server,please refer to the eat_soc_recv() description in eat_socket.h
* Parameters :
*     sid  - socket id
*     buf - buffer for receiving data
*     len -  buffer size
* Returns:
*     0 :                   receive the FIN from the server
*     SOC_INVALID_SOCKET :  invalid socket id
*     SOC_INVAL :           buf is NULL or len equals to zero
*     SOC_WOULDBLOCK :      no data available
*     SOC_BEARER_FAIL :     bearer broken
*     SOC_NOTCONN :         socket is not connected in case of TCP
*     SOC_PIPE :            socket is already been shutdown
*     SOC_ERROR :           unspecified error
*****************************************************************************/
s32 simcom_recv_from_server(s8 sid, void *buf, s32 len)
{
    s32 ret = 0;

    ret = eat_soc_recv(sid,buf,len);
    if(ret == SOC_WOULDBLOCK){
        eat_trace("eat_soc_recv no data available");
    }
    else if(ret > 0) {
        eat_trace("eat_soc_recv data:%s",buf);
    }
    else{
        eat_trace("eat_soc_recv return error:%d",ret);
    }
}

/*****************************************************************************
* Function :   hostname_notify_cb
* Description: The callback function of simcom_gethostbyname 
* Parameters :
*     request_id - it set by  eat_soc_gethostbyname
*     result - the result about gethostname.
*     ip_addr[4] - if the result is TRUE, this parameter indicate the IP address
*                            of hostname.
* Returns:
*     EAT_FALSE or EAT_TRUE
*****************************************************************************/
void hostname_notify_cb(u32 request_id,eat_bool result,u8 ip_addr[4])
{
    //u8 buffer[128] = {0};
    //sprintf(buffer,"HOSTNAME_NOTIFY:%d,%d,%d:%d:%d:%d\r\n",request_id,result,ip_addr[0],ip_addr[1],ip_addr[2],ip_addr[3]);
    //eat_uart_write(EAT_UART_2,buffer,strlen(buffer));
    eat_trace("hostname_notify_cb");
    
}

/*****************************************************************************
* Function :   simcom_gethostbyname
* Description: This function gets the IP of the given domain name. 
* Parameters :
*     domain_name   - domain name
* Returns:
*     EAT_FALSE or EAT_TRUE
*****************************************************************************/
eat_bool simcom_gethostbyname(const char *domain_name)
{
    u8 len;
    u8 ipaddr[4];    
    s32 result = 0;
    eat_bool ret = EAT_FALSE;     
    
    eat_soc_gethost_notify_register(hostname_notify_cb);
    
    result = eat_soc_gethostbyname(domain_name,ipaddr,&len,1234);
    if (SOC_SUCCESS == result){
        //u8 buffer[128] = {0};
        eat_trace("eat_soc_gethostbyname success");
        //sprintf(buffer,"HOSTNAME:%d,%d:%d:%d:%d\r\n",ipaddr[0],ipaddr[1],ipaddr[2],ipaddr[3]);
        //eat_uart_write(EAT_UART_1,buffer,strlen(buffer));
        ret = EAT_TRUE;
    } else if(SOC_WOULDBLOCK == result){
        eat_trace("eat_soc_gethostbyname wait callback function");
    } else
        eat_trace("eat_soc_gethostbyname error");

    return ret;
}

/*****************************************************************************
* Function :   simcom_tcpip_test
* Description: socket connect and app update usage exemple
* Parameters :
* Returns:
*****************************************************************************/
void simcom_tcpip_test(void)
{
    //socket usage example entry
    simcom_gprs_start(g_modemConfig.apnName,g_modemConfig.apnUserName,g_modemConfig.apnPassword);
    
    //app update example entry
    /*simcom_fota_update(g_modemConfig.ftpUserName, g_modemConfig.ftpPassword,
                                g_modemConfig.ftpFileName, g_modemConfig.ftpFilePath, g_modemConfig.FTPServerIP, g_modemConfig.ftpPort);*/
}
