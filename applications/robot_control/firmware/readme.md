## 特别标识，易于改动后的程序进一步处理

//##     表示下面这个函数可以注释掉，与项目预期功能无关
%%%%     表示这个变量or函数是原控制板没有的，项目新增

---

## 一些重要功能函数的解读

1. **serialEvent()**
serialEvent() 是一个可选的用户定义函数，如果用户在代码中定义了这个函数，那么它将在主循环中被定期调用，而不是通过中断触发。
所以，在这个程序中，serialEvent() 是通过主循环 loop() 主动被调用的，并不是一个真正的中断服务例程。这个函数用于模拟串口接收中断的功能，实际上它是在主循环中轮询串口缓冲区，检查是否有数据可读，然后进行相应的处理。在这个函数中，程序通过读取串口中的字符，并根据接收到的字符来确定数据的格式。具体来说：
如果接收到字符 <，则将 uart1_mode 设置为 4，表示进入下载模式，并记录当前时间。
**如果接收到字符 {，则将 uart1_mode 设置为 3。**
如果接收到字符 $，则将 uart1_mode 设置为 1。
如果接收到字符 #，则将 uart1_mode 设置为 2。
在每一种模式下，都会将 uart_receive_str 清空，并开始接收新的字符，直到接收到相应的结束字符（>, }, !）时，将 uart1_get_ok 置为 1，表示已经成功接收到一条完整的指令。此外，还会将接收到的字符串备份到 uart_receive_str_bak。

2. **loop_uart()**
这个函数在主循环 loop() 中被调用。在这个函数中，首先检查 uart1_get_ok 是否为真。如果为真，表示已经成功接收到一条指令，根据 uart1_mode 的值调用相应的处理函数：
如果 uart1_mode 为 1，调用 parse_cmd(uart_receive_buf) 处理命令。
**如果 uart1_mode 为 2 或 3，调用 parse_action(uart_receive_buf) 处理动作。(项目所需要的)**
如果 uart1_mode 为 4，调用 save_action(uart_receive_buf) 保存动作。
然后，将 uart1_get_ok、uart1_mode 置零，清空接收字符串 uart_receive_str。
最后，还有一个超时判断，如果当前时间减去上一次下载指令的时间超过 3000 毫秒，将 downLoad 置为假，表示下载结束。

上述函数原为：
```cpp
//解析串口接收到的字符串指令
void loop_uart(){
    if(uart1_get_ok) {
    	  //如果有同步标志直接返回，让同步函数处理
        if(flag_sync)return;
        //打印字符串，测试的时候可以用
        //转换成字符串数组
        uart_receive_str.toCharArray(uart_receive_buf, uart_receive_str.length()+1);
        uart_receive_str_len = uart_receive_str.length();
        if(uart1_mode == 1) {
            parse_cmd(uart_receive_buf);
        } else if(uart1_mode == 2 || uart1_mode == 3){
            parse_action(uart_receive_buf);
        } else if(uart1_mode == 4){
            save_action(uart_receive_buf);
        }
        uart1_get_ok = 0;
        uart1_mode = 0;
        uart_receive_str = "";
    }
    if(millis()-downLoadSystickMs>3000) {
    	downLoad = false;
    }
}
//串口中断
void serialEvent() {
    static char sbuf_bak;
    while(Serial.available())  {      //
        sbuf_bak = char(Serial.read());

        if(uart1_get_ok) return;
        if(uart1_mode == 0) {
          if(sbuf_bak == '<') {
            uart1_mode = 4;
            downLoadSystickMs = millis();//通过它单开一个线程——从而实现保存动作组
			  downLoad = true;//loop函数里作为中断符
          } else if(sbuf_bak == '{') {
            uart1_mode = 3;
          } else if(sbuf_bak == '$') {
            uart1_mode = 1;
          } else if(sbuf_bak == '#') {
            uart1_mode = 2;
          }
          uart_receive_str = "";
        }

        uart_receive_str  += sbuf_bak;

        if((uart1_mode == 4) && (sbuf_bak == '>')){
          uart1_get_ok = 1;
        } else if((uart1_mode == 3) && (sbuf_bak == '}')){
          uart1_get_ok = 1;
        } else if((uart1_mode == 1) && (sbuf_bak == '!')){
          uart_receive_str_bak = uart_receive_str;
          uart1_get_ok = 1;
        } else if((uart1_mode == 2) && (sbuf_bak == '!')){
          uart1_get_ok = 1;
        }

        if(uart_receive_str.length() >= RECV_SIZE) {
            uart_receive_str = "";
        }
    }
}
```

3.**舵机控制类**
要记住的就是舵机值是500——2500.初始化为1500
还有就是六个servo，第一个为手腕处的servo，i=1-5为五个手指（见glove.ino程序中的这部分可知）
```cpp
//获取5个手指的数值
        adc_read2buf();

        //pwm控制
        for(int i=0;i<5;i++) {
            pwm_value[i] = (int)(1500+(pos[i]-ADC_MID[i])*pos_p[i]);
            
            if(abs(pos_bak[i] - pos[i]) >= 5) {
                pos_bak[i] = pos[i];
                
                if(pwm_value[i]>2500)pwm_value[i]=2500;
                else if(pwm_value[i]<500)pwm_value[i]=500;
                
                myservo[i+1].writeMicroseconds(pwm_value[i]); 
            }
        } 
```

程序中实际有用部分内容——变量及函数。

```cpp
#define SERVO_NUM 6
#define SERVO_TIME_PERIOD    20    //每隔20ms处理一次（累加）舵机的PWM增量
byte servo_pin[SERVO_NUM] = {7, 3, 5, 6, 9 ,8}; //宏定义舵机控制引脚
typedef struct {                  //舵机结构体变量声明
    unsigned int aim = 1500;      //舵机目标值
    float cur = 1500.0;           //舵机当前值
    unsigned  int time1 = 1000;   //舵机执行时间
    float inc= 0.0;               //舵机值增量，以20ms为周期
}duoji_struct;
duoji_struct servo_do[SERVO_NUM];           //用结构体变量声明一个舵机变量组
//六路舵机初始化
void setup_servo() {
    //蜷缩 "{G0002#000P1500T1500!#001P2200T1500!#002P2500T1500!#003P2000T1500!#004P1500T1500!#005P1500T1500!}",
    if(eeprom_info.pre_cmd[PRE_CMD_SIZE] != FLAG_VERIFY) {
    	servo_do[0].aim = servo_do[0].cur = 1500 + eeprom_info.dj_bias_pwm[0];servo_do[0].inc=0;
    	servo_do[1].aim = servo_do[1].cur = 1500 + eeprom_info.dj_bias_pwm[1];servo_do[1].inc=0;
    	servo_do[2].aim = servo_do[2].cur = 1500 + eeprom_info.dj_bias_pwm[2];servo_do[2].inc=0;
    	servo_do[3].aim = servo_do[3].cur = 1500 + eeprom_info.dj_bias_pwm[3];servo_do[3].inc=0;
    	servo_do[4].aim = servo_do[4].cur = 1500 + eeprom_info.dj_bias_pwm[4];servo_do[4].inc=0;
    	servo_do[5].aim = servo_do[5].cur = 1500 + eeprom_info.dj_bias_pwm[5];servo_do[5].inc=0;
    }

    for(byte i = 0; i < SERVO_NUM; i ++){
        myservo[i].attach(servo_pin[i]);   // 将10引脚与声明的舵机对象连接起来
        myservo[i].writeMicroseconds(servo_do[i].aim);
    }
}
//舵机PWM增量处理函数，每隔SERVO_TIME_PERIOD毫秒处理一次，这样就实现了舵机的连续控制
void loop_servo() {
  static byte servo_monitor[SERVO_NUM] = {0};
  static long long systick_ms_bak = 0;
  if(millis() - systick_ms_bak > SERVO_TIME_PERIOD) {
      systick_ms_bak = millis();
      for(byte i=0; i<SERVO_NUM; i++) {
          if(servo_do[i].inc) {
              if(abs( servo_do[i].aim - servo_do[i].cur) <= abs (servo_do[i].inc) ) {
                   myservo[i].writeMicroseconds(servo_do[i].aim);
                   servo_do[i].cur = servo_do[i].aim;
                   servo_do[i].inc = 0;
              } else {
                     servo_do[i].cur +=  servo_do[i].inc;
                     myservo[i].writeMicroseconds((int)servo_do[i].cur);
              }
          } else {
          }
      }
  }
}


void set_servo(int mindex, int mpwm, int mtime) {
	servo_do[mindex].aim = mpwm;
	servo_do[mindex].time1 = mtime;
	servo_do[mindex].inc = (servo_do[mindex].aim -  servo_do[mindex].cur) / (servo_do[mindex].time1/20.000);
	sprintf((char *)cmd_return, "#%03dP%04dT%04d! ", mindex, mpwm, mtime);
	Serial.print(cmd_return);
	//Serial.println(kinematics.servo_angle[mindex]);
}```


//还有parse_action的部分
/*for(int i=0;i<SERVO_NUM;i++) {
                    		pwm2 = pwm1+eeprom_info.dj_bias_pwm[i];
                    		if(pwm2 > 2500)pwm2 = 2500;
                    		if(pwm2 < 500)pwm2 = 500;
                    		servo_do[i].aim = pwm2; //舵机PWM赋值,加上偏差的值
                    		servo_do[i].time1 = time1;      //舵机执行时间赋值
                    		float pwm_err = servo_do[i].aim - servo_do[i].cur;
                    		servo_do[i].inc = (pwm_err*1.00)/(time1/SERVO_TIME_PERIOD); //根据时间计算舵机PWM增量
						}
                } else if((index >= SERVO_NUM) || (pwm1 > 2500) ||(pwm1 < 500)|| (time1>10000)) {  //如果舵机号和PWM数值超出约定值则跳出不处理
                } else {
                    servo_do[index].aim = pwm1+eeprom_info.dj_bias_pwm[index];; //舵机PWM赋值,加上偏差的值
                    if(servo_do[index].aim > 2500)servo_do[index].aim = 2500;
                    if(servo_do[index].aim < 500)servo_do[index].aim = 500;
                    servo_do[index].time1 = time1;      //舵机执行时间赋值
                    float pwm_err = servo_do[index].aim - servo_do[index].cur;
                    servo_do[index].inc = (pwm_err*1.00)/(time1/SERVO_TIME_PERIOD); //根据时间计算舵机PWM增量
                }*/
```

---

## 根据功能需求设计更新后的新函数

1.**新的serialEvent()函数**
单开了两个模式，“[]”模式为单纯的只写手指弯曲的，“**”加入了陀螺仪数据后有了原手套程序的那些姿态判断。
注意新增的servo_input_flag 、gyro_input_flag标志

```cpp
//新的串口中断函数
//“[4096!4096!4096]”型为三个手指的电位值，“*4096!4096!4096!111!111*”为舵机电位值+陀螺仪的值
//实际手套传入都信息中没有$、#、<型，所以这里大胆改动
void serialEvent() {
    static char sbuf_bak;
    while(Serial.available())  {      //
        sbuf_bak = char(Serial.read());

        if(uart1_get_ok) return;
        if(uart1_mode == 0) {
          if(sbuf_bak == '<') {
            uart1_mode = 4;
            downLoadSystickMs = millis();
			  downLoad = true;
          } else if(sbuf_bak == '{') {
            uart1_mode = 3;
          } else if(sbuf_bak == '$') {
            uart1_mode = 1;
          } else if(sbuf_bak == '#') {
            uart1_mode = 2;
          } else if(sbuf_bak == '[') {
            uart1_mode = 5;
            servo_input_flag = 1;
          } else if(sbuf_bak == '*') {
            gyro_input_flag = 1;
            uart1_mode = 6;
          }

          uart_receive_str = "";
        }

        uart_receive_str  += sbuf_bak;

        if((uart1_mode == 4) && (sbuf_bak == '>')){
          uart1_get_ok = 1;
        } else if((uart1_mode == 3) && (sbuf_bak == '}')){
          uart1_get_ok = 1;
        } else if((uart1_mode == 1) && (sbuf_bak == '!')){
          uart_receive_str_bak = uart_receive_str;
          uart1_get_ok = 1;
        } else if((uart1_mode == 2) && (sbuf_bak == '!')){
          uart1_get_ok = 1;
        }else if((uart1_mode == 5) && (sbuf_bak == ']')){
          uart1_get_ok = 1;
        }else if((uart1_mode == 6) && (sbuf_bak == '*')){
          uart1_get_ok = 1;
        }

        if(uart_receive_str.length() >= RECV_SIZE) {
            uart_receive_str = "";
        }
    }
}
```

2. **新的loop_uart()函数**
对于两种新增模式有两种思路:（1）只有servo更新值时，仅仅将新值写入更新便跳出函数；（2）当既有servo值又有陀螺仪测得的值传入时，利用由glove手套板程序改编而来的hand_action()函数，更新uart_receive_buf以及uart1_mode，从而借助原有的程序实现条件判断后的特殊行为。

```cpp
//此外还新定义了一个全局变量用于计算两次接收到servo角度更新的指令之间的间隔时间——即得到servo的struct里的time1——进而得到inc值，最终放到loop_servo()中执行实现角度更新。
u32 servo_uart_Ms=0;//两次舵机串口输入的间隔时间，项目新增%%%%
//解析串口接收到的字符串指令
void loop_uart(){
    if(uart1_get_ok) {
    	  //如果有同步标志直接返回，让同步函数处理
        if(flag_sync)return;
        //打印字符串，测试的时候可以用
        //转换成字符串数组
        uart_receive_str.toCharArray(uart_receive_buf, uart_receive_str.length()+1);
        uart_receive_str_len = uart_receive_str.length();
        //servo_input_flag == 1时，简单写入舵机值
        if(servo_input_flag == 1){
          servo_input_flag = 0;
          int data1,data2,data3;
          if (sscanf(uart_receive_buf, "[%d!%d!%d!]", &data1, &data2, &data3) == 3){
            //取三者中值
            int servo_new = findMedian(data1, data2, data3);
            //舵机值转换
            servo_new = int(map(servo_new, 800, 4000, 500, 2500));
            int time1 = mills() - servo_uart_Ms;
            servo_uart_Ms = mills();
            if(time1 > 1000) return;//对于可能的第一次发送值时带来的错误值滤除保证mills()-suM的正确性
            //舵机值赋值，只赋予五个手指的servo
            for(byte i = 1; i < SERVO_NUM; i ++){
              servo_do[i].aim = servo_new;
              servo_do[i].time1 = time1;
              servo_do[i].inc = (servo_do[i].aim - servo_do[i].cur) / (time1 / SERVO_TIME_PERIOD);
            }
          }
          return;
        }
        //gyro_input_flag == 1时，简单写入陀螺仪值与舵机值
        if(gyro_input_flag == 1){
          gyro_input_flag = 0;
          int data1,data2,data3,data4,data5;
          if (sscanf(uart_receive_buf, "*%d!%d!%d!%d!%d*", &data1, &data2, &data3, &data4, &data5) == 5){
            //取三者中值
            int servo_new = findMedian(data1, data2, data3);
            //舵机值转换
            servo_new = int(map(servo_new, 800, 4000, 500, 2500));
            int time1 = mills() - servo_uart_Ms;
            servo_uart_Ms = mills();
            if(time1 > 1000) return;//对于可能的第一次发送值时带来的错误值滤除保证mills()-suM的正确性
            //舵机值赋值
            for(byte i = 1; i < SERVO_NUM; i ++){
              servo_do[i].aim = servo_new;
              servo_do[i].time1 = time1;
              servo_do[i].inc = (servo_do[i].aim - servo_do[i].cur) / (time1 / SERVO_TIME_PERIOD);
            }
            //陀螺仪值转换???未完成！！！！仍需测量待定（但原glove程序里其取值范围确实是500-2500，待测的是传输来的pos_xy的取值范围，以及pos_verify没测）
            data4 = int(map(data4, 800, 4000, 500, 2500));
            data5 = int(map(data5, 800, 4000, 500, 2500));
            //导入处理函数
            hand_action(data4, data5);//hand_action函数作用是根据五个手指的servo值与陀螺仪的pos_x,pos_y做出相应的判断——覆写uart_receive_buf，进入下面的uart1_mode == 3的parse_action函数中做出相应的小车控制等等行为
            uart1_mode = 3;//使其进入下面的if判断中，从而执行出对应动作。

          }
          
        }

        if(uart1_mode == 1) {
            parse_cmd(uart_receive_buf);
        } else if(uart1_mode == 2 || uart1_mode == 3){
            parse_action(uart_receive_buf);
        } else if(uart1_mode == 4){
            save_action(uart_receive_buf);
        }
        uart1_get_ok = 0;
        uart1_mode = 0;
        uart_receive_str = "";
    }
    if(millis()-downLoadSystickMs>3000) {
    	downLoad = false;
    }
}

```

3. **hand_action()函数**
   简单的将原来输出到cmd的值改为更新为uart_receive_buf，同时加了一些传参操作。

```cpp
//原手套板程序，项目新增%%%%大修了
void hand_action(int pos_x, int pos_y ){    
    static u32 knob_value, curMode = 0;
    static u8 first_in = 1;
        //Serial.print("pos_x:");
        //Serial.print(pos_x);
        pos_x_bak = pos_x;
        pos_x = pos_x-pos_x_verify;
        //Serial.print(" pos_x2:");
        //Serial.println(pos_x);
        pos_x = 1500 - pos_x*4;
        pos_x = 3000-pos_x;
        
        if(pos_x>2500)pos_x=2500;
        else if(pos_x<500)pos_x=500; 
        
        //Serial.print("pos_y:");
        //Serial.print(pos_y);
        pos_y_bak = pos_y;
        pos_y = pos_y-pos_y_verify;
        //Serial.print(" pos_y2:");
        //Serial.println(pos_y);
        pos_y = 1500 - pos_y*6;
        
        if(pos_y>2500)pos_y=2500;
        else if(pos_y<500)pos_y=500;  
        for(byte i = 0; i < SERVO_NUM; i ++){
              servo_do[i].cur = pwm_value[i];
              
            }
        //手指数据进行更改
        if((pwm_value[0]>1800) &&
        (pwm_value[1]<1200) &&
        (pwm_value[2]<1200) &&
        (pwm_value[3]>1800) &&
        (pwm_value[4]>1800) 
         ) {//闭合手控制
            if(abs(pos_x - pos_x_bak) >= 5 || abs(pos_y - pos_y_bak) >= 5) {
                                //sprintf(cmd_return,"{#000P%04dT0500!#001P1900T0500!#002P1100T0500!#003P1100T0500!#004P1900T0500!#005P1900T0500!}", pos_x);
                //                 
                
                //云台及小车控制
                if(pos_y>2000 && abs(pos_x-1500)<500) {
                    if(curMode != 1) {
                        curMode = 1;
                        sprintf(uart_receive_buf,"{#006P2500T0000!#007P0500T0000!}"); 
                        
                        
                    } 
                } else if(pos_y<1000 && abs(pos_x-1500)<500) {
                    if(curMode != 2) {
                        curMode = 2;
                        sprintf(uart_receive_buf,"{#006P0500T0000!#007P2500T0000!}"); 
                        
                        
                }
                } else if(pos_x>2000) {
                    if(curMode != 3) {
                        curMode = 3;  
                        sprintf(uart_receive_buf,"{#006P2400T0000!#007P2400T0000!}");
                        
                        
                    }   
                } else if(pos_x<1000) {
                    if(curMode != 4) {
                        curMode = 4;
                        sprintf(uart_receive_buf,"{#006P0600T0000!#007P0600T0000!}"); 
                        
                        
                    }
                } else {
                    if(curMode != 5) {
                        curMode = 5;
                        sprintf(uart_receive_buf,"{#006P1500T0000!#007P1500T0000!#000P1500T1500!#001P1000T1500!#002P2000T1500!#003P2000T1500!#004P2000T1500!#005P2000T1500!}"); 
                        
                    }
                }
                
            }        
        } else  if((pwm_value[0]<1200) &&
        (pwm_value[1]>1800) &&
        (pwm_value[2]>1800) &&
        (pwm_value[3]<1200) &&
        (pwm_value[4]<1200) 
        ) {//张开手控制 通过陀螺仪Y控制0号舵机前倾后仰
            if(pos_y>1800)pos_y=1800;
            else if(pos_y<1200)pos_y=1200;
            if(abs(pos_y - pos_y_bak) >= 5) {     
                pos_y_bak = pos_y;           
                sprintf(uart_receive_buf,"{#000P%04dT1000!#001P1900T1500!#002P1100T1500!#003P1100T1500!#004P1100T1500!#005P1100T1500!}", 3000-pos_y);
                if(first_in) {
                    //判断手是否放平
                    if(pos_y>1450 && pos_y<1550) {
                        first_in = 0;
                        beep_on();delay(200);beep_off();
                    } else {
                        Serial.println("{#000P1500T1500!#001P1900T1500!#002P1100T1500!#003P1100T1500!#004P1100T1500!#005P1100T1500!}");
                    }
                } else {
                         
                }
            }
        } else {//手指控制
            //停止车
            if(curMode != 0) {
                curMode = 0;
                sprintf(uart_receive_buf,"{#006P1500T0000!#007P1500T0000!}"); 
                
            }
            sprintf(uart_receive_buf,"{#000P1500T1000!#001P%04dT0100!#002P%04dT0100!#003P%04dT0100!#004P%04dT0100!#005P%04dT0100!}", 3000-pwm_value[0],3000-pwm_value[1],3000-pwm_value[2],pwm_value[3],pwm_value[4]);
             
            first_in = 1; 
        }
    }
```


---

## 待改进问题

1. 当前版本程序里（11\23版）对于五个手指的servo更新是统一传入了三个传输值的中值————非常不合理，仅能用于展示弯曲手指。亟需以后拿手套确认三个传输值各自对应的手指。以及具体的映射关系，此版仅仅使用map()函数做了线性映射，以后可以具体调参总结出经验公式。
2. 对于servo[0]即手腕处的servo，以后可以尝试根据传参过来的陀螺仪pos_x\pos_y写一个函数确定（仅仅是设想）。
3. 程序仍欠缺pos_x_verify\pos_y_verify,其实到时候测一测平摊数值时候的陀螺仪数值即可。
4. 我们的陀螺仪数值区间范围待测，从而与程序里的（对应机械手的）[500,2500]实现映射。
5. 其实还有对于servo值进行更新时，对于一些不合理情况的过滤没有写，本版（11\23）程序仅考虑并过滤了第一次发来指令使用servo_uart_Ms时带来的time1过大——从而inc太小——影响后续动作写入的情况。
6. 
