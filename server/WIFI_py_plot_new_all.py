import socket
import threading
import queue
import tkinter as tk
from collections import deque
import time
from datetime import datetime
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.animation import FuncAnimation
import matplotlib.pyplot as plt
import csv

host = '192.168.137.1'  # 本地IP地址
port = 8081             # 端口号，可修改

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((host, port))
server_socket.listen(1)

print(f"等待连接在 {host}:{port}...")

client_socket, client_address = server_socket.accept()
print(f"连接来自 {client_address}")

all_received_data = []
data_queue = queue.Queue()
result_queue = queue.Queue()  # 用于在处理线程中传递处理结果
max_data_length = 1000
data_buffer = deque(maxlen=max_data_length)

# 设置Tkinter GUI
root = tk.Tk()
root.title("实时数据绘图")

num_channel=20 #时间戳，6通道电压，3个姿态角，9个原始数据，电量
fig, (ax, ax2, ax3,ax4) = plt.subplots(4, 1)  # 创建一个带有两个子图的图，垂直堆叠
#lines = [ax.plot([], [], label=f'Channel {i}')[0] for i in range(num_channel-1)]
lines = [ax.plot([], [], label=f'Channel {i}')[0] for i in range(5)]

canvas = FigureCanvasTkAgg(fig, master=root)
canvas_widget = canvas.get_tk_widget()
canvas_widget.pack(side=tk.TOP, fill=tk.BOTH, expand=1)

lines2= [ax2.plot([], [], label='IMU_yaw')[0],ax2.plot([], [], label='IMU_pitch')[0],ax2.plot([], [], label='IMU_roll')[0]]
lines3= [ax3.plot([], [], label='IMU_gyro_x')[0],ax3.plot([], [], label='IMU_gyro_y')[0],ax3.plot([], [], label='IMU_gyro_z')[0]]
lines4= [ax4.plot([], [], label='IMU_acc_x')[0],ax4.plot([], [], label='IMU_acc_y')[0],ax4.plot([], [], label='IMU_acc_z')[0]]

# 在 Tkinter 窗口中添加一个标签用于显示接收频率
receive_rate_label = tk.Label(root, text="接收频率: N/A")
receive_rate_label.pack(side=tk.BOTTOM)

# 在 Tkinter 窗口中添加一个标签用于显示接收频率
#receive_charge = tk.Label(root, text="电量: N/A")
#receive_charge.pack(side=tk.BOTTOM)

received_data_count = 0
start_time=time.time()

def receive_data_thread(client_socket):
    global received_data_count
    data = b''
    while True:
        chunk = client_socket.recv(2048)
        if not chunk:
            break
        data += chunk
        while b'\n\x00' in data:
            end_index = data.find(b'\n\x00') + 2
            message = data[:end_index]
            data = data[end_index:]
            message=str(message, errors = 'ignore')#
            #message_str = message.decode('utf-8').rstrip('\n\x00')
            message_str = message.rstrip('\n\x00')
            data_queue.put(message_str)

            # 计算接收频率
            received_data_count += 1
            update_receive_rate()

# 更新接收频率标签
def update_receive_rate():
    global received_data_count
    global start_time
    elapsed_time = time.time() - start_time
    if elapsed_time >= 1.0:  # 每秒更新一次
        receive_rate = received_data_count / elapsed_time
        received_data_count = 0
        start_time = time.time()
        receive_rate_label.config(text=f"接收频率: {receive_rate:.2f} Hz")

def parse_data(data_str):
    try:
        #data_values = [int(value.replace('\n', '').replace('\x00', '')) for value in data_str.split(',')]
        values = [value.replace('\n', '').replace('\x00', '') for value in data_str.split(',')]
        timestamp = int(values[0])
        #data_values = [int(value) for value in values[1:]]
        data_values = []
        if len(values) > num_channel-1:
            # 将前6个数据转换为整数
            for i in range(1,7):
                data_values.append(int(values[i]))
            # 将后13个数据转换为小数
            for i in range(7, 20):
                data_values.append(float(values[i]))
        
        if len(data_values) == num_channel-1:
            return timestamp, data_values
        else:
            print("接收到的数据不完整")
            print(len(data_values))
            return None,None
    except ValueError as e:
        print(f"无法解析数据: {e}")
        return None,None

def save_data_to_csv(filename, data_list):
    with open(filename, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['Timestamp'] + [f'Data{i}' for i in range(0, num_channel)])  # 写入表头

        for timestamp, data_values in data_list:
            row = [timestamp] + data_values
            writer.writerow(row)

##数据的应用
def use_data(timestamp, data_values):  
    print(timestamp, data_values)

# 处理数据的线程
def data_handler_thread():
    while True:
        try:
            data_to_handle = data_queue.get_nowait()
            timestamp, data_values = parse_data(data_to_handle)
            if data_values is not None:
                result_queue.put((timestamp, data_values))
                # 在这里使用数据，将处理结果放入结果队列
                #use_data(timestamp, data_values) 
        except queue.Empty:
            time.sleep(0.00001)  # 避免线程空转过快

# 修改绘图逻辑
def update_plot(frame):
    while True:
        try:
            timestamp, data_values = result_queue.get_nowait()
            data_buffer.append((timestamp, data_values)) #绘图数据
            all_received_data.append((timestamp, data_values))#所有数据
        except queue.Empty:
            break
    
    for i, line in enumerate(lines):
        #print(i)
        #line.set_data(range(len(data_buffer)), [data_values[i] for _, data_values in data_buffer])  #数组索引作为横坐标
        line.set_data([timestamp for timestamp, _ in data_buffer], [data_values[i] for _, data_values in data_buffer]) #时间戳作为横坐标
        #if i>5:#只展示IMU数据
        #    line.set_data([timestamp for timestamp, _ in data_buffer], [data_values[i] for _, data_values in data_buffer]) #时间戳作为横坐标
    for i, line in enumerate(lines2):
        line.set_data([timestamp for timestamp, _ in data_buffer], [data_values[i+6] for _, data_values in data_buffer]) #时间戳作为横坐标
    #lines2[0].set_data([timestamp for timestamp, _ in data_buffer], [data_values[7] for _, data_values in data_buffer]) #时间戳作为横坐标
    #lines2[1].set_data([timestamp for timestamp, _ in data_buffer], [data_values[8] for _, data_values in data_buffer]) #时间戳作为横坐标 
    #lines2[2].set_data([timestamp for timestamp, _ in data_buffer], [data_values[6] for _, data_values in data_buffer]) #时间戳作为横坐标  
    for i, line in enumerate(lines3):
        line.set_data([timestamp for timestamp, _ in data_buffer], [data_values[i+9] for _, data_values in data_buffer]) #时间戳作为横坐标
    for i, line in enumerate(lines4):
        line.set_data([timestamp for timestamp, _ in data_buffer], [data_values[i+12] for _, data_values in data_buffer]) #时间戳作为横坐标
    
    ax.relim()
    ax.autoscale_view()
    ax2.relim()
    ax2.autoscale_view()
    ax3.relim()
    ax3.autoscale_view()
    ax4.relim()
    ax4.autoscale_view()



# 启动 Matplotlib 动画
ani = FuncAnimation(fig, update_plot, interval=1,blit=False, cache_frame_data=False)#更新频率1毫秒
ax.legend(loc='upper right')  # 添加图例
ax2.legend(loc='upper right')  # 添加图例
ax3.legend(loc='upper right')  # 添加图例
ax4.legend(loc='upper right')  # 添加图例

if __name__ == "__main__":
    
    # 启动数据接收线程
    receive_thread = threading.Thread(target=receive_data_thread, args=(client_socket,))
    receive_thread.start()

    # 启动数据处理线程
    handler_thread = threading.Thread(target=data_handler_thread)
    handler_thread.start()

    try:
        root.mainloop()
    except KeyboardInterrupt:
        pass
    finally:
        print("关闭连接")

        saved_time=time.time()
        dt = datetime.fromtimestamp(saved_time)
        formatted_dt = dt.strftime("%Y-%m-%d_%H-%M-%S")
        saved_filename=f'./data_{formatted_dt}.csv'#设置保存文件名

        save_data_to_csv(saved_filename, all_received_data)
        print(f"数据已保存到 {saved_filename}")
        client_socket.close()
        server_socket.close()
        # 结束数据接收线程
        receive_thread.join()
        
        # 结束数据处理线程
        handler_thread.join()
