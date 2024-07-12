/*
 * 智能语言控制控制，支持天猫、小爱、小度、google Assistent同时控制
 * Time:20211127
 * Author: 2345VOR
 * 项目实例：发送on、off的指令开关灯
 * 参考文献：https://bbs.bemfa.com/84/last
 */
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include "comm.h"
#include <PubSubClient.h>

const int LED_Pin = 12;                          // 单片机LED引脚值，D2是NodeMcu引脚命名方式，其他esp8266型号将D2改为自己的引脚

const int buttonPin = 13;  // input
const int ledPinRed = 16;  // output
const int ledPinBlue = 12; // output
const int outputPin = 14;  // output

//**************************************************//
// 最大字节数
#define MAX_PACKETSIZE 512
// 设置心跳值30s
#define KEEPALIVEATIME 30 * 1000
// tcp客户端相关初始化，默认即可
WiFiClient TCPclient;
PubSubClient publishClient(TCPclient);
String TcpClient_Buff = ""; // 初始化字符串，用于接收服务器发来的数据
unsigned int TcpClient_BuffIndex = 0;
unsigned long TcpClient_preTick = 0;
unsigned long preHeartTick = 0;    // 心跳
unsigned long preTCPStartTick = 0; // 连接
bool preTCPConnected = false;
// 相关函数初始化
// 连接WIFI
void doWiFiTick();

// TCP初始化连接
void doTCPClientTick();
void startTCPClient();
void sendtoTCPServer(String p);


// 初始化，相当于main 函数
void setup()
{
    delay(200);
    Serial.begin(115200);
    delay(500);

    pinMode(buttonPin, INPUT);

    pinMode(ledPinRed, OUTPUT);
    pinMode(ledPinBlue, OUTPUT);
    setLed(LED_MODE_BLINK);

    pinMode(outputPin, OUTPUT);
    digitalWrite(outputPin, LOW);

    pinMode(LED_Pin, OUTPUT);
    digitalWrite(LED_Pin, HIGH);
    Serial.println("");
    Serial.println("");
    Serial.println("Beginning...");
    
    publishClient.setServer(server_ip, server_port);
}

// 循环
void loop()
{
    doWiFiTick();
    doTCPClientTick();
    monitorButton();

    updateState(false);
    delay(10);
}