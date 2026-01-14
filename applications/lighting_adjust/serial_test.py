import serial
import serial.tools.list_ports
import time

def openSerial():
    # 获取所有串口设备实例。
    # 如果没找到串口设备，则输出：“无串口设备。”
    # 如果找到串口设备，则依次输出每个设备对应的串口号和描述信息。
    ports_list = list(serial.tools.list_ports.comports())
    if len(ports_list) <= 0:
        print("无串口设备。")
    else:
        print("可用的串口设备如下：")
        for comport in ports_list:
            print(list(comport)[0], list(comport)[1])
    global ser #全局变量
    ser= serial.Serial("COM3", 115200)    # 打开COM5，将波特率配置为115200，其余参数使用默认值
    if ser.isOpen():                        # 判断串口是否成功打开
        print("打开串口成功。")
        print(ser.name)    # 输出串口号
    else:
        print("打开串口失败。")
    input("Press Enter to continue...")

openSerial()#打开串口，回车进入下一步
while(1):
    ser.write(b"3")#开灯
    print(1)
    time.sleep(0.5) # 休眠0.1秒
    print(3)
    ser.write(b"2")#关灯
    time.sleep(0.5) # 休眠0.1秒
