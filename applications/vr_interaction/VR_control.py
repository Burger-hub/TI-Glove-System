#灯光控制
import serial
import serial.tools.list_ports
import time
import socket

#define
no_turn=b'8'

def open_socket():
    
    return None

#数据传输格式[%d!%d!%d!%d!%d] #大拇指~小拇指
signal_low=[100000,100000,800000,100000,100000]
signal_high=[1000000,1000000,900000,1000000,1000000]

start_time=0
#flag=0
set_frequecy=100
def get_finger_data(data_values):
    global start_time
    #flag=1 if flag==0 else 0 #100Hz减为50Hz
    gap_time=time.time()-start_time
    var=[0]*5
    command_num=[0]*5
    for i in range(5):
        var[i]=data_values[i]
        signal_norm=(var[i]-signal_low[i])/(signal_high[i]-signal_low[i])#归一化，完全弯曲为1
        if signal_norm<0:
            signal_norm=0
        elif signal_norm>1:
            signal_norm=1
        else:
            signal_norm=signal_norm
        command_num[i]=signal_norm
    finger=[0,0,0,0,0]
    for i in range(5):
        #finger[i]=command_num[i]
        finger[i]=command_num[2]
    command_str='%.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f'%(finger[0],finger[1],finger[2],finger[3],finger[4],data_values[6]-120,data_values[7],data_values[8])
    command=bytes(command_str, encoding='utf-8')  #转换为byte
    if gap_time>=1/set_frequecy: #到达设定时间才发送命令
        start_time=time.time()
        return command
    else:
        return b'8' #不输出命令
#{5}前进 {6}后退 {8}左转 {9}右转
def get_move_command(data_values):
    x=data_values[8]
    y=data_values[9]
    z=data_values[10]
    command=x
    #print(is_overLimit)
    return command

if __name__ == "__main__":
    print('Wrong')
    


