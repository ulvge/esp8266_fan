#include "comm.h"

void updateBin(char* para);
static void CmdGetHandler();
static void CmdPowerCtrlHandlerOn(void);
static void CmdPowerCtrlHandlerOff(void);
static void CmdHelpHandler();
static void CmdPowerCtrlHandlerForceOff(void);

// 定义Wi-Fi账号和密码
const char *wifiCredentials[][2] = {
    {"FAST_FFF43A", "1234567890"},
    {"5530", "123456780"},
    {"hxzy_guest", "hxzy123123!"}
};

typedef  void (*CmdHandler)(char* para);
typedef struct {
    const char*              alias; //  人为输入的字符串,可能存在空格
    const char*              cmd;    // 标准命令
    CmdHandler          cmdHandler;
} CmdHandlerList;
const CmdHandlerList cmdList[] = {
    {"update",      CUSTOM_CMD_UPDATE_FIRMWARE, updateBin},
    {"get",         CUSTOM_CMD_GET, CmdGetHandler},
    {"on",          CUSTOM_CMD_ON, CmdPowerCtrlHandlerOn},
    {"off",         CUSTOM_CMD_OFF, CmdPowerCtrlHandlerOff},
    {"forceoff",    CUSTOM_CMD_FORCE_POWEROFF, CmdPowerCtrlHandlerForceOff},
    {"force_off",    CUSTOM_CMD_FORCE_POWEROFF, CmdPowerCtrlHandlerForceOff},
    {"force off",    CUSTOM_CMD_FORCE_POWEROFF, CmdPowerCtrlHandlerForceOff},
    {"db",        CUSTOM_CMD_RF_DB, CmdSetPowerHandler},
    {"help",        CUSTOM_CMD_HELP, CmdHelpHandler},
    {"?",           CUSTOM_CMD_HELP, CmdHelpHandler}
};
const String GetAllCmdList(void)
{
    String cmdListStr = "#";
    for (int i = 0; i < sizeof(cmdList) / sizeof(cmdList[0]); i++) {
        cmdListStr += cmdList[i].cmd;
        cmdListStr += "#";
    }
    return cmdListStr;
}
const int wifiCount = sizeof(wifiCredentials) / sizeof(wifiCredentials[0]);

/// @brief 短按开关机
/// @param  
static void CmdPowerCtrlHandler(char* para)
{
    Serial.println("power on/off prepare....");
    digitalWrite(outputPin, LOW);
    delay(POWER_ON_OFF_DURATION);
    digitalWrite(outputPin, HIGH);
    Serial.println("power on/off finished ");
}
POWER_STATE_IO_t g_PowerStateOfAPP = POWER_STATE_IO_UNKONWN;
static void CmdPowerCtrlHandlerOn(char* para)
{
    CmdPowerCtrlHandler(para);
    g_PowerStateOfAPP = POWER_STATE_IO_ON;
    LastSyncTickReset(); // 等一会儿再同步
}

static void CmdPowerCtrlHandlerOff(char* para)
{
    CmdPowerCtrlHandler(para);
    g_PowerStateOfAPP = POWER_STATE_IO_OFF;
    LastSyncTickReset(); // 等一会儿再同步
}
/// @brief 强制关机
/// @param  
static void CmdPowerCtrlHandlerForceOff(char* para)
{
    Serial.println("force power off prepare....");
    digitalWrite(outputPin, LOW);
    delay(POWER_OFF_FROCE_DURATION);
    digitalWrite(outputPin, HIGH);
    Serial.println("force power off finished");
}
static void CmdGetHandler(char* para)
{
    updateState(UPDATE_TYPE_SERVICE_READ, (POWER_STATE_IO_t)digitalRead(powerCheckPin));
}
static void CmdHelpHandler(char* para)
{
    UpdateStateToServer(GetAllCmdList());
}
static void CmdSetPowerHandler(char* para)
{
    uint32_t power = atoi(para + 1);
    if (power > 20) {
        Serial.printf("WIFI POWER DB config para [%d] error, current: [%d]\r\n", power, g_powerDB);
        return;
    }
    g_powerDB = power;
    WiFi.setOutputPower(g_powerDB);
    Serial.printf("WIFI POWER DB config sucess: [%d], max 20\r\n", g_powerDB);
}
/*订阅的主题有消息发布时的回调函数*/
void MsgCallBack(char *topic, byte *payload, unsigned int length)
{
    if (topic == nullptr || payload == nullptr || length == 0){
        return;
    }
    Serial.printf("Rev string: topic = %s\r\n", topic);
    Serial.printf("\tmsg : ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]); // 打印消息
    }
    Serial.println("");
    delay(10);

    // payload 和 alias 相比较，如果匹配，则执行对应的cmdHandler
    for (int i = 0; i < sizeof(cmdList) / sizeof(cmdList[0]); i++) {
        uint32_t cmdLen = strlen(cmdList[i].cmd);
        if (length >= cmdLen && strncmp((char *)payload, cmdList[i].alias, cmdLen) == 0) {
            cmdList[i].cmdHandler((char *)&payload[cmdLen]);
        }
    }
}

/**************************************************************************
                                 WIFI
***************************************************************************/

// 0:未连上
// 1:已连接
int setup_wifi()
{
    static uint32_t i = -1;
    static uint32_t retry = 0; 
    static uint32_t errCount;

    if (WiFi.status() == WL_CONNECTED) {
        return 1;
    }
    // 10ms 进一次，有 wifiCount 组，当前组，重试500, 5s次。
    if (retry > 500) {
        retry = 0;
        if (errCount > 5* 12) { // wifi connect timeout in 5min, reset
            Serial.println("Beginning...");
            ESP.restart();
        }
    }
    if (retry == 0) {
        i++;
        if (i >= wifiCount) {
            i = 0;
        }
        if (WiFi.status() != WL_CONNECTED) {
            WiFi.setOutputPower(g_powerDB);
            WiFi.begin(wifiCredentials[i][0], wifiCredentials[i][1]);
            Serial.printf("Connecting to %s\n", wifiCredentials[i][0]);
        } else {
            return 1;
        }
    } else {
        // 等待WiFi连接
        if (WiFi.status() != WL_CONNECTED) {
            if (retry % 100 == 0) {
                Serial.print(".");
            }
        } else {
            return 1;
        }
    }
    retry++;
    return 0;
}
/*重连MQTT函数*/
bool MQTT_reconnect()
{
    if (WiFi.status() != WL_CONNECTED) {
        return false; // waitting for connect wifi first
    }
    if (MQTTClient.state() != MQTT_CONNECTED) {
        Serial.println("Attempting MQTT connection...");
        // 尝试去连接
        if (MQTTClient.connect(UID)) {
            Serial.println("MQTT connected");   // 连接成功
            MQTTClient.subscribe(TOPIC_PCPowerCtrl); // 订阅主题
            return true;
        } else {
            Serial.print("failed, rc="); // 连接失败，输出状态，五秒后重试
            Serial.print(MQTTClient.state());
        }
    }
    return false;
}
/*
  WiFiTick
  检查是否需要初始化WiFi
  检查WiFi是否连接上，若连接成功启动TCP Client
  控制指示灯
*/
bool doWiFiConnect()
{
    static bool startSTAFlag = false;
    static bool taskStarted = false;

    if (!startSTAFlag) {
        startSTAFlag = true;
        WiFi.disconnect();
        WiFi.mode(WIFI_STA);
    }

    if (setup_wifi() == 1) {
        // 每次连接成功之后，设置一次
        if (!taskStarted) {
            taskStarted = true;

            // wifi连接成功后输出成功信息
            Serial.println("");
            Serial.println("WiFi Connected!"); // 显示wifi连接成功
            Serial.println(WiFi.localIP());    // 返回wifi分配的IP
            Serial.println(WiFi.macAddress()); // 返回设备的MAC地址
            Serial.println("");
            return true;
        }
    } else {
        taskStarted = false;
    }
    return false;
}

// https://cloud.bemfa.com/docs/src/tcp.html#%E5%AD%97%E6%AE%B5%E8%AF%B4%E6%98%8E
/// @brief 发送任意数据 为心跳消息,包括上述指令也算是心跳，但要以回车换行结尾。
/// 心跳消息是告诉服务器设备还在线，建议60秒发送一次，超过65秒未发送心跳会掉线。
/// @param
void keepLive(void)
{
    static unsigned long lastSendLiveTick;
    unsigned long currentTick;
    if (MQTTClient.state() != MQTT_CONNECTED) {
        return;
    }
    currentTick = millis();
    if (currentTick - lastSendLiveTick >= KEEPALIVEATIME) {
        Serial.println("--Keep alive:");
        MQTTClient.publish(TOPIC_KEEP_ALIVE, "anything\r\n"); // 发送心跳
        lastSendLiveTick = currentTick;
    }
}
const unsigned int debounceDelay = 30;
unsigned int lastDebounceTime;
void MonitorPCPower()
{
    static POWER_STATE_IO_t powerIOStateLast = POWER_STATE_IO_UNKONWN;
    int isChanged = 0;
    POWER_STATE_IO_t powerIOState = (POWER_STATE_IO_t)digitalRead(powerCheckPin);
    if (powerIOState != powerIOStateLast) {
        delay(10);
        powerIOState = (POWER_STATE_IO_t)digitalRead(powerCheckPin);
        if (powerIOState != powerIOStateLast) {
            updateState(UPDATE_TYPE_STATE_CHANGED, powerIOState);
            powerIOStateLast = powerIOState;
        }
    }
}
/// @brief app上按下开关按键后，真实的电源状态，可能并没有改变，比如控制引脚断开。所以需要app 和实际的检测 状态 同步
void MonitorAppSync()
{
    POWER_STATE_IO_t nowState = (POWER_STATE_IO_t)digitalRead(powerCheckPin);
    if (g_PowerStateOfAPP == nowState) { // 真实的按键状态和app上同步过来的状态一致，不需要更新
        return;
    }
    Serial.printf("app = %s, io = %s\r\n", 
    g_PowerStateOfAPP == POWER_STATE_IO_ON ? "ON" : "OFF", nowState == (POWER_STATE_IO_t)powerCheckPin_active ? "ON" : "OFF");

    if (updateState(UPDATE_TYPE_UNMATCH_SYNC, nowState)){
        g_PowerStateOfAPP = nowState;
    }
}
bool UpdateStateToServer(String cmd)
{
    char topic[64];
    if (!wifiClient.connected()) {
        return false;
    }
    // 推送消息时：主题名后加/set推送消息，表示向所有订阅这个主题的设备们推送消息，假如推送者自己也订阅了这个主题，
    // 消息不会被推送给它自己，以防止自己推送的消息被自己接收。 例如向主题 light002推送数据，可为 light002/set。
    snprintf(topic, sizeof(topic), "%s/set", TOPIC_PCPowerCtrl);
    MQTTClient.publish(topic, cmd.c_str());
    return true;
}
/// @brief 发送消息到服务器
/// @param updateMode 周期性更新还是状态改变更新，周期性更新时，用上次的状态，状态改变时，用当前状态
/// @param nowState 只有在状态改变时才需要传入当前状态，周期性更新时，忽略此参数
bool updateState(UPDATE_TYPE_t updateMode, POWER_STATE_IO_t nowState)
{
    if (MQTTClient.state() != MQTT_CONNECTED) {
        return false;
    }
    if (updateMode == UPDATE_TYPE_FORCE) {
        Serial.printf("-----force\r\n");
    } else if (updateMode == UPDATE_TYPE_SERVICE_READ){
        Serial.printf("-----service read\r\n");
    } else if (updateMode == UPDATE_TYPE_STATE_CHANGED){
        Serial.printf("-----state changed\r\n");
        g_PowerStateOfAPP = nowState;
    } else if (updateMode == UPDATE_TYPE_UNMATCH_SYNC){
        Serial.printf("-----app with power state un match, then sync\r\n");
    }
    LastUpdateTickReset();
    LastSyncTickReset(); // 刚刚同步过，等一会儿再同步
    if (nowState == powerCheckPin_active) {
        Serial.printf("Update State To Server io = %d, power on\r\n", nowState);
        return UpdateStateToServer(CUSTOM_CMD_ON);
    } else {
        Serial.printf("Update State To Server io = %d, power off\r\n", nowState);
        return UpdateStateToServer(CUSTOM_CMD_OFF);
    }
}