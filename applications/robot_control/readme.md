### 手指控制

数据传输格式[%d!%d!%d!%d!%d]，对应servo_do[]数组1-5。

细节注意：
1. servo_do[i].aim = int(map(data[i-1], 800, 4000, 500, 2500));//建议输入范围在1500-3200之间，这样就可以避免出现舵机卡住。
2. 对于1也就是大拇指与其它四个驱动方向不一致，1对应1000（蜷缩）到3200（伸展），而其它四指为1000（伸展）到3200（蜷缩）。
3. 速度调节：原程序`servo_do[i].inc = 2 * (servo_do[i].aim - servo_do[i].cur) / ( time1 / SERVO_TIME_PERIOD);`实测较慢，想要将动作速度提高只需将前面系数调大，可以根据实际发送频率更改设置。
4. 可能会出现传输过去但不移动，但同样的值再次发送才会移动，可能与机器本身回读自身状态有关，但未确定。

### 动作控制

数据传输格式{%d},对应动作组，可直接调用——也可后续改为直接修改参数，具体参数表明。

**case：**
**{1}   剪刀（会出声！）**，对应参数如下：

```cpp
  group_num_start = 14;
  group_num_end = 14;
  group_num_times = 1;
  do_time = 1;
  do_start_index = 14;
```
**{11}   石头（会出声！）**，对应参数如下：

```cpp
  group_num_start = 15;
  group_num_end = 15;
  group_num_times = 1;
  do_time = 1;
  do_start_index = 15;
```
**{2}   布（会出声！）**，对应参数如下：

```cpp
  group_num_start = 16;
  group_num_end = 16;
  group_num_times = 1;
  do_time = 1;
  do_start_index = 16;
```

**（PS：这里可以构建一个识别手势并语音播报的演示）**

**{5}   小车前进**，对应参数如下：

```cpp
  group_num_start = 95;
  group_num_end = 95;
  group_num_times = 1;
  do_time = 1;
  do_start_index = 95;
```
**{6}   小车左转**，对应参数如下：

```cpp
  group_num_start = 97;
  group_num_end = 97;
  group_num_times = 1;
  do_time = 1;
  do_start_index = 97;
```
**{7}   小车停止**，对应参数如下：

```cpp
  group_num_start = 91;
  group_num_end = 91;
  group_num_times = 1;
  do_time = 1;
  do_start_index = 91;
```
**{8}   小车右转**，对应参数如下：

```cpp
  group_num_start = 98;
  group_num_end = 98;
  group_num_times = 1;
  do_time = 1;
  do_start_index = 98;
```
**{9}   小车后退**，对应参数如下：

```cpp
  group_num_start = 96;
  group_num_end = 96;
  group_num_times = 1;
  do_time = 1;
  do_start_index = 96;
```

一些其它的动作：
**{3}  六手势 ruaike**，对应参数如下：

```cpp
  group_num_start = 12;
  group_num_end = 12;
  group_num_times = 1;
  do_time = 1;
  do_start_index = 12;
```
**{4}   自动演示**，对应参数如下：

```cpp
  group_num_start = 17;
  group_num_end = 81;
  group_num_times = 1;
  do_time = 1;
  do_start_index = 17;
```
**{10}   一二三四五数数(出声)**，对应参数如下：

```cpp
  group_num_start = 1;
  group_num_end = 10;
  group_num_times = 1;
  do_time = 1;
  do_start_index = 1;
```

**原理解析：**

该部分动作（尤其包括小车舵机控制）都是预编码好存储在**mem**`winbondFlashSPI mem;//声明flash对象，用于读写flash`的，经过一根总线直接调用的。
上述五个全局参量`group_num_start、group_num_end、group_num_times、do_time、do_start_index`都是传输到总线的参数。
```cpp
   if(group_num_start == group_num_end){
     do_group_once(group_num_start);
   }
   else{
     group_do_ok = 0;
   }
```
当动作组非单行时，就会`group_do_ok = 0`进入`loop_action()`函数内执行。


### HC-12

set接地时，可以AT指令查询波特率、传输模式，更改波特率、传输模式等等。详情见[用户手册](./HC-12用户手册V3.0_20230506.pdf)。串口助手为[HC-T串口助手V1.2](./HC-T串口助手V1.2(2022.01.07).exe)。

注意两个hc12模块必须分别单独设置好相同的波特率才可进行无线传输——此时set悬空或置高电平即可。