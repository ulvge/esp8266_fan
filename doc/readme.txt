项目功能：
	插座可实现远程遥控和定时
项目背景：
	天猫精灵插座只能手动开关，无法实现定时功能，于是重新刷入代码

硬件信息:
	吉控智能插座，YKYC-001。
	
	C:\Users\admin>esptool.py --port COM9 chip_id
	esptool.py v3.0
	Serial port COM9
	Connecting...
	Detecting chip type... ESP8266
	Chip is ESP8285
	Features: WiFi, Embedded Flash
	Crystal is 26MHz
	MAC: 84:0d:8e:41:95:13
	Uploading stub...
	Running stub...
	Stub running...
	Chip ID: 0x00419513
	Hard resetting via RTS pin...
	
	

软件信息：
	arduino + 巴法云 + mqtt
	
	https://cloud.bemfa.com/tcp/index.html
	
	灯的逻辑:
    // output pin: GPIO2


烧录方式：
	需要把GPIO0拉低并保持，再把RST也拉低，再释放，进行复位，此时芯片会进入下载模式，并一直等待烧录命令
	需要把GPIO0拉低并保持，再把RST也拉低，再释放，进行复位，此时芯片会进入下载模式，并一直等待烧录命令
	需要把GPIO0拉低并保持，再把RST也拉低，再释放，进行复位，此时芯片会进入下载模式，并一直等待烧录命令
	
    用镊子，一头接地，另一头把IO0拉低，然后碰一下RST，即可进入烧录模式

也可用esptool烧录
	安装烧录工具：
	pip install esptool
	esptool.py --port COM3 erase_flash

调试笔记
    **********************************************************************************************************
    MQTT上传
        https://blog.csdn.net/bemfa/article/details/107367547?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522172059668216800186528446%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fblog.%2522%257D&request_id=172059668216800186528446&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_ecpm_v1~rank_v31_ecpm-1-107367547-null-null.nonecase&utm_term=light002%2Fup&spm=1018.2226.3001.4450
        程序讲解：在本示例中，ESP8266 有两个角色，
        esp8266上传LED状态：一个是temp(传感器数据)主题消息的发布者，esp8266往这个主题推送消息，微信小程序就可以收到传感器数据了。
        手机控制esp8266开关：esp8266联网后，订阅light002，手机往这个主题推送消息，esp8266就能收到手机的控制的指令了。


        点击创建主题，要创建两个主题，一个主题用来传递传感器数据，另一个主题用来进行LED灯的控制。在本例程中一个主题名字是：temp ，
        另一个主题名字是：light002，可自定义或修改，不过下方微信小程序里面的主题名字要和esp8266的主题保持一致，以便正常的往同一个主题发布订阅。
        
        
        MQTT 调试工具 MQTT.fx
    **********************************************************************************************************
    不能烧相同代码，否则一条指令，控制所有
        当烧录的代码完全一样的时候，服务器，并不能区别谁是谁，但能识别出订阅者的个数。
    **********************************************************************************************************
    