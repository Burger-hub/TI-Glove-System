#机械手控制
import serial
import serial.tools.list_ports
import time
from pykalman import KalmanFilter
import pywt
import numpy as np
from scipy.signal import butter, lfilter, lfilter_zi
#define
no_turn=b'8'

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
    ser= serial.Serial("COM7", 115200)    # 打开COM6，将波特率配置为115200，其余参数使用默认值
    if ser.isOpen():                        # 判断串口是否成功打开
        print("打开串口成功。")
        print(ser.name)    # 输出串口号
        time.sleep(0.5)
        ser.write(b'[4000!800!800!800!800]') #写入命令'[%d!%d!%d!%d!%d]'
    else:
        print("打开串口失败。")
    input("Press Enter to continue...")
    return ser

# 巴特沃斯低通滤波器
def low_pass_filter(data_values, zi=None, cutoff_freq=2, fs=200, order=5):
    '''
    利用lfilter_zi函数获取滤波器的初始状态，然后使用lfilter函数应用滤波器,同时返回的zi可以保证滤波器的状态连续，可以在数据流中使用
    order:滤波器阶数，cutoff_freq:截止频率，fs:采样频率，order一般不要超过10，否则存储数据点太多，也会导致延迟增大
    '''
    nyquist_freq = 0.5 * fs
    normal_cutoff = cutoff_freq / nyquist_freq
    b, a = butter(order, normal_cutoff, btype='low', analog=False)
    if zi is None:
        zi = lfilter_zi(b, a) * data_values[0]  # 获取滤波器的初始状态
    smoothed_data, zi = lfilter(b, a, data_values, zi=zi)  # 应用滤波器并获取新的状态
    return smoothed_data.tolist(), zi  # 返回滤波后的数据和新的状态

def wavelet_smooth(data, wavelet='db4', level=3):
    '''
    需要一定数量的数据点才能进行
    '''
    # 执行小波变换
    coeffs = pywt.wavedec(data, wavelet, level=level)

    # 将高频部分系数置零，以实现平滑
    coeffs_smoothed = [coeffs[0]] + [np.zeros_like(coeffs[i]) for i in range(1, len(coeffs))]

    # 重构平滑后的信号
    data_smoothed = pywt.waverec(coeffs_smoothed, wavelet)

    return data_smoothed

#数据传输格式[%d!%d!%d!%d!%d] #大拇指~小拇指
signal_low=[1000000,1000000,500000,1000000,1000000]
signal_high=[1800000,1800000,510000,1800000,1800000]


servo_bend=[2400,3600,3600,3600,2400]#800-4000
servo_release=[2400,2200,2200,2100,2400]#中间值[2400,2400,2400,2400,2400]

start_time=0
#flag=0
set_frequecy=100
def get_hand_command(data_values):
    global start_time

    #flag=1 if flag==0 else 0 #100Hz减为50Hz
    gap_time=time.time()-start_time
    var=[0]*5
    command_num=[0]*5
    for i in range(5):
        #var[i]=data_values[4-i]
        var[i]=data_values[2]
        #wprint(var[i])
        signal_norm=(var[i]-signal_low[2])/(signal_high[2]-signal_low[2])#归一化，完全弯曲为1
        if signal_norm<0:
            signal_norm=0
        elif signal_norm>1:
            signal_norm=1
        else:
            signal_norm=signal_norm
        command_num[i]=servo_release[i]-signal_norm*(servo_release[i]-servo_bend[i]) #完全弯曲为1
        #command_num[i]=servo_bend[i]-signal_norm*(servo_bend[i]-servo_release[i]) #完全伸展为1
    command_str='[%d!%d!%d!%d!%d]'%(command_num[0],command_num[1],command_num[2],command_num[3],command_num[4])
    command=bytes(command_str, encoding='utf-8')  #转换为byte
    if gap_time>=1/set_frequecy: #到达设定时间才发送命令
        start_time=time.time()
        return command
    else:
        return b'8' #不输出命令
#{5}前进 {6}后退 {8}左转 {9}右转
    #{8}右转{9}后退{5}前进{6}左转{7}停止
turn_limit=40
for_back_limit=40
set_frequecy_2=100
start_time_2=0
def get_move_command(data_values):
    global start_time_2
    gap_time_2=time.time()-start_time_2
    roll=data_values[8]
    pitch=data_values[7]
    if gap_time_2>=1/set_frequecy_2: #到达设定时间才发送命令
        start_time=time.time()
        if roll<-turn_limit:#左转
            command=b'{6}'
        elif roll>turn_limit:#右转
            command=b'{8}'
        else:
            command=b'{7}'
    
        if pitch<-turn_limit:#前进
            command=b'{5}'
        elif pitch>turn_limit:#后退
            command=b'{9}'
        else:
            command=command
        return command
    else:
        return b'8' #不输出命令

if __name__ == "__main__":
    ###
    global ser
    ser=openSerial()
    
    ###
    data_values=[0,0,0,0,0]
    #var=get_command(data_values)
    while(1):
        
        
        command=input("input command:  ")
        var=bytes(command, encoding='utf-8')  #转换为byte
        ser.write(var) #写入命令
        
        '''
        #25Hz
        cir=125
        for i in range(cir):
            command="{5}"
            var=bytes(command, encoding='utf-8')
            print(var)
            ser.write(var) #写入命令
            time.sleep(0.04)      
        time.sleep(2)
        
        #data_values：0~500000 
        #50Hz不错    
        gap=2000
        cir=int(500000/gap)
        for i in range(cir):
            for j in range(5):
                data_values[j]=data_values[j]+gap
            hand_command=get_hand_command(data_values)
            print(data_values)
            if (hand_command!=b'8'):
                print(hand_command)
                ser.write(hand_command) #写入命令

            time.sleep(0.01)  # 避免线程空转过快
        for i in range(cir):
            for j in range(5):
                data_values[j]=data_values[j]-gap
            hand_command=get_hand_command(data_values)
            if (hand_command!=b'8'):    
                ser.write(hand_command) #写入命令
            time.sleep(0.01)  # 避免线程空转过快   
        #time.sleep(5)
        '''

    


