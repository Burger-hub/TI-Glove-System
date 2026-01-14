#灯光控制
import serial
import serial.tools.list_ports
import time

#define
turn_up=b'0'
turn_down=b'1'
turn_off=b'2'
turn_on=b'3'
turn_red=b'4'
turn_green=b'5'
turn_blue=b'6'
turn_white=b'7'
no_turn=b'8'

low=500000
high=1500000

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
    ser= serial.Serial("COM3", 115200)    # 打开COM5，将波特率配置为115200，其余参数使用默认值
    if ser.isOpen():                        # 判断串口是否成功打开
        print("打开串口成功。")
        print(ser.name)    # 输出串口号
    else:
        print("打开串口失败。")
    input("Press Enter to continue...")
    return ser

# 初始化灯光亮度
current_brightness = 5 / 10.0  # 例如，初始亮度为50%
brightness_levels = 10

def pre_process(data_values):
    #返回0~1的信号
    var=data_values[2]
    signal=(var-low)/(high-low)
    if signal>1:
        signal=1
    elif signal<0:
        signal=0
    else:
        signal=signal
    return signal

#0/亮度增加 1/亮度减小 2/关灯 3/开灯 4/红色 5/绿色 6/蓝色 7/白色 8/不动作 /亮度共10级（不包括关灯）
def get_bright_command(signal):
    global current_brightness
    #signal=data_values[3] #控制信号
    # 将传入的亮度映射到0到brightness_levels之间
    mapped_brightness = int((1-signal) * brightness_levels)#向下取整 0~9
    print(mapped_brightness)
    # 确保灯光级别在合法范围内
    mapped_brightness = max(0, min(mapped_brightness, brightness_levels - 1))
    
    last_brightness=current_brightness#上一状态的亮度
    current_brightness = mapped_brightness# 更新当前灯光亮度
    # 计算当前亮度对应的命令
    if current_brightness < last_brightness:
        command = turn_down
    elif current_brightness > last_brightness:
        command = turn_up
    else:
        command = no_turn
    return command

colorList=[turn_red,turn_green,turn_blue,turn_white]
color_index=3 # 初始化灯光颜色
acc_limit=-40
is_overLimit=0 #没超限
#左右，前后，翻滚
def get_color_command(data_values):
    global is_overLimit,color_index,colorList
    x=data_values[8]
    if ((x < acc_limit)&(is_overLimit==0)):#上升沿
        is_overLimit=1#标记
        color_index=color_index-1 if color_index>0 else 3
        command=colorList[color_index]#赋值命令
    elif((x > acc_limit)&(is_overLimit==1)):#下降沿
        is_overLimit=0#标记
        command=no_turn
    else:
        command=no_turn
    #print(is_overLimit)
    return command

if __name__ == "__main__":
    ###
    global ser
    ser=openSerial()
    ###
    #data_values=[0,0,0,0,0]
    #var=get_command(data_values)
    ser.write(turn_on) #先开灯
    data_values=[0]*12
    while(1):
        #command=input("input command:  ")
        #var=bytes(command, encoding='utf-8')  #转换为byte
        for i in range(100):
            data_values[8]=data_values[8]+200
            print(data_values[8])
            color_command=get_color_command(data_values)
            if (color_command!=b'8'):
                ser.write(color_command) #写入命令
            time.sleep(0.01)  # 避免线程空转过快
        for i in range(100):
            data_values[8]=data_values[8]-200
            print(data_values[8])
            color_command=get_color_command(data_values)
            if (color_command!=b'8'):
                ser.write(color_command) #写入命令
            time.sleep(0.01)  # 避免线程空转过快 
        time.sleep(5)
    
    '''
    # 模拟不断变化的亮度值
    brightness_values = [0.2, 0.4, 0.6, 0.8, 1.0, 0.5, 0.3]
    
    # 逐个处理亮度值
    for brightness_value in brightness_values:
        command = get_bright_command(brightness_value)
        if (command!=no_turn):
            ser.write(command) #写入命令
        print(f"传入亮度值 {brightness_value}，对应的命令是 {command}，当前灯光亮度为 {current_brightness}")
        time.sleep(3)  # 避免线程空转过快
    '''    


