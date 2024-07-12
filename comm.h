#ifndef COMM_H
#define COMM_H



#define POWER_ON_OFF_DURATION 1000    // 开关机低脉冲时长
#define POWER_OFF_FROCE_DURATION 6500    // 强制关机低脉冲时长
//********************需要修改的部分*******************//
#define server_ip "bemfa.com" // 巴法云服务器地址默认即可
#define server_port 8344    // 服务器端口，tcp创客云端口8344

#define TOPIC "PC006"   // PC006 控制电脑开关
#define UID  "bd384547fd7f4bb19e4ab4db837b1c47" // 用户私钥，可在控制台获取,修改为自己的UID
                      // 主题名字，可在控制台新建
const String g_updateURL = "http://bin.bemfa.com/b/3BcYmQzODQ1NDdmZDdmNGJiMTllNGFiNGRiODM3YjFjNDc=" + String(TOPIC) + ".bin"; // 固件链接，在巴法云控制台复制、粘贴到这里即可

#endif