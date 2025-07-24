#ifndef COMM_H
#define COMM_H

#define ARRARY_SIZE(str)    (sizeof(str) / sizeof(str[0]))

typedef enum {DISABLE = 0, ENABLE = !DISABLE} EventStatus, ControlStatus;

typedef enum {
    PinButton = 0,
    PinFANEnable = 2,
    PinFANDirctionOut = 3, 
    Pin_MAX
} GPIO_enum;

typedef enum {
    ButtonAction_POWER = 1,
    ButtonAction_DIR = 2,
} ButtonAction_st;

typedef enum {
	UPDATE_TYPE_FORCE = 1,  // 周期性强制更新
    UPDATE_PRESSED = 2, // 状态改变时更新
    UPDATE_TYPE_SERVICE_READ = 3, // 服务器查询时更新
    UPDATE_TYPE_UNMATCH_SYNC = 4, // app和实际状态不匹配时的同步
    UPDATE_TYPE_AUTO_POWEROFF = 5, // 超时，自动关机
} UPDATE_TYPE_t;

#define POWER_OFF_STABLE 1000    // 关机后延时，等完全停止后，再开机

#define CUSTOM_CMD_UPDATE_FIRMWARE  "update"
#define CUSTOM_CMD_GET              "get"
#define CUSTOM_CMD_ON                "on" // 短按开机
#define CUSTOM_CMD_OFF               "off"// 短按关机
#define CUSTOM_CMD_FORCE_POWEROFF    "forceoff"// 长按关机
#define CUSTOM_CMD_HELP              "help"
#define CUSTOM_CMD_DIR_IN             "in" // 吸进来
#define CUSTOM_CMD_DIR_OUT            "out" // 吹出去

typedef enum {
    POWER_STATE_IO_UNKONWN = -1,
    POWER_STATE_IO_OFF = (int)DISABLE, // 0
    POWER_STATE_IO_ON = (int)ENABLE,  // 1
} POWER_STATE_IO_t;

#define TOPIC_KEEP_ALIVE "alive" 
#define KEEPALIVEATIME (30 * 1000)
#define UPDATE_FORCE_PERIOD_S  (60 * 60)
#define WIFI_POWER_DB_DEFALUT   18 // max=20, min=0. 10，在旁边也连不上

//********************需要修改的部分*******************//
#define server_ip "bemfa.com" // 巴法云服务器地址默认即可
#define server_port 9501    // 服务器端口，tcp创客云端口8344

#define TOPIC_CTRL "Fan003"   // 控制电脑开关
#define UID  "bd384547fd7f4bb19e4ab4db837b1c47" // 用户私钥，可在控制台获取,修改为自己的UID
                      // 主题名字，可在控制台新建
const String g_updateURL = String("http://bin.bemfa.com/b/1BcYmQzODQ1NDdmZDdmNGJiMTllNGFiNGRiODM3YjFjNDc=") + TOPIC_CTRL + ".bin";                                                                                           // 主题名字，可在控制台新建

#endif