@echo on
esptool.py --chip esp8266 --port COM4 --baud 115200 write_flash 0x00000 D:\6misc\esp8266\baFaYun8266-01_Fan\build\esp8266.esp8266.esp8285\baFaYun8266-01_Fan.ino.bin
pause


