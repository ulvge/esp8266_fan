#ifndef COMM_H
#define COMM_H


typedef enum {
    UPDATE_PERIOD = 1,  // 周期性更新
    UPDATE_STATE_CHANGED = 2 // 状态改变时更新
} UPDATE_MODE_t;

#define POWER_ON_OFF_DURATION 1000    // 开关机低脉冲时长
#define POWER_OFF_FROCE_DURATION 6500    // 强制关机低脉冲时长

#define CUSTOM_CMD_UPDATE_FIRMWARE  "update"
#define CUSTOM_CMD_GET              "get"
#define CUSTOM_CMD_ON                "on" // 短按开机
#define CUSTOM_CMD_OFF               "off"// 短按关机
#define CUSTOM_CMD_FORCE_POWEROFF    "forceoff"// 长按关机
#define CUSTOM_CMD_HELP              "help"
#define CUSTOM_CMD_RF_DB              "db"

#define  powerCheckPin_active LOW  // 默认是高，低有效

#define TOPIC_KEEP_ALIVE "alive" 
#define KEEPALIVEATIME 30 * 1000
#define WIFI_POWER_DB_DEFALUT   18 // max=20, min=0. 10，在旁边也连不上

//********************需要修改的部分*******************//
#define server_ip "bemfa.com" // 巴法云服务器地址默认即可
#define server_port 9501    // 服务器端口，tcp创客云端口8344

#define TOPIC_PCPowerCtrl "PCPowerCtrl006"   // 控制电脑开关
#define UID  "bd384547fd7f4bb19e4ab4db837b1c47" // 用户私钥，可在控制台获取,修改为自己的UID
                      // 主题名字，可在控制台新建
const String g_updateURL = String("http://bin.bemfa.com/b/1BcYmQzODQ1NDdmZDdmNGJiMTllNGFiNGRiODM3YjFjNDc=") + TOPIC_PCPowerCtrl + ".bin";                                                                                           // 主题名字，可在控制台新建

#endif