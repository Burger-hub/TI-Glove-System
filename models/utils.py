import pywt
import seaborn
import numpy as np
import matplotlib.pyplot as plt
from sklearn.metrics import confusion_matrix
from sklearn.model_selection import train_test_split
from torch.utils.data import random_split
import pandas as pd
import os
def standardize_data(data):
    mean = np.mean(data, axis=0)
    std = np.std(data, axis=0)
    standardized_data = (data - mean) / std
    return standardized_data

def load_data(data_path,test_ratio, random_seed,sample_gap,lasting_time):
    # 加载数据
    dataSet = []
    lableSet = []
    classSet=[]
    file_paths = [f for f in os.listdir(data_path) if f.endswith('.csv')]
    for file_path in file_paths:
        label = file_path.split('_')[0]  # 从文件名中获取标签
        if label not in classSet:
            classSet.append(label)
    '''
    file_paths = []
    for root,dir,files in os.walk(data_path):
        for file in files:
            # 检查文件是否以.csv结尾
            if file.endswith('.csv'):
                file_paths.append(file)
                label = str(file.split('.')[0])  # 从文件名中获取标签
                # 将csv文件名添加到列表中
                classSet.append(label)
    '''
    print(classSet)            
    #[batch_size, channel, length] #[batch_size, 4, 100]
    #lable = ecgClassSet.index(Rclass[i])
    #ClassSet=file_paths

    for file_path in file_paths:
        #label = str(file_path.split('.')[0])  # 从文件名中获取标签
        label = file_path.split('_')[0]  # 从文件名中获取标签
        label_idx=classSet.index(label)

        df = pd.read_csv(data_path+file_path)
        data = df.iloc[1:, 1:9].values  # 获取除了时间戳外的数据列
        #data = df.iloc[1:, 1:6].values  # 获取除了时间戳外的所有数据列
        #data = df.iloc[1:, 1].values  # 获取除了时间戳外的所有数据列
        #print(len(data))
        sample_point=int(lasting_time/sample_gap)
        num_samples = len(data) // sample_point # 每400个数据点分为一组,取整
        
        datas = np.split(data[:num_samples*sample_point], num_samples)  # 将数据分割为400个数据点的组
        #print(np.shape(datas))
        
        for i in range(len(datas)):
            segment = datas[i]
            '''
            # 处理前五个通道
            min_values = np.min(segment, axis=0)
            max_values = np.max(segment, axis=0)
            max_range = np.max(max_values - min_values)
            # Shift each channel's minimum to zero
            segment -= min_values
            # Scale each channel based on the maximum range
            segment = segment / max_range

            '''
            if label=='Bye':
                #TENG
                min_values = np.min(segment[:, :5], axis=0)
                max_values = np.max(segment[:, :5], axis=0)
                
                max_range = 2000000
                # Shift each channel's minimum to zero
                segment[:, :5] -= min_values
                # Scale each channel based on the maximum range
                segment[:, :5] = segment[:, :5]/max_range
            else:
                #TENG
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
            
            #
            datas[i] = segment 

        dataSet.extend(datas)
        lableSet.extend([label_idx] * num_samples)
        #plot_sample(label,datas,data_path)
        
    # 输出每类数据集的数据个数
    #for cls in classSet:
    #    num_data = lableSet.count(classSet.index(cls))
    #    print(f"Class {cls} has {num_data} samples.")
    
    # 从每个类别中取前100组数据
    filtered_dataSet = []
    filtered_labelSet = []
    for cls in classSet:
        class_idx = classSet.index(cls)
        count = 0
        for data, label in zip(dataSet, lableSet):
            if label == class_idx and count < 100:
                filtered_dataSet.append(data)
                filtered_labelSet.append(label)
                count += 1
    # 输出每类数据集的数据个数
    for cls in classSet:
        num_data = filtered_labelSet.count(classSet.index(cls))
        print(f"Class {cls} has {num_data} samples.")

    filtered_dataSet = np.array(filtered_dataSet)
    filtered_labelSet = np.array(filtered_labelSet)
    filtered_dataSet = np.transpose(filtered_dataSet, (0, 2, 1))
    #dataSet=np.transpose(dataSet,(0, 2, 1))
    #dataSet=np.permute(dataSet,(1,0))
    print(np.shape(filtered_dataSet))

    # 划分数据集
    X_train, X_test, y_train, y_test = train_test_split(filtered_dataSet, filtered_labelSet, test_size=test_ratio, random_state=random_seed)
    print("There are {} training samples".format(len(y_train)))
    print("There are {} testing samples".format(len(y_test)))
    return X_train, X_test, y_train, y_test,classSet

def plot_sample(label,datas,result_path):
    num_samples=len(datas)
    sample_points=len(datas[0])
    num_plot=5 #子图个数
    num_step=int(num_samples/num_plot)-1
    x=np.arange(sample_points)
    y=[]
    titles=[]
    for i in range(num_plot):
        idx=i*num_step
        y.append(datas[idx])
        title='sample '+str(idx)
        titles.append(title)
    fig, axes = plt.subplots(1, num_plot, figsize=(12, 3))  # 设置整个图的大小和子图个数
    for i, ax in enumerate(axes):
        ax.plot(x, y[i])
        ax.set_title(titles[i])
    # 添加一个整体图例
    fig.legend(labels=['TENG 0', 'TENG 1', 'TENG 2', 'TENG 3', 'TENG 4','yaw','pitch','roll'], loc='upper right')
    plt.suptitle('signal samples: '+str(label))  # 设置总图的标题
    plt.tight_layout()  # 调整子图布局，避免重叠
    plt.savefig(result_path+str(label)+'_sample.png')
    plt.show()
# confusion matrix
def plot_heat_map(y_test, y_pred,result_path,acc,labels):
    con_mat = confusion_matrix(y_test, y_pred)
    # normalize
    con_mat_norm = con_mat.astype('float') / con_mat.sum(axis=1)[:, np.newaxis]
    #con_mat_norm = np.around(con_mat_norm, decimals=2)
    print(con_mat_norm)

    # plot
    plt.figure(figsize=(10, 10))
    seaborn.heatmap(con_mat_norm, annot=True, fmt='.1%', cmap='Blues',xticklabels=labels, yticklabels=labels)
    #seaborn.heatmap(con_mat, annot=True, fmt='d', cmap='Blues')
    plt.ylim(0, 10)
    plt.xlabel('Predicted labels')
    plt.ylabel('True labels')
    plt.title('Accuracy:{:.2%} '.format(acc))
    plt.savefig(result_path+'confusion_matrix.png')
    plt.show()

def plot_history_torch(history,result_path):
    plt.figure(figsize=(8, 8))
    plt.plot(history['train_acc'])
    plt.plot(history['test_acc'])
    plt.title('Model Accuracy')
    plt.ylabel('Accuracy')
    plt.xlabel('Epoch')
    plt.legend(['Train', 'Test'], loc='upper left')
    plt.savefig(result_path+'accuracy.png')
    plt.show()

    # Save accuracy data to CSV
    accuracy_df = pd.DataFrame({
        'Epoch': range(1, len(history['train_acc']) + 1),
        'Train Accuracy': history['train_acc'],
        'Test Accuracy': history['test_acc']
    })
    accuracy_df.to_csv(result_path + 'accuracy.csv', index=False)

    plt.figure(figsize=(8, 8))
    plt.plot(history['train_loss'])
    plt.plot(history['test_loss'])
    plt.title('Model Loss')
    plt.ylabel('Loss')
    plt.xlabel('Epoch')
    plt.legend(['Train', 'Test'], loc='upper left')
    plt.savefig(result_path+'loss.png')
    plt.show()

    # Save loss data to CSV
    loss_df = pd.DataFrame({
        'Epoch': range(1, len(history['train_loss']) + 1),
        'Train Loss': history['train_loss'],
        'Test Loss': history['test_loss']
    })
    loss_df.to_csv(result_path + 'loss.csv', index=False)

if __name__ == '__main__':
    config = {
        'seed': 23,  # the random seed
        'test_ratio': 0.3,  # the ratio of the test set
        'num_epochs': 200,
        'batch_size': 32,
        'lr': 0.001,
        'sample_gap':0.01, #单位s
        'lasting_time':4 #单位s
    }
    project_path = "./手势识别/"
    # define log directory
    # must be a subdirectory of the directory specified when starting the web application
    # it is recommended to use the date time as the subdirectory name
    data_path=project_path + "data/"
    # X_train,y_train is the training set
    # X_test,y_test is the test set
    X_train, X_test, y_train, y_test,classSet = load_data(data_path,config['test_ratio'], config['seed'],config['sample_gap'],config['lasting_time'])
    #print(classSet)