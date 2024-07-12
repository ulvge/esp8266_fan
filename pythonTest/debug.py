import paho.mqtt.client as mqtt
import time

# 定义常量
SERVER_IP = "bemfa.com"  # 巴法云服务器地址
SERVER_PORT = 9501  #8344       # 服务器端口    ，两者端口不一样 https://bemfa.com/m/mqttfx.html
TOPIC = "switch001"     # 主题
UID = "bd384547fd7f4bb19e4ab4db837b1c47"  # 用户私钥

# MQTT 连接回调函数
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected successfully!")
        #    # 订阅主题
        
        tcpTemp = "cmd=1&uid=";
        tcpTemp += UID;
        tcpTemp += "&topic=";
        tcpTemp += TOPIC;
        tcpTemp += "\r\n";
        #client.subscribe(TOPIC, qos=0)  # 订阅主题 /up
        client.subscribe(tcpTemp, qos=0)  # 订阅主题 /up

        # 发布测试消息
        client.publish(f"{TOPIC}/up", "ON")  # 发布状态 "ON"
        print(f"Published 'ON' to topic '{TOPIC}/up'")
        client.publish(f"{TOPIC}/up", "OFF")  # 发布状态 "OFF"
        print(f"Published 'OFF' to topic '{TOPIC}/up'")
    else:
        print(f"Failed to connect with result code {rc}")

# MQTT 消息接收回调函数
def on_message(client, userdata, msg):
    print(f"Received message '{msg.payload.decode()}' on topic '{msg.topic}'")
# MQTT 消息发布回调函数
def on_publish(client, userdata, mid):
    print(f"Message {mid} published.")
    
# 创建 MQTT 客户端实例
client = mqtt.Client()

# 设置用户名和密码
client.username_pw_set(UID, "")  # UID作为用户名，密码留空

# 设置回调函数
client.on_connect = on_connect
client.on_publish = on_publish
client.on_message = on_message  # 设置消息接收回调函数

# 连接到MQTT服务器
try:
    client.connect(SERVER_IP, SERVER_PORT, 60)
    time.sleep(1)
except Exception as e:
    print(f"Error connecting to the server: {e}")

# 开始循环，处理网络流量和回调
client.loop_forever()
