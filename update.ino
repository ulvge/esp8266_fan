#include "comm.h"
/**
 * 固件升级函数
 * 在需要升级的地方，加上这个函数即可，例如setup中加的updateBin();
 * 原理：通过http请求获取远程固件，实现升级
 */
void updateBin()
{
    Serial.println("start update");
    WiFiClient UpdateClient;
    t_httpUpdate_return ret = ESPhttpUpdate.update(UpdateClient, g_updateURL);
    switch (ret) {
    case HTTP_UPDATE_FAILED: // 当升级失败
        Serial.println("[update] Update failed.");
        break;
    case HTTP_UPDATE_NO_UPDATES: // 当无升级
        Serial.println("[update] Update no Update.");
        break;
    case HTTP_UPDATE_OK: // 当升级成功
        Serial.println("[update] Update ok.");
        break;
    }
}
