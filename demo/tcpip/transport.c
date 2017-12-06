/******************************************************************************
 *  Include Files
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>  
#include "eat_modem.h"
#include "eat_interface.h"
#include "eat_uart.h"
#include "eat_timer.h" 
#include "eat_socket.h"
#include "transport.h"
#include "MQTTPacket.h"  //MQTT



/********************************************************************
 * Macros
 ********************************************************************/
#define MQTT_BUF_SIZE 1024

/********************************************************************
 * Types
 ********************************************************************/

/********************************************************************
 * Extern Variables (Extern /Global)
 ********************************************************************/
u8 *SOC_EVENT[]={
    "SOC_READ",
    "SOC_WRITE",  
    "SOC_ACCEPT", 
    "SOC_CONNECT",
    "SOC_CLOSE", 
    "SOC_ACKED"
};

extern s8 socket_id;
 
/********************************************************************
 * Local Variables:  STATIC
 ********************************************************************/
char deviceID[20] = "200025";
char topic_group[30];
int len = 0;
unsigned char mqtt_send_buf[MQTT_BUF_SIZE]; 
unsigned char mqtt_receive_buf[MQTT_BUF_SIZE]; 
MQTTString topicString = MQTTString_initializer;
char payload[MQTT_BUF_SIZE] = "testmessage\n";	//MQTT发布出去的数据
int payloadlen = MQTT_BUF_SIZE;
u32 mqtt_ack_flag = 0; //记录mqtt需要接收的ack标志

/********************************************************************
 * External Functions declaration
 ********************************************************************/


/********************************************************************
 * Local Function declaration
 ********************************************************************/
eat_soc_notify soc_notify_cb(s8 s,soc_event_enum event,eat_bool result, u16 ack_size);
void mqtt_ack_set(int ack_type);
void mqtt_ack_clear(int ack_type);
eat_bool mqtt_connect();
eat_bool mqtt_subscribe();
eat_bool mqtt_connack();

/********************************************************************
 * Local Function
 ********************************************************************/
int transport_open(char* addr, int port)
{
    u32 VAL;
    u8 val = 0;
    s8 ret = 0;
    sockaddr_struct address={0};
    eat_soc_notify_register(soc_notify_cb);
    socket_id = eat_soc_create(SOC_SOCK_STREAM,0);
    if(socket_id < 0)
        my_printf("eat_soc_create return error :%d",socket_id);
    val = (SOC_READ | SOC_WRITE | SOC_CLOSE | SOC_CONNECT|SOC_ACCEPT);
    ret = eat_soc_setsockopt(socket_id,SOC_ASYNC,&val,sizeof(val));
    if (ret != SOC_SUCCESS)
        my_printf("eat_soc_setsockopt 1 return error :%d",ret);

    val = TRUE;
    ret = eat_soc_setsockopt(socket_id, SOC_NBIO, &val, sizeof(val));
    if (ret != SOC_SUCCESS)
        my_printf("eat_soc_setsockopt 2 return error :%d",ret);

    val = TRUE;
    ret = eat_soc_setsockopt(socket_id, SOC_NODELAY, &val, sizeof(val));
    if (ret != SOC_SUCCESS)
        my_printf("eat_soc_setsockopt 3 return error :%d",ret);

    ret = eat_soc_getsockopt(socket_id, SOC_NODELAY, &VAL, sizeof(VAL));
    if (ret != SOC_SUCCESS)
        my_printf("eat_soc_getsockopt  return error :%d",ret);
    else 
        my_printf("eat_soc_getsockopt return %d",val);

    address.sock_type = SOC_SOCK_STREAM;
    address.addr_len = 4;
    address.port = port;                /* TCP server port */
#if 0
    address.addr[0]=222;                /* TCP server ip address */
    address.addr[1]=29;
    address.addr[2]=40;
    address.addr[3]=68;
#else
    address.addr[0]=47;                /* TCP server ip address */
    address.addr[1]=92;
    address.addr[2]=81;
    address.addr[3]=9;
#endif
    ret = eat_soc_connect(socket_id,&address); 
    if(ret >= 0){
        my_printf("NEW Connection ID is :%d",ret);
    }
    else if (ret == SOC_WOULDBLOCK) {
        my_printf("Connection is in progressing");
    }
    else {
        my_printf("Connect return error:%d",ret);
    }
}

void my_msg_send(EatUser_enum user_src, EatUser_enum user_dst, int id, int mqtt_status)
{
    eat_msg_t msg;
    memset(&msg, 0, sizeof(eat_msg_t));
    msg.id = id;
    msg.mqtt_status = mqtt_status;
    eat_send_msg_to_user(user_src, user_dst, EAT_FALSE, sizeof(eat_msg_t), (u8 *)&msg, NULL);
}

eat_soc_notify soc_notify_cb(s8 s,soc_event_enum event,eat_bool result, u16 ack_size)
{
    u8 buffer[128] = {0};
    u8 id = 0;
    int rc = 0;
    /*Transfer struct date*/
    eat_msg_t msg;

    if(event&SOC_READ) {id = 0;
        socket_id = s;
    }
    else if (event&SOC_WRITE) id = 1;
    else if (event&SOC_ACCEPT) id = 2;
    else if (event&SOC_CONNECT) id = 3;
    else if (event&SOC_CLOSE){ id = 4;
        eat_soc_close(s);
    }
    else if (event&SOC_ACKED) id = 5;

    switch(id)
    {
        case 0:
            memset(mqtt_receive_buf, 0, MQTT_BUF_SIZE);
            rc = MQTTPacket_read(mqtt_receive_buf, MQTT_BUF_SIZE, transport_getdata);
            if(rc != -1)
            {
                my_msg_send(EAT_USER_0, EAT_USER_0, SOC_READ, rc);
            }
            break;
        case 3:
            my_msg_send(EAT_USER_0, EAT_USER_0, SOC_CONNECT, CONNECT);
            break;
        case 4:
            eat_uart_write(EAT_UART_1,"TCP CLOSED\r\n",14);
            my_printf("module_reset_test");
            eat_reset_module();
        case 5:
            break;
        default:
            break;
    }
    sprintf(buffer,"SOC_NOTIFY:%d,%s,%d\r\n",s,SOC_EVENT[id],result);
    eat_uart_write(EAT_UART_1,buffer,strlen(buffer));

    if(SOC_ACCEPT==event){
        u8 val = 0;
        s8 ret = 0;
        sockaddr_struct clientAddr={0};
        s8 newsocket = eat_soc_accept(s,&clientAddr);
        if (newsocket < 0){
            my_printf("eat_soc_accept return error :%d",newsocket);
        }
        else{
            sprintf(buffer,"client accept:%s,%d:%d:%d:%d\r\n",clientAddr.addr[0],clientAddr.addr[1],clientAddr.addr[2],clientAddr.addr[3]);
        }

        val = TRUE;
        ret = eat_soc_setsockopt(socket_id, SOC_NODELAY, &val, sizeof(val));
        if (ret != SOC_SUCCESS)
            my_printf("eat_soc_setsockopt SOC_NODELAY return error :%d",ret);

    }

    my_printf("soc_notify_cb");

}

int transport_sendPacketBuffer(int sock, unsigned char* buf, int buflen)
{
	int rc = 0;
	rc = eat_soc_send(sock, buf, buflen);
	return rc;
}

int transport_getdata(unsigned char* buf, int count)
{
	int rc = eat_soc_recv(socket_id, buf, count);
    if(rc == SOC_WOULDBLOCK){
        my_printf("eat_soc_recv no data available");
    }
    else if(rc > 0) {
        //my_printf("eat_soc_recv rc:%d, data:%s", rc, buf);
    }
    else{
        my_printf("eat_soc_recv return error:%d",rc);
    }
	//printf("received %d bytes count %d\n", rc, (int)count);
	return rc;
}

void mqtt_ack_set(int ack_type)
{
    eat_timer_start(EAT_TIMER_3, 3000); //打开定时器，在定时器到期后检测对应的ack是否清零
    mqtt_ack_flag |= 1<<ack_type;
}

void mqtt_ack_clear(int ack_type)
{
    mqtt_ack_flag &= ~(1<<ack_type);
}

eat_bool mqtt_connect()
{
    int ret = 0, rc = 0;
    MQTTPacket_connectData mqtt_data = MQTTPacket_connectData_initializer;

    my_printf("MQTT conection init begin!");

    mqtt_data.clientID.cstring = deviceID;
    mqtt_data.keepAliveInterval = 120;
    mqtt_data.cleansession = 1;
    mqtt_data.username.cstring = deviceID;
    mqtt_data.password.cstring = "testpassword";
    //for will message
    mqtt_data.willFlag = 1;
    sprintf(topic_group, "clients/%s/state", deviceID);
    my_printf("willtopic = %s\r\n", topic_group);
    mqtt_data.will.topicName.cstring = topic_group;
    mqtt_data.will.message.cstring = "0";
    mqtt_data.will.qos = 1;
    mqtt_data.will.retained = 1;

    memset(mqtt_send_buf, 0, MQTT_BUF_SIZE);
    len = MQTTSerialize_connect(mqtt_send_buf, MQTT_BUF_SIZE, &mqtt_data);  //这句话开始MQTT的连接，但是不直接和发送函数相连，而是存到一个buf里面，再从buf里面发送
    ret = transport_sendPacketBuffer(socket_id, mqtt_send_buf, len);
    if (ret < 0)
    {
        my_printf("eat_soc_send return error :%d",ret);
        return EAT_FALSE;
    }
    else
        my_printf("eat_soc_send success :%d",ret);

    mqtt_ack_set(CONNACK);
    
    return EAT_TRUE;
}

eat_bool mqtt_subscribe()
{
    my_printf("mqtt_subscribe is begin");
    return EAT_TRUE;
}

eat_bool mqtt_send_handle(eat_msg_t msg)
{
    switch(msg.mqtt_status)
    {
        case CONNECT:
            mqtt_connect();
            break;
        case SUBSCRIBE:
            mqtt_subscribe();
    }
    return EAT_TRUE;
}

eat_bool mqtt_connack()
{
    unsigned char sessionPresent, connack_rc;

    if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, mqtt_receive_buf, MQTT_BUF_SIZE) != 1 || connack_rc != 0)
    {
        my_printf("MQTT CONNACK1 FAILED!\r\n");
        return EAT_FALSE;
    }
    else
    {
        my_printf("MQTT CONNACK OK!\r\n");
    }
    mqtt_ack_clear(CONNACK);
    my_msg_send(EAT_USER_0, EAT_USER_0, SOC_WRITE, SUBSCRIBE);
    return EAT_TRUE;
}

eat_bool mqtt_receive_handle(eat_msg_t msg)
{
    switch(msg.mqtt_status)
    {
        case CONNACK:
            mqtt_connack();
            break;
    }
    return EAT_TRUE;
}

void MQTT_Initial()
{
    int ret = 0, rc = 0;
    MQTTPacket_connectData mqtt_data = MQTTPacket_connectData_initializer;

    my_printf("MQTT conection init begin!");

    mqtt_data.clientID.cstring = deviceID;
    mqtt_data.keepAliveInterval = 120;
    mqtt_data.cleansession = 1;
    mqtt_data.username.cstring = deviceID;
    mqtt_data.password.cstring = "testpassword";
    //for will message
    mqtt_data.willFlag = 1;
    sprintf(topic_group, "clients/%s/state", deviceID);
    my_printf("willtopic = %s\r\n", topic_group);
    mqtt_data.will.topicName.cstring = topic_group;
    mqtt_data.will.message.cstring = "0";
    mqtt_data.will.qos = 1;
    mqtt_data.will.retained = 1;

    memset(mqtt_send_buf, 0, MQTT_BUF_SIZE);
    len = MQTTSerialize_connect(mqtt_send_buf, MQTT_BUF_SIZE, &mqtt_data);  //这句话开始MQTT的连接，但是不直接和发送函数相连，而是存到一个buf里面，再从buf里面发送
    ret = transport_sendPacketBuffer(socket_id, mqtt_send_buf, len);
    if (ret < 0)
    {
        my_printf("eat_soc_send return error :%d",ret);
        return;
    }
    else
        my_printf("eat_soc_send success :%d",ret);

    eat_sleep(2000);

    memset(mqtt_send_buf, 0, MQTT_BUF_SIZE);
    rc = MQTTPacket_read(mqtt_send_buf, MQTT_BUF_SIZE, transport_getdata);
    my_printf("rc = %d\r\n", rc);
    if ( rc == CONNACK)   //这里把获取数据的指针传了进去！！！
    {
        unsigned char sessionPresent, connack_rc;

        if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, mqtt_send_buf, MQTT_BUF_SIZE) != 1 || connack_rc != 0)
        {
            my_printf("MQTT CONNACK1 FAILED!\r\n");
            return;
        }
        else
        {
            my_printf("MQTT CONNACK OK!\r\n");
        }
    }
    else
    {
        //failed ???
        my_printf("MQTT CONNACK2 FAILED!\r\n");
        return;
    }
}

int Public_Open(int time)
{
    int ret = 0, rc = 0;
    int i = 0;
    unsigned char dup = 0;
    int qos = 1;
    unsigned char retain = 1;
    unsigned short packedid = 1;	//PUBLISH（QoS 大于 0）控制报文 必须包含一个非零的 16 位报文标识符（Packet Identifier）

    for(i = 0; i < time; i++)
    {
        sprintf(topic_group, "clients/%s/state", deviceID);
        my_printf("opentopic = %s\r\n", topic_group);
        topicString.cstring = topic_group;
        sprintf(payload, "1");
        //strcpy(payload, http_buf);
        payloadlen = strlen(payload);
        memset(mqtt_send_buf, 0, MQTT_BUF_SIZE);
        len = MQTTSerialize_publish(mqtt_send_buf, MQTT_BUF_SIZE, dup, qos, retain, packedid, topicString, (unsigned char*)payload, payloadlen);
        ret = transport_sendPacketBuffer(socket_id, mqtt_send_buf, len);
        if (ret < 0)
        {
            my_printf("eat_soc_send return error :%d",ret);
            return;
        }
        else
            my_printf("eat_soc_send success :%d",ret);

        eat_sleep(2000);

        memset(mqtt_send_buf, 0, MQTT_BUF_SIZE);
        rc = MQTTPacket_read(mqtt_send_buf, MQTT_BUF_SIZE, transport_getdata);
        my_printf("rc = %d\r\n", rc);
        if(rc == PUBACK) return 1;
        else dup = 1;	//如果 DUP 标志被设置为 1，表示这可能是一个早前报文请求的重发。
    }

    return 0;
}

void MQTT_Sub0Pub1()
{
    int ret = 0, rc = 0;
    int msgid = 1;
    int req_qos = 0;

    //订阅主题
    sprintf(topic_group, "SHAir/%s/get", deviceID);
    my_printf("subtopic = %s\r\n", topic_group);
    topicString.cstring = topic_group;
    memset(mqtt_send_buf, 0, MQTT_BUF_SIZE);
    len = MQTTSerialize_subscribe(mqtt_send_buf, MQTT_BUF_SIZE, 0, msgid, 1, &topicString, &req_qos);
    //所有这些都不是直接发送，而是通过先获取buffer，我们再手动发送出去
    ret = transport_sendPacketBuffer(socket_id, mqtt_send_buf, len);
    if (ret < 0)
    {
        my_printf("eat_soc_send return error :%d",ret);
        return;
    }
    else
        my_printf("eat_soc_send success :%d",ret);

    eat_sleep(2000);

    memset(mqtt_send_buf, 0, MQTT_BUF_SIZE);
    rc = MQTTPacket_read(mqtt_send_buf, MQTT_BUF_SIZE, transport_getdata);
    my_printf("rc = %d\r\n", rc);
    if (rc == SUBACK)  /* wait for suback */ //会在这里阻塞？
    {
        unsigned short submsgid;
        int subcount;
        int granted_qos;

        rc = MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, mqtt_send_buf, MQTT_BUF_SIZE);
        if (granted_qos != 0)
        {
            //wrong
            my_printf("MQTT SUBACK1 FAILED!\r\n");
            return;
        }
        else
        {
            my_printf("MQTT SUBACK OK!\r\n");
        }
    }
    else
    {
        my_printf("MQTT SUBACK2 FAILED!\r\n");
        return;
    }

    eat_sleep(5000);
        
    //接收retain数据：expiresAt和childLock
    memset(mqtt_send_buf, 0, MQTT_BUF_SIZE);
    rc = MQTTPacket_read(mqtt_send_buf, MQTT_BUF_SIZE, transport_getdata);
    my_printf("rc = %d\r\n", rc);
    if (rc == PUBLISH)
    {
        unsigned char dup;
        int qos;
        unsigned char retained;
        unsigned short msgid;
        int payloadlen_in;
        unsigned char* payload_in;
        MQTTString receivedTopic;

        my_printf("recive retain publish\r\n");
        rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
                                    &payload_in, &payloadlen_in, mqtt_send_buf, MQTT_BUF_SIZE);
        //handle "payload_in" as data from the server
        my_printf("retained = %d, msgid = %d, receivedTopic = %s", retained, msgid, receivedTopic.cstring);
        my_printf("payloadlen_in = %d, payload_in = %s", payloadlen_in, payload_in);

        //recv_mqtt(payload_in, payloadlen_in, payload, &payloadlen);
    }

    //发布开机提示
    if(!Public_Open(5))
    {
        my_printf("PUBLIC OPEN ERROR");
        return;
    }
    my_printf("PUBLIC OPEN OK!\r\n");

    //发布主题
    sprintf(topic_group, "SHAir/%s/update", deviceID);
    my_printf("pubtopic = %s", topic_group);
    topicString.cstring = topic_group;
}