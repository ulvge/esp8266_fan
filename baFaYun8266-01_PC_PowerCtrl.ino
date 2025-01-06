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


const int powerCheckPin = 0;  // 光耦4脚默认上拉为高；当光耦1脚有电导通后，4脚为低。
const int outputPin = 2;  // output 输出开关机、重启的脉冲,接的光耦2脚。默认为高，光耦不导通。光耦导通后，会使光耦4脚 ，即笔记本开关机按键，产生低电平。

//**************************************************//
// tcp客户端相关初始化，默认即可
WiFiClient wifiClient;
PubSubClient MQTTClient(wifiClient);
// 相关函数初始化
// 连接WIFI
bool doWiFiConnect();
bool MQTT_reconnect();

uint32_t g_powerDB = WIFI_POWER_DB_DEFALUT;
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

    Serial.printf("Build Date: %s\n", __DATE__);
    Serial.printf("Build Time: %s\n", __TIME__);
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
    pinMode(powerCheckPin, INPUT);
    digitalWrite(outputPin, HIGH);
    pinMode(outputPin, OUTPUT);
    digitalWrite(outputPin, HIGH);

    Serial.begin(115200);
    Serial.println("");
    Serial.println("");
    delay(3000);
    printHardwareInfo();
    ESP.wdtEnable(WDTO_8S);
    ESP.wdtFeed();
    delay(2000);
    Serial.println("Beginning...");
    MQTTClient.setServer(server_ip, server_port);
    MQTTClient.setCallback(MsgCallBack);
}

volatile unsigned long g_lastUpdateTick;
void LastUpdateTickReset()
{
    g_lastUpdateTick = millis();
}

volatile unsigned long g_lastSyncTick;  // app 和实际的检测 状态 同步
// 当app改变了状态，或者检测到状态变化。重新计算同步时间
void LastSyncTickReset()
{
    g_lastSyncTick = millis();
}

unsigned long AbsSub(unsigned long a, unsigned long b) {
    if (a >= b) {
        return a - b;
    } else {
        return b - a;
    }
}

// 循环
void loop()
{
    static unsigned long lastPrintTick;
    unsigned long currentTick = millis();
    bool isFirstConnect = false;

    doWiFiConnect();

    if (WiFi.status() == WL_CONNECTED) {
        if (MQTTClient.state() != MQTT_CONNECTED){
            isFirstConnect = MQTT_reconnect();
        }
        if (MQTTClient.state() == MQTT_CONNECTED) {
            MQTTClient.loop();
            MonitorPCPower();

            if (AbsSub(currentTick, g_lastSyncTick) >= 1500) {
                MonitorAppSync();
            }

            if (isFirstConnect || (AbsSub(currentTick, g_lastUpdateTick) >= UPDATE_FORCE_PERIOD_S * 1000)) {
                updateState(UPDATE_TYPE_FORCE, (POWER_STATE_IO_t)digitalRead(powerCheckPin));
            }
            keepLive();
        }
    }
    if ((currentTick - lastPrintTick) >= 5 * 1000) {
        lastPrintTick = currentTick;
        Serial.print(".");
    }
    ESP.wdtFeed();
    delay(20);
}