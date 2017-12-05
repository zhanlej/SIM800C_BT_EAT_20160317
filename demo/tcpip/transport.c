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
extern s8 socket_id;
 
/********************************************************************
 * Local Variables:  STATIC
 ********************************************************************/
char deviceID[20] = "200025";
char topic_group[30];
int len = 0;
unsigned char mqtt_buf[MQTT_BUF_SIZE]; 
MQTTString topicString = MQTTString_initializer;
char payload[MQTT_BUF_SIZE] = "testmessage\n";	//MQTT发布出去的数据
int payloadlen = MQTT_BUF_SIZE;

/********************************************************************
 * External Functions declaration
 ********************************************************************/


/********************************************************************
 * Local Function declaration
 ********************************************************************/

/********************************************************************
 * Local Function
 ********************************************************************/
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

    memset(mqtt_buf, 0, MQTT_BUF_SIZE);
    len = MQTTSerialize_connect(mqtt_buf, MQTT_BUF_SIZE, &mqtt_data);  //这句话开始MQTT的连接，但是不直接和发送函数相连，而是存到一个buf里面，再从buf里面发送
    ret = transport_sendPacketBuffer(socket_id, mqtt_buf, len);
    if (ret < 0)
    {
        my_printf("eat_soc_send return error :%d",ret);
        return;
    }
    else
        my_printf("eat_soc_send success :%d",ret);

    eat_sleep(2000);

    memset(mqtt_buf, 0, MQTT_BUF_SIZE);
    rc = MQTTPacket_read(mqtt_buf, MQTT_BUF_SIZE, transport_getdata);
    my_printf("rc = %d\r\n", rc);
    if ( rc == CONNACK)   //这里把获取数据的指针传了进去！！！
    {
        unsigned char sessionPresent, connack_rc;

        if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, mqtt_buf, MQTT_BUF_SIZE) != 1 || connack_rc != 0)
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
        memset(mqtt_buf, 0, MQTT_BUF_SIZE);
        len = MQTTSerialize_publish(mqtt_buf, MQTT_BUF_SIZE, dup, qos, retain, packedid, topicString, (unsigned char*)payload, payloadlen);
        ret = transport_sendPacketBuffer(socket_id, mqtt_buf, len);
        if (ret < 0)
        {
            my_printf("eat_soc_send return error :%d",ret);
            return;
        }
        else
            my_printf("eat_soc_send success :%d",ret);

        eat_sleep(2000);

        memset(mqtt_buf, 0, MQTT_BUF_SIZE);
        rc = MQTTPacket_read(mqtt_buf, MQTT_BUF_SIZE, transport_getdata);
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
    memset(mqtt_buf, 0, MQTT_BUF_SIZE);
    len = MQTTSerialize_subscribe(mqtt_buf, MQTT_BUF_SIZE, 0, msgid, 1, &topicString, &req_qos);
    //所有这些都不是直接发送，而是通过先获取buffer，我们再手动发送出去
    ret = transport_sendPacketBuffer(socket_id, mqtt_buf, len);
    if (ret < 0)
    {
        my_printf("eat_soc_send return error :%d",ret);
        return;
    }
    else
        my_printf("eat_soc_send success :%d",ret);

    eat_sleep(2000);

    memset(mqtt_buf, 0, MQTT_BUF_SIZE);
    rc = MQTTPacket_read(mqtt_buf, MQTT_BUF_SIZE, transport_getdata);
    my_printf("rc = %d\r\n", rc);
    if (rc == SUBACK)  /* wait for suback */ //会在这里阻塞？
    {
        unsigned short submsgid;
        int subcount;
        int granted_qos;

        rc = MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, mqtt_buf, MQTT_BUF_SIZE);
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
    memset(mqtt_buf, 0, MQTT_BUF_SIZE);
    rc = MQTTPacket_read(mqtt_buf, MQTT_BUF_SIZE, transport_getdata);
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
                                    &payload_in, &payloadlen_in, mqtt_buf, MQTT_BUF_SIZE);
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