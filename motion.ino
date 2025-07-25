#include "comm.h"
#include <Ticker.h>
#include <functional>  // 用于 std::bind

void updateBin(char* para);
static void CmdGetHandler();
static void CmdPowerCtrlHandlerOn(void);
static void CmdPowerCtrlHandlerOff(void);
static void CmdHelpHandler();
static void CmdPowerCtrlHandlerForceOff(void);
static void CmdPowerCtrlHandlerDirIn(char *para);
static void CmdPowerCtrlHandlerDirOut(char *para);

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
    {"in",          CUSTOM_CMD_DIR_IN, CmdPowerCtrlHandlerDirIn},
    {"out",          CUSTOM_CMD_DIR_OUT, CmdPowerCtrlHandlerDirOut},
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

POWER_STATE_IO_t g_PowerStateOfAPP = POWER_STATE_IO_UNKONWN;
static void CmdPowerCtrlHandlerOn(char* para)
{
    if (para == NULL){
        CmdPowerCtrlHandlerDirIn(NULL); // 本地按键
    }
    else{ // 对服务器下发的命令，进行解析。例如：#3#0，#3#1.    on#3#0
        uint8_t cmdType;
        uint8_t fanDirOut;
        if (sscanf(para + 1, "%d#%d", &cmdType, &fanDirOut) == 2) {
            if (cmdType != 3) {
                printf("解析失败, cmdType is %d ,!= 3\n", cmdType);
                return;
            }
            if (fanDirOut == ENABLE) {
                CmdPowerCtrlHandlerDirOut(NULL);
            }else{
                CmdPowerCtrlHandlerDirIn(NULL);
            }
        } else {
            printf("解析失败\n");
        }
    }
}

static void CmdPowerCtrlHandlerOff(char* para)
{
    Serial.println("power off");
    GPIO_setPinStatus(PinFANEnable, DISABLE);
    g_PowerStateOfAPP = (POWER_STATE_IO_t)DISABLE;
    LastSyncTickReset(); // 等一会儿再同步
}
Ticker timer_DirIn, timer_DirOut; // 定义两个定时器

static void timerCallback_DirIn(){
    GPIO_setPinStatus(PinFANDirctionOut, ENABLE);
    delay(500);    // 继电器响应时间
    GPIO_setPinStatus(PinFANEnable, ENABLE);
    g_PowerStateOfAPP = (POWER_STATE_IO_t)ENABLE;
    LastSyncTickReset(); // 等一会儿再同步
    Serial.println("power on, dir in");
    timer_DirIn.detach(); 
    
    updateState(UPDATE_PRESSED);
}
static void timerCallback_DirOut(){
    GPIO_setPinStatus(PinFANDirctionOut, DISABLE);
    delay(500);    // 继电器响应时间
    GPIO_setPinStatus(PinFANEnable, ENABLE);
    g_PowerStateOfAPP = (POWER_STATE_IO_t)ENABLE;
    LastSyncTickReset(); // 等一会儿再同步
    
    Serial.println("power on, dir out");
    timer_DirOut.detach();
    updateState(UPDATE_PRESSED);
}

static void CmdPowerCtrlHandlerDirIn(char* para)
{
    CmdPowerCtrlHandlerOff(para);

    timer_DirIn.once(POWER_OFF_STABLE, std::bind(timerCallback_DirIn));
}

static void CmdPowerCtrlHandlerDirOut(char* para)
{
    CmdPowerCtrlHandlerOff(para);

    timer_DirIn.once(POWER_OFF_STABLE, std::bind(timerCallback_DirOut)); 
}
static void CmdGetHandler(char* para)
{
    updateState(UPDATE_TYPE_SERVICE_READ);
}
static void CmdHelpHandler(char* para)
{
    UpdateStateToServer(GetAllCmdList());
}
/*订阅的主题有消息发布时的回调函数*/
void MsgCallBack(char *topic, byte *payload, unsigned int payloadLen)
{
    if (topic == nullptr || payload == nullptr || payloadLen == 0){
        return;
    }
    Serial.printf("Rev string: topic = %s\r\n", topic);
    Serial.printf("\tmsg : ");
    for (int i = 0; i < payloadLen; i++) {
        Serial.print((char)payload[i]); // 打印消息
    }
    Serial.println("");
    delay(10);

    // payload 和 alias 相比较，如果匹配，则执行对应的cmdHandler
    for (int i = 0; i < sizeof(cmdList) / sizeof(cmdList[0]); i++) {
        uint32_t cmdLen = strlen(cmdList[i].cmd);
        if (payloadLen >= cmdLen && strncmp((char *)payload, cmdList[i].alias, cmdLen) == 0) {
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
            MQTTClient.subscribe(TOPIC_CTRL); // 订阅主题
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
// void MonitorPCPower()
// {
//     static POWER_STATE_IO_t powerIOStateLast = POWER_STATE_IO_UNKONWN;
//     int isChanged = 0;
//     bool powerIOState = GPIO_isPinActive(PinFANEnable);
//     if ((POWER_STATE_IO_t)powerIOState != powerIOStateLast) {
//         delay(10);
//         powerIOState = GPIO_isPinActive(PinFANEnable);
//         if (powerIOState != powerIOStateLast) {
//             updateState(UPDATE_TYPE_STATE_CHANGED);
//             powerIOStateLast = (POWER_STATE_IO_t)powerIOState;
//         }
//     }
// }

/// @brief app上按下开关按键后，真实的电源状态，可能并没有改变，比如控制引脚断开。所以需要app 和实际的检测 状态 同步
// void MonitorAppSync()
// {
//     bool nowState = GPIO_isPinActive(PinFANEnable);
//     if (g_PowerStateOfAPP == (POWER_STATE_IO_t)nowState) { // 真实的按键状态和app上同步过来的状态一致，不需要更新
//         return;
//     }
//     Serial.printf("app = %s, io = %s\r\n", g_PowerStateOfAPP ? "ON" : "OFF", nowState ? "ON" : "OFF");

//     if (updateState(UPDATE_TYPE_UNMATCH_SYNC)){
//         g_PowerStateOfAPP = (POWER_STATE_IO_t)nowState;
//     }
// }
bool UpdateStateToServer(String cmd)
{
    char topic[64];
    if (!wifiClient.connected()) {
        return false;
    }
    // 推送消息时：主题名后加/set推送消息，表示向所有订阅这个主题的设备们推送消息，假如推送者自己也订阅了这个主题，
    // 消息不会被推送给它自己，以防止自己推送的消息被自己接收。 例如向主题 light002推送数据，可为 light002/set。
    snprintf(topic, sizeof(topic), "%s/set", TOPIC_CTRL);
    MQTTClient.publish(topic, cmd.c_str());
    return true;
}
/// @brief 发送消息到服务器
/// @param updateMode 周期性更新还是状态改变更新，周期性更新时，用上次的状态，状态改变时，用当前状态
/// @param nowState 只有在状态改变时才需要传入当前状态，周期性更新时，忽略此参数
bool updateState(UPDATE_TYPE_t updateMode)
{
    if (MQTTClient.state() != MQTT_CONNECTED) {
        return false;
    }
    if (updateMode == UPDATE_TYPE_FORCE) {
        Serial.printf("-----force\r\n");
    } else if (updateMode == UPDATE_TYPE_SERVICE_READ){
        Serial.printf("-----service read\r\n");
    } else if (updateMode == UPDATE_PRESSED){
        Serial.printf("-----key pressed\r\n");
    } else if (updateMode == UPDATE_TYPE_UNMATCH_SYNC){
        Serial.printf("-----app with power state un match, then sync\r\n");
    }
    LastUpdateTickReset();
    LastSyncTickReset(); // 刚刚同步过，等一会儿再同步
    
    
    bool fanDirOut = GPIO_isPinActive(PinFANDirctionOut);
    bool fanEnable = GPIO_isPinActive(PinFANEnable);
    if (fanEnable) {
        Serial.printf("Update State To Server , power on\r\n");
        if (fanDirOut){
            return UpdateStateToServer(String(CUSTOM_CMD_ON)+"3#0");
        }else{
            return UpdateStateToServer(String(CUSTOM_CMD_ON)+"3#1");
        }
    } else {
        Serial.printf("Update State To Server , power off\r\n");
        return UpdateStateToServer(CUSTOM_CMD_OFF);
    }
}
typedef struct {
    bool isIOActiveLast; //  上次IO状态，是否激活

    unsigned int tickPressed; // 按下的时间
    unsigned int tickRelesed; // 释放的时间

    unsigned int pressedCount; // 按键按下的次数
} Button_ST;

static Button_ST button;
#define debounceDelay  30
#define buttonPressLastTime_ms 3000 // 按键释放后， 超过这个值，才开始真正启作用
int ScanButton()
{
    int res = 0;  // 按键次数
    unsigned int currentTimeTick = millis();

    bool stateIOCurrent = GPIO_isPinActive(PinButton); // 当前状态
    // 消抖逻辑：状态变化时开始计时
    if (stateIOCurrent != button.isIOActiveLast) {
        if (stateIOCurrent == true) {
            button.tickPressed = currentTimeTick;
        }else{
            button.tickRelesed = currentTimeTick;
        }
    }
    // 按键释放
    if (button.isIOActiveLast == true && stateIOCurrent == false) {
        if ((currentTimeTick - button.tickPressed) >= debounceDelay) { // 满足去抖条件
            button.pressedCount++; // 按键按下次数
            return 0; // 只计数，不处理
        }else{
            return 0; // 去抖中
        }
        
        button.isIOActiveLast = stateIOCurrent; // update
    }
    // 如果长时间没有按，则返回按键总次数。
    if ((currentTimeTick - button.tickRelesed) >= buttonPressLastTime_ms) {
        res = button.pressedCount;
        Serial.printf("Button pressed count is %d\n", button.pressedCount);

        button.pressedCount = 0; // 清零
        return res;
    }
    
    return 0;
}

/*
关机状态
	1正
	2反
开机状态
	1关
	2换方向

    当前状态    按下次数    操作
    关机状态    1           开机
    关机状态    2           换方向
    开机状态    1           关机
    开机状态    2           换方向
*/
void monitorButton()
{
    int buttonPressedCount = ScanButton();
    if (buttonPressedCount == 0) {
        return;
    }
    
    bool fanEnable = GPIO_isPinActive(PinFANEnable);
    bool fanDirOut = GPIO_isPinActive(PinFANDirctionOut);
    if (buttonPressedCount == ButtonAction_POWER) {
        if (fanEnable == ENABLE) {
            CmdPowerCtrlHandlerOff(NULL);
            updateState(UPDATE_PRESSED);
        }else{
            CmdPowerCtrlHandlerOn(NULL);
        }
    }else if (buttonPressedCount == ButtonAction_DIR) {
        if (fanDirOut == ENABLE) {
            CmdPowerCtrlHandlerDirIn(NULL);
        }else{
            CmdPowerCtrlHandlerDirOut(NULL);
        }
    }
}