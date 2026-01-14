import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
from torch.utils.data import Dataset, DataLoader
import pandas as pd
from sklearn.metrics import accuracy_score
from torchinfo import summary
from tqdm import tqdm
from torch.utils.tensorboard import SummaryWriter
import datetime
from utils import load_data, plot_history_torch, plot_heat_map
import os
import random
# 导入计算F1分数所需的库
from sklearn.metrics import f1_score
import time

# project root path
project_path = "./手势识别/"
# define log directory
# must be a subdirectory of the directory specified when starting the web application
# it is recommended to use the date time as the subdirectory name
log_dir = project_path + "results/logs/" + datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
data_path=project_path + "data/"
result_path=project_path + "results/"
model_path = result_path + "model.pt"

# the device to use
device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
#device = torch.device("cpu")
print("Using {} device".format(device))

class SensorsDataset(Dataset):
    def __init__(self, data,label):
        self.data = data
        self.label = label

    def __len__(self):
        return len(self.data)
    
    def __getitem__(self, idx):
        x = torch.tensor(self.data[idx], dtype=torch.float32)
        y = torch.tensor(self.label[idx], dtype=torch.long)
        return x, y

class CNNModel(nn.Module):
    def __init__(self):
        super().__init__()
        # the first convolution layer, 4 7x1 convolution kernels, output shape (batch_size, 16, 400)
        self.conv1 = nn.Conv1d(in_channels=8, out_channels=16, kernel_size=7, stride=1, padding='same')
        # the first pooling layer, max pooling, pooling size=3 , stride=2, output shape (batch_size, 16, 200)
        self.pool1 = nn.MaxPool1d(kernel_size=3, stride=2, padding=1)
        # the second convolution layer, 16 9x1 convolution kernels, output shape (batch_size, 32, 200)
        self.conv2 = nn.Conv1d(in_channels=16, out_channels=32, kernel_size=9, stride=1, padding='same')
        # the second pooling layer, max pooling, pooling size=3, stride=2, output shape (batch_size, 32, 100)
        self.pool2 = nn.MaxPool1d(kernel_size=3, stride=2, padding=1)
        # the third convolution layer, 32 11x1 convolution kernels, output shape (batch_size, 64, 100)
        self.conv3 = nn.Conv1d(in_channels=32, out_channels=64, kernel_size=11, stride=1, padding='same')
        # the third pooling layer, average pooling, pooling size=3, stride=2, output shape (batch_size, 64, 50)
        self.pool3 = nn.AvgPool1d(kernel_size=3, stride=2, padding=1)
        # the fourth convolution layer, 64 13x1 convolution kernels, output shape (batch_size, 128, 50)
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

# define the training function and validation function
def train_steps(loop, model, criterion, optimizer):
    train_loss = []
    train_acc = []
    model.train()
    for step_index, (X, y) in loop:
        X, y = X.to(device), y.to(device)
        pred = model(X)
        loss = criterion(pred, y)

        optimizer.zero_grad()
        loss.backward()
        optimizer.step()

        loss = loss.item()
        train_loss.append(loss)
        pred_result = torch.argmax(pred, dim=1).detach().cpu().numpy()
        y = y.detach().cpu().numpy()
        acc = accuracy_score(y, pred_result)
        train_acc.append(acc)
        loop.set_postfix(loss=loss, acc=acc)
    return {"loss": np.mean(train_loss),
            "acc": np.mean(train_acc)}

def test_steps(loop, model, criterion):
    test_loss = []
    test_acc = []
    model.eval()
    with torch.no_grad():
        for step_index, (X, y) in loop:
            X, y = X.to(device), y.to(device)
            pred = model(X)
            loss = criterion(pred, y).item()

            test_loss.append(loss)
            pred_result = torch.argmax(pred, dim=1).detach().cpu().numpy()
            y = y.detach().cpu().numpy()
            acc = accuracy_score(y, pred_result)
            test_acc.append(acc)
            loop.set_postfix(loss=loss, acc=acc)
    return {"loss": np.mean(test_loss),
            "acc": np.mean(test_acc)}

def train_epochs(train_dataloader, test_dataloader, model, criterion, optimizer, config, writer):
    start_time = time.time()  # 开始计时
    num_epochs = config['num_epochs']
    train_loss_ls = []
    train_loss_acc = []
    test_loss_ls = []
    test_loss_acc = []
    for epoch in range(num_epochs):
        train_loop = tqdm(enumerate(train_dataloader), total=len(train_dataloader))
        test_loop = tqdm(enumerate(test_dataloader), total=len(test_dataloader))
        train_loop.set_description(f'Epoch [{epoch + 1}/{num_epochs}]')
        test_loop.set_description(f'Epoch [{epoch + 1}/{num_epochs}]')

        train_metrix = train_steps(train_loop, model, criterion, optimizer)
        test_metrix = test_steps(test_loop, model, criterion)

        train_loss_ls.append(train_metrix['loss'])
        train_loss_acc.append(train_metrix['acc'])
        test_loss_ls.append(test_metrix['loss'])
        test_loss_acc.append(test_metrix['acc'])

        print(f'Epoch {epoch + 1}: '
              f'train loss: {train_metrix["loss"]}; '
              f'train acc: {train_metrix["acc"]}; ')
        print(f'Epoch {epoch + 1}: '
              f'test loss: {test_metrix["loss"]}; '
              f'test acc: {test_metrix["acc"]}')

        writer.add_scalar('train/loss', train_metrix['loss'], epoch)
        writer.add_scalar('train/accuracy', train_metrix['acc'], epoch)
        writer.add_scalar('validation/loss', test_metrix['loss'], epoch)
        writer.add_scalar('validation/accuracy', test_metrix['acc'], epoch)

    end_time = time.time()  # 结束计时
    train_duration = end_time - start_time  # 计算训练时长

    return {'train_loss': train_loss_ls,
            'train_acc': train_loss_acc,
            'test_loss': test_loss_ls,
            'test_acc': test_loss_acc,
            'train_duration': train_duration}

def setup_seed(seed):
     torch.manual_seed(seed)
     torch.cuda.manual_seed_all(seed)
     np.random.seed(seed)
     random.seed(seed)
     torch.backends.cudnn.deterministic = True
    
def main():
    config = {
        'seed': 29,  # the random seed#原通道23最佳
        'test_ratio': 0.3,  # the ratio of the test set
        'num_epochs': 130,
        'batch_size': 32,
        'lr': 0.001,
        'sample_gap':0.01, #单位s
        'lasting_time':4 #单位s
    }
    setup_seed(config['seed'])
    
    # X_train,y_train is the training set
    # X_test,y_test is the test set
    X_train, X_test, y_train, y_test,classSet = load_data(data_path,config['test_ratio'], config['seed'],config['sample_gap'],config['lasting_time'])
    train_dataset, test_dataset = SensorsDataset(X_train, y_train), SensorsDataset(X_test, y_test)
    train_dataloader = DataLoader(train_dataset, batch_size=config['batch_size'], shuffle=True)
    test_dataloader = DataLoader(test_dataset, batch_size=config['batch_size'], shuffle=False)
    
    # 定义模型
    model = CNNModel()
    if os.path.exists(project_path+"model.pt"):
        # import the pre-trained model if it exists
        print('Import the pre-trained model, skip the training process')
        model.load_state_dict(torch.load(project_path+"model.pt"))
        model.eval()
    else:
        # build the CNN model
        model = model.to(device)
        criterion = nn.CrossEntropyLoss()
        optimizer = torch.optim.Adam(model.parameters(), lr=config['lr'])

        # print the model structure
        #summary(model, (config['batch_size'], np.array(X_train).shape[1]), col_names=["input_size", "kernel_size", "output_size"],
        #        verbose=2)

        # define the Tensorboard SummaryWriter
        writer = SummaryWriter(log_dir=log_dir)
        # train and evaluate model
        history = train_epochs(train_dataloader, test_dataloader, model, criterion, optimizer, config, writer)
        writer.close()
        # 确保保存模型时的文件夹路径存在
        os.makedirs(os.path.dirname(model_path), exist_ok=True)
        # save the model
        torch.save(model.state_dict(), model_path)
        print(f"Training Duration: {history['train_duration']:.4f} seconds")
        # plot the training history
        plot_history_torch(history,result_path)

    # predict the class of test data
    # 计算平均推理时长
    # 计算平均推理时长
    total_inference_time = 0
    num_samples = 0
    y_true = []
    y_pred = []
    model.eval()
    with torch.no_grad():
        for step_index, (X, y) in enumerate(test_dataloader):
            X, y = X.to(device), y.to(device)
            start_time = time.time()
            pred = model(X)
            end_time = time.time()
            # 累加推理时间
            total_inference_time += (end_time - start_time)
            # 累加样本数
            num_samples += X.size(0)
            _, predicted = torch.max(pred, 1)
            y_true.extend(y.cpu().numpy())
            y_pred.extend(predicted.cpu().numpy())
            #pred_result = torch.argmax(pred, dim=1).detach().cpu().numpy()
            #y_pred.extend(pred_result)
    acc = accuracy_score(y_true, y_pred)
    f1 = f1_score(y_true, y_pred, average='macro')

    average_inference_time = total_inference_time / num_samples * 1000# 转换为毫秒并计算每个样本的平均推理时间
    print(classSet)
    print(f"Model Accuracy: {acc:.4f}")

    print(f"F1 Score: {f1:.4f}")
    print(f"Average Inference Time: {average_inference_time:.3f} ms")
    # plot confusion matrix heat map
    plot_heat_map(y_test, y_pred,result_path,acc,classSet)
    
if __name__ == '__main__':
    main()