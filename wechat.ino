
#include <ArduinoJson.h>  // 确保包含 ArduinoJson 库

String g_wechatMsgType = "2";    

typedef struct  {
    bool isValid; // 是否有效
    String name;
    String deviceType;
} topicInfo;
topicInfo g_topicInfo = {false, "", ""}; // 主题信息

/// @brief 获取主题信息的名字和类型
/// @param openID
/// @param topic
/// @param type 1	MQTT协议设备; 3	HTTP协议设备; 5	MQTT协议设备V2版本，内测中;7	TCP协议设备V2版本，内测中
static bool GetTopicInfoFromService(String openID, String topic, int type = 1) {
    if (g_topicInfo.isValid) {
        return true; // 已经获取过了
    }
    HTTPClient http;
    WiFiClient wifiClient;
    String getTopicInfoUrl = "http://apis.bemfa.com/vb/api/v2/topicInfo?openID=" + openID + "&type=" + String(type) + "&topic=" + topic;
    // http://apis.bemfa.com/vb/api/v2/topicInfo?openID=bd384547fd7f4bb19e4ab4db837b1c47&type=1&topic=switchC001 // yes
    http.setTimeout(5000);  // 5秒超时
    http.begin(wifiClient, getTopicInfoUrl);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();

        // 解析 JSON
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);// 检查 JSON 解析是否成功
        if (error) {
            Serial.print("HTTP JSON GetTopicInfo parase error: ");
            Serial.println(error.c_str());
            return false;
        }

        if (doc["code"] == 0) {
           if (!doc["data"]["deviceType"].isNull()) {
                g_topicInfo.deviceType = doc["data"]["deviceType"].as<String>();
            } else {
                g_topicInfo.deviceType = ""; // 或默认值
            }
            if (!doc["data"]["name"].isNull()) {
                g_topicInfo.name = doc["data"]["name"].as<String>();
            } else {
                g_topicInfo.name = ""; // 或默认值
            }

            Serial.println("HTTP get deviceType: " + g_topicInfo.deviceType);
            Serial.println("HTTP get name: " + g_topicInfo.name); // sscom
            if (g_topicInfo.deviceType.length() > 0 && g_topicInfo.name.length() > 0) {
                g_topicInfo.isValid = true;
            } else {
                g_topicInfo.isValid = false;
            }
        } else {
            Serial.println("HTTP request failed, GetTopicInfo msg: " + doc["msg"].as<String>());
        }
    } else {
        Serial.println("HTTP request failed, GetTopicInfo httpCode code2: " + String(httpCode) + " url: " + getTopicInfoUrl);
    }

    http.end();
    return g_topicInfo.isValid;
}

/**
 * @brief 更新状态到微信的函数
 * @param cmd 需要发送到微信的命令字符串
 * @return bool 返回操作是否成功，true表示成功，false表示失败
 */
bool UpdateStateToWechat(String cmd) {
    WiFiClient wifiClient;
    // 从服务获取主题信息
    GetTopicInfoFromService(UID, TOPIC_CTRL);
    // 创建HTTP客户端对象
    HTTPClient http;
    // 构造 get 请求数据
    String deviceName;
    if (g_topicInfo.isValid) {
        deviceName = GetChineseDevType(g_topicInfo.deviceType) + "(" + g_topicInfo.name  + ")";
    }else{
        deviceName = TOPIC_CTRL;
    }
    // https://apis.bemfa.com/vb/wechat/v1/wechatWarn?uid=bd384547fd7f4bb19e4ab4db837b1c47&device=switchC001&message=haha // yes
    String wechatApiUrl = String("http://apis.bemfa.com/vb/wechat/v1/wechatWarn?uid=") + UID + "&device=" + String(deviceName) + "&message=" + cmd;    
    Serial.println("Update to Wechat url:" + wechatApiUrl);
    
    // 发起 HTTP 请求
    http.setTimeout(5000);  // 5秒超时
    http.begin(wifiClient, wechatApiUrl);
    int httpCode = http.GET();
    // 检查 HTTP 请求是否成功（200 表示成功）
    if (httpCode != HTTP_CODE_OK) {
        Serial.print("HTTP request failed, Update to Wechat error code: ");
        Serial.println(httpCode);
        http.end();  // 确保关闭连接
        return false;
    }

    // 获取响应数据
    String payload = http.getString();
    Serial.println("HTTP Update to Wechat success");
    Serial.println("\tpayload: " + payload);

    // 解析 JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    // 解析完成后关闭连接
    http.end();

    // 检查 JSON 解析是否成功
    if (error) {
        Serial.print("HTTP JSON parase, Update to Wechat error: ");
        Serial.println(error.c_str());
        return false;
    }

    // 检查返回的 code 是否为 0（成功）
    int responseCode = doc["code"];
    if (responseCode == 0) {
        Serial.println("HTTP push msg to wechat success");
        LastUpdateTickReset();// 和服务端同步完成，重置同步时间
        return true;
    } else {
        Serial.print("HTTP push msg to wechat failed, response code: ");
        Serial.println(responseCode);
        return false;
    }
}

// 设备类型结构体
struct DeviceType {
    const char* typeEnglish;
    const char* typeChinese;
};

// 设备类型对照表
const DeviceType DEVICE_TYPES[] = {
    {"outlet", "插座"},
    {"light", "灯"},
    {"fan", "风扇"},
    {"sensor", "传感器"},
    {"aircondition", "空调"},
    {"switch", "开关"},
    {"curtain", "窗帘"},
    {"thermostat", "温控器"},
    {"waterheater", "热水器"},
    {"television", "电视"},
    {"airpurifier", "空气净化器"}
};

const unsigned DEVICE_TYPE_COUNT = sizeof(DEVICE_TYPES) / sizeof(DEVICE_TYPES[0]);

// 查找函数
String GetChineseDevType(const String& englishTypeName) {
    // 转换为小写进行比较（不区分大小写）
    String lowerEnglish = englishTypeName;
    lowerEnglish.toLowerCase();
    
    for (unsigned int i = 0; i < DEVICE_TYPE_COUNT; i++) {
        if (lowerEnglish == DEVICE_TYPES[i].typeEnglish) {
            return String(DEVICE_TYPES[i].typeChinese);
        }
    }
    
    // 找不到时返回原始英文或错误提示
    return englishTypeName; // 或者 return "未知设备";
}