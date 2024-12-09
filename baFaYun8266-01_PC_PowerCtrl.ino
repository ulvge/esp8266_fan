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


const int powerCheckPin = 0;  // 默认是高，低有效。input ispower on now
const int outputPin = 2;  // output 输出开关机、重启的脉冲

//**************************************************//
// tcp客户端相关初始化，默认即可
WiFiClient wifiClient;
PubSubClient MQTTClient(wifiClient);
// 相关函数初始化
// 连接WIFI
bool doWiFiConnect();
bool MQTT_reconnect();


void printHardwareInfo()
{
    unsigned int flashRealSize = ESP.getFlashChipRealSize();
    unsigned int flashSize = ESP.getFlashChipSize();
    unsigned int flashSpeed = ESP.getFlashChipSpeed();
    unsigned int sketchSize = ESP.getSketchSize();
    unsigned int freeSketchSpace = ESP.getFreeSketchSpace();

    Serial.printf("Flash real size: %u bytes (%.2f KB, %.2f MB)\n", flashRealSize, flashRealSize / 1024.0, flashRealSize / (1024.0 * 1024.0));
    Serial.printf("Flash size: %u bytes (%.2f KB, %.2f MB)\n", flashSize, flashSize / 1024.0, flashSize / (1024.0 * 1024.0));
    Serial.printf("Flash speed: %u Hz\n", flashSpeed);
    Serial.printf("Sketch size: %u bytes (%.2f KB, %.2f MB)\n", sketchSize, sketchSize / 1024.0, sketchSize / (1024.0 * 1024.0));
    Serial.printf("Free sketch space: %u bytes (%.2f KB, %.2f MB)\n", freeSketchSpace, freeSketchSpace / 1024.0, freeSketchSpace / (1024.0 * 1024.0));
    Serial.printf("default CPU Frequency: %d MHz\r\n", ESP.getCpuFreqMHz());
    Serial.printf("OTA url: %s\r\n", g_updateURL.c_str());

    // 4Mbit = 0.5MByte  fre=10M
    // Flash real size: 524288 bytes (512.00 KB, 0.50 MB)
    // Flash size: 524288 bytes (512.00 KB, 0.50 MB)
    // Flash speed: 40000000 Hz 
    // Sketch size: 308928 bytes (301.69 KB, 0.29 MB)
    // Free sketch space: 1785856 bytes (1744.00 KB, 1.70 MB)
}
// 初始化，相当于main 函数
void setup()
{
    delay(200);
    Serial.begin(115200);
    delay(500);

    pinMode(powerCheckPin, INPUT);
    digitalWrite(outputPin, HIGH);
    pinMode(outputPin, OUTPUT);
    digitalWrite(outputPin, HIGH);

    Serial.println("");
    Serial.println("");
    printHardwareInfo();
    Serial.println("Beginning...");

    MQTTClient.setServer(server_ip, server_port);
    MQTTClient.setCallback(MsgCallBack);
}

// 循环
void loop()
{
    static unsigned long lastUpdateTick;
    unsigned long currentTick = millis();
    bool isFirstConnect = false;

    isFirstConnect = doWiFiConnect();
    if (MQTTClient.state() != MQTT_CONNECTED){
        isFirstConnect |= MQTT_reconnect();
    }else{
        MQTTClient.loop();
    }
    monitorButton();

    if (isFirstConnect || (currentTick - lastUpdateTick >= 60 * 1000)) {
        lastUpdateTick = currentTick;
        updateState(UPDATE_PERIOD, -1);
    }
    keepLive();
    delay(10);
}