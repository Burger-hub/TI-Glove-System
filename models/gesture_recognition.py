#gesture_recogniton.py

import os
import torch
import torch.nn as nn
import numpy as np
import torch.nn.functional as F
import time

import tkinter as tk
from PIL import Image, ImageTk

import pyttsx3
# project root path
project_path = "./手势识别/"
# define log directory
model_path = project_path + "model.pt"

class CNNModel(nn.Module):
    def __init__(self):
        super().__init__()
        # the first convolution layer, 4 7x1 convolution kernels, output shape (batch_size, 5, 400)
        self.conv1 = nn.Conv1d(in_channels=8, out_channels=16, kernel_size=7, stride=1, padding='same')
        # the first pooling layer, max pooling, pooling size=3 , stride=2, output shape (batch_size, 5, 200)
        self.pool1 = nn.MaxPool1d(kernel_size=3, stride=2, padding=1)
        # the second convolution layer, 16 9x1 convolution kernels, output shape (batch_size, 16, 200)
        self.conv2 = nn.Conv1d(in_channels=16, out_channels=32, kernel_size=9, stride=1, padding='same')
        # the second pooling layer, max pooling, pooling size=3, stride=2, output shape (batch_size, 16, 100)
        self.pool2 = nn.MaxPool1d(kernel_size=3, stride=2, padding=1)
        # the third convolution layer, 32 11x1 convolution kernels, output shape (batch_size, 32, 100)
        self.conv3 = nn.Conv1d(in_channels=32, out_channels=64, kernel_size=11, stride=1, padding='same')
        # the third pooling layer, average pooling, pooling size=3, stride=2, output shape (batch_size, 32, 50)
        self.pool3 = nn.AvgPool1d(kernel_size=3, stride=2, padding=1)
        # the fourth convolution layer, 64 13x1 convolution kernels, output shape (batch_size, 64, 50)
        self.conv4 = nn.Conv1d(in_channels=64, out_channels=128, kernel_size=13, stride=1, padding='same')
        # flatten layer, for the next fully connected layer, output shape (batch_size, 50*128)
        self.flatten = nn.Flatten()
        # fully connected layer, 128 nodes, output shape (batch_size, 128)
        self.fc1 = nn.Linear(50 * 128, 128)
        # Dropout layer, dropout rate = 0.2
        self.dropout = nn.Dropout(0.2)
        # fully connected layer, 6 nodes (number of classes), output shape (batch_size, 5)
        self.fc2 = nn.Linear(128, 10)

    def forward(self, x):
        # x.shape = (batch_size, 100)
        # reshape the tensor with shape (batch_size, 100) to (batch_size, 1, 100)
        #x = x.reshape(-1, 1, 100)
        x = F.relu(self.conv1(x))
        x = self.pool1(x)
        x = F.relu(self.conv2(x))
        x = self.pool2(x)
        x = F.relu(self.conv3(x))
        x = self.pool3(x)
        x = F.relu(self.conv4(x))
        x = self.flatten(x)
        x = F.relu(self.fc1(x))
        x = self.dropout(x)
        x = self.fc2(x)
        return x

# 加载训练好的模型
model = CNNModel()
model.load_state_dict(torch.load(model_path))
model.eval()

# 定义预测函数
def predict(signal_tensor):
    # 转换信号为PyTorch张量
    # 进行预测
    with torch.no_grad():
        output = model(signal_tensor)
        
        probabilities = F.softmax(output, dim=1)  # 使用softmax将输出转换为概率
        print(probabilities)
        _, predicted = torch.max(probabilities, 1)  # 获取预测结果
    return predicted.item(), probabilities.squeeze().tolist()  # 返回预测结果和概率

set_num=400
num_points=0
signal_data=[]

def pre_process(segment):
    #TENG+IMU
    min_values = np.min(segment[:, :5], axis=0)
    max_values = np.max(segment[:, :5], axis=0)
    max_range = np.max(max_values- min_values)
    # Shift each channel's minimum to zero
    segment[:, :5] -= min_values
    # Scale each channel based on the maximum range
    segment[:, :5] = segment[:, :5]/max_range
    #方位角
    min_values = np.min(segment[:, 5], axis=0)
    segment[:, 5] = (segment[:, 5]-min_values)/90 
    #俯仰角
    min_values = np.min(segment[:, 6], axis=0)
    segment[:,6]= (segment[:, 6]-min_values)/90 
    #翻滚角
    min_values = np.min(segment[:, 7], axis=0)
    segment[:,7]= (segment[:, 7]-min_values)/90 
    return segment

def recognition(data_values):
    global num_points,signal_data
    #print(num_points)
    if num_points==int(set_num/3):
        a=[data_values[0],data_values[1],data_values[2],data_values[3],data_values[4],data_values[6],data_values[7],data_values[8]]
        signal_data.append(a)
        num_points=num_points+1
        #print(num_points)
        return 'wait',None
    elif num_points<set_num:
        a=[data_values[0],data_values[1],data_values[2],data_values[3],data_values[4],data_values[6],data_values[7],data_values[8]]
        signal_data.append(a)
        num_points=num_points+1
        return None,None
    elif num_points==set_num:   
        #print(np.shape(signal_data))
        segment=np.array(signal_data)
        pro_segment=pre_process(segment)
        #print(segment)
        signal_tensor = torch.tensor(pro_segment, dtype=torch.float32).unsqueeze(0)  # 添加批次维度
        #print(np.shape(signal_tensor))
        signal_tensor=np.transpose(signal_tensor,(0, 2, 1))
        #print(np.shape(signal_tensor))
        result, probabilities = predict(signal_tensor)
        signal_data=[]
        num_points=0
        return ClassSet[result], probabilities
    else:
        print('EORRO')
        return None,None

ClassSet=['B', 'Bye', 'C', 'Good', 'Hello', 'I', 'J', 'One', 'Yes', 'Z']

gestures_path=project_path + "gestures/"

class ResultWindow(tk.Toplevel):
    def __init__(self, master):
        super().__init__(master)
        self.title("Result")
        self.geometry("2560x1600")
        self.result_label = tk.Label(self, font=("Helvetica", 160))
        self.result_label.pack(padx=10, pady=10)

        self.image_label = tk.Label(self)
        self.image_label.pack(padx=10, pady=10)

    def update_result(self, result):
        image_path = gestures_path + result + ".JPG"
        image = Image.open(image_path)
        image = image.resize((1500, 1000))
        photo = ImageTk.PhotoImage(image)
        if result=='wait':
            self.result_label.config(text="Gesture: " )
            self.image_label.config(image=photo)
            self.image_label.image = photo
        else:
            self.result_label.config(text="Gesture: " + result)
            self.image_label.config(image=photo)
            self.image_label.image = photo

def show_result(result,window):
    window.update_result(result)

if __name__ == '__main__':
    root = tk.Tk()
    root.withdraw() 
    
    root = tk.Toplevel()
    window = ResultWindow(root)

    #data_values = [-_ for _ in range(20)]
    data_values=[0]*20
    for i in range(1600):
        for j in range(20):
            data_values[j]=data_values[j]+j
        #print(data_values) 
        result,probabilities=recognition(data_values)
        if result!=None:
            show_result(result,window)
            root.update()
            print("预测结果:", result)
            print("分类概率:", probabilities)    
        time.sleep(0.01)
    
    '''
    # 示例结果列表，这里假设你有一些结果需要展示
    results = ['One','Two','Seven','C','Nice','Y']

    # 循环调用show_result函数，并每隔4秒更新结果
    for result in results: 
        show_result(result,window)
        root.update()
        time.sleep(1)  # 等待4秒钟
    '''
    