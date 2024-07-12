
/*
 *发送数据到TCP服务器
 */
void sendtoTCPServer(String p)
{
    if (!TCPclient.connected()) {
        Serial.println("Client is not readly");
        return;
    }
    TCPclient.print(p);
}

/*
 *初始化和服务器建立连接 :style="value.online?'订阅设备在线':'无订阅设备'"  color:#9A9A9A;
 */
void startTCPClient()
{
    if (TCPclient.connect(server_ip, server_port)) {
        Serial.print("\nConnected to server:");
        Serial.printf("%s:%d\r\n", server_ip, server_port);

        String tcpTemp = "";                                       // 初始化字符串
        tcpTemp = "cmd=1&uid=";
        tcpTemp += UID;
        tcpTemp += "&topic=";
        tcpTemp += TOPIC;
        tcpTemp += "\r\n"; // 构建订阅指令

        sendtoTCPServer(tcpTemp);                                  // 发送订阅指令
        tcpTemp = "";                                              // 清空
        /*
         //如果需要订阅多个主题，可再次发送订阅指令
          tcpTemp = "cmd=1&uid="+UID+"&topic="+主题2+"\r\n"; //构建订阅指令
          sendtoTCPServer(tcpTemp); //发送订阅指令
          tcpTemp="";//清空
         */

        preTCPConnected = true;
        preHeartTick = millis();
        TCPclient.setNoDelay(true);
    } else {
        Serial.print("Failed connected to server:");
        Serial.println(server_ip);
        TCPclient.stop();
        preTCPConnected = false;
    }
    preTCPStartTick = millis();
}
