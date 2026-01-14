#define PIN_RGB  A3
#define PIN_hw  A0
#define PIN_xj0  A7
#define PIN_xj1  A6
#define PIN_trig  A1
#define PIN_echo  A2
#define PIN_sound  2
#define PIN_IR  2
//#include <IRremote.h>//红外功能实现库——项目无关

#include <Adafruit_NeoPixel.h>//rgb实现相关——项目无关
#include <Wire.h>
#include <Adafruit_TCS34725.h>//颜色识别库，项目无用，相关函数可注释
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_24MS, TCS34725_GAIN_4X);

volatile int mode;
volatile int mode_bak;
volatile int num;
volatile int num_bak;
volatile int hw_flag;
volatile byte sound_num;
volatile long sound_check_bak;
volatile long sound_check_bak1;
volatile int sound_time;
volatile boolean sound_flg;
volatile long sound_time_L;
volatile int distance;
volatile int distance_bak;
volatile int mode_bs;
volatile int mode_bs_bak;
volatile long baoshu_time;
volatile int flag;
volatile char xj_mode;
int pose[]={0, 0};

int pwm[]={0, 0};
int pwm_value[5]={0, 0, 0, 0, 0};
Adafruit_NeoPixel rgb_display_A3 = Adafruit_NeoPixel(6,A3,NEO_GRB + NEO_KHZ800);
volatile long systick_ms_yanse;
volatile long systick_ms_hw;
volatile long systick_ms_hw_bak;

#include <Servo.h>          //声明调用Servo.h库
#include <winbondflash.h>  //flash调用的库

#define ACTION_SIZE 512
#define RECV_SIZE 168
#define INFO_ADDR_SAVE_STR   (((8<<10)-4)<<10) //(8*1024-4)*1024    //eeprom_info结构体存储的位置
#define BIAS_ADDR_VERIFY  0 //偏差存储的地址
#define FLAG_VERIFY 0x38

#define  PIN_nled   13              //宏定义工作指示灯引脚
#define  PIN_beep   4               //蜂鸣器引脚定义

#define nled_on() {digitalWrite(PIN_nled, LOW);}
#define nled_off() {digitalWrite(PIN_nled, HIGH);}

#define beep_on() {digitalWrite(PIN_beep, HIGH);}
#define beep_off() {digitalWrite(PIN_beep, LOW);}

#define PRE_CMD_SIZE 32
#define SERVO_NUM 6
#define SERVO_TIME_PERIOD    2    //每隔10ms处理一次（累加）舵机的PWM增量

typedef struct {
  long myversion;
  long dj_record_num;
  byte pre_cmd[PRE_CMD_SIZE+1];
  int  dj_bias_pwm[SERVO_NUM+1];
}eeprom_info_t;
eeprom_info_t eeprom_info;

Servo myservo[SERVO_NUM];         //创建一个舵机类
char buffer[RECV_SIZE];                 // 定义一个数组用来存储每小组动作组
byte uart_receive_buf[RECV_SIZE];
byte servo_pin[SERVO_NUM] = {7, 3, 5, 6, 9 ,8}; //宏定义舵机控制引脚
String uart_receive_str = "";    //声明一个字符串数组
String uart_receive_str_bak = "";    //声明一个字符串数组
byte uart1_get_ok = 0, uart1_mode=0;
char cmd_return[64];
int uart_receive_str_len;
int zx_read_id = 0, zx_read_flag = 0, zx_read_value = 0;
byte flag_sync=0;
byte servo_input_flag = 0;//舵机输入标志，项目新增%%%%
byte gyro_input_flag = 0;//陀螺仪输入标志，项目新增%%%%
bool downLoad = false;
u32 downLoadSystickMs=0;
u32 servo_uart_Ms=0;//两次舵机串口输入的间隔时间，项目新增%%%%
u32 cmd_uart_Ms=0;//两次指令串口输入的间隔时间，项目新增%%%%
int  pos_bak[5] = {0}, pos_x_bak=0, pos_y_bak=0;    //变量pos备份值
int pos_x_verify = 0, pos_y_verify = 0;             //x y校验值
typedef struct {                  //舵机结构体变量声明
    unsigned int aim = 1500;      //舵机目标值
    float cur = 1500.0;           //舵机当前值
    unsigned  int time1 = 1000;   //舵机执行时间
    float inc= 0.0;               //舵机值增量，以20ms为周期
}duoji_struct;
duoji_struct servo_do[SERVO_NUM];           //用结构体变量声明一个舵机变量组

winbondFlashSPI mem;//声明flash对象，用于读写flash
int do_start_index, do_time, group_num_start, group_num_end, group_num_times;
char group_do_ok = 1;
long long action_time = 0;
void(* resetFunc) (void) = 0; //declare reset function at address 0

//对 a 数进行排序
void selection_sort(int *a, int len) {
    int i,j,mi,t;
    for(i=0;i<len-1;i++) {
        mi = i;
        for(j=i+1;j<len;j++) {
            if(a[mi] > a[j]) {
                mi = j;
            }
        }
        if(mi != i) {
            t = a[mi];
            a[mi] = a[i];
            a[i] = t;
        }
    }
}
int findMedian(int a, int b, int c) {
    if ((a >= b && a <= c) || (a >= c && a <= b)) {
        return a;
    } else if ((b >= a && b <= c) || (b >= c && b <= a)) {
        return b;
    } else {
        return c;
    }
}
//查询str是包含str2，并返回最后一个字符所在str的位置
u16 str_contain_str(u8 *str, u8 *str2) {
  u8 *str_temp, *str_temp2;
  str_temp = str;
  str_temp2 = str2;
  while(*str_temp) {
    if(*str_temp == *str_temp2) {
      while(*str_temp2) {
        if(*str_temp++ != *str_temp2++) {
          str_temp = str_temp - (str_temp2-str2) + 1;
          str_temp2 = str2;
          break;
        }
      }
      if(!*str_temp2) {
        return (str_temp-str);
      }
    } else {
      str_temp++;
    }
  }
  return 0;
}

//电机控制函数，地盘两个电机分别控制左右轮，从而控制转向和速度
void car_run(short speed_left, short speed_right) {
    Serial.println("car_run:");
    sprintf(cmd_return, "#006P%04dT0000!#007P%04dT0000!", 1500+speed_left, 1500-speed_right);//sprintf函数用于格式化字符串，将格式化的字符串写入某个字符串数组中，这里是将格式化的字符串写入cmd_return数组中
    Serial.println((char *)cmd_return);
    sprintf(cmd_return, "#008P%04dT0000!#009P%04dT0000!", 1500+speed_left, 1500-speed_right);
    Serial.println((char *)cmd_return);
    sprintf(cmd_return, "#000P%04dT0000!", 1500-(speed_left-speed_right)/2);
    Serial.println((char *)cmd_return);
}

//led灯初始化 这个不用注释掉，因为初始化完成标志要用到
void setup_nled() {
    pinMode(PIN_nled,OUTPUT); //设置引脚为输出模式
    nled_off();
}

//蜂鸣器初始化 //##
void setup_beep() {
    pinMode(PIN_beep,OUTPUT);
    beep_off();
}

//存储器初始化 eeprom库是用来存储数据的，这里用来存储舵机偏差值
void setup_w25q() {
	read_eeprom();
	if(eeprom_info.dj_bias_pwm[SERVO_NUM] != FLAG_VERIFY) {
		for(int i=0;i<SERVO_NUM; i++) {
			eeprom_info.dj_bias_pwm[i] = 0;
		}
		eeprom_info.dj_bias_pwm[SERVO_NUM] = FLAG_VERIFY;
	}
}

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

//启动提示
void setup_start_pre_cmd() {
	if(eeprom_info.pre_cmd[PRE_CMD_SIZE] == FLAG_VERIFY) {
	      parse_cmd(eeprom_info.pre_cmd);
	}
}

//功能介绍：LED灯闪烁，每秒闪烁一次
void loop_nled() {
    static u8 val = 0;
    static unsigned long systick_ms_bak = 0;
    if(millis() - systick_ms_bak > 500) {
      systick_ms_bak = millis();
      if(val) {
        nled_on();
      } else {
        nled_off();
      }
      val = !val;
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
            myservo[i].writeMicroseconds((int)servo_do[i].cur);
          }
      }
  }
}

/*//串口中断
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
}*/

//新的串口中断函数
//“[4096!4096!4096]”型为三个手指的电位值，“*4096!4096!4096!111!111*”型为舵机电位值+陀螺仪的值
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
            cmd_uart_Ms = millis();
            gyro_input_flag = 1;
            uart1_mode = 3;
          } else if(sbuf_bak == '$') {
            uart1_mode = 1;
          } else if(sbuf_bak == '#') {
            uart1_mode = 2;
          } else if(sbuf_bak == '[') {

            uart1_mode = 5;
            servo_input_flag = 1;
          } else if(sbuf_bak == '*') {
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

//若200ms内没有接收到串口指令，则执行此函数自动停止小车运动。
//cmd_uart_Ms只会在收到小车{指令}时更新
void loop_cmd() {
    if(millis() - cmd_uart_Ms > 200) {
      cmd_uart_Ms = millis();
      do_group_once(91);//停止指令

    }
}

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
          //此时输入了[4096!4096!4096!4096!4096]型的字符串，代表五个手指的电位值，需要将其转换为舵机值，覆写到servo_do数组中
          int data[5];
          if (sscanf(uart_receive_buf, "[%d!%d!%d!%d!%d]", &data[0], &data[1], &data[2], &data[3], &data[4]) == 5){
            int time1 = millis() - servo_uart_Ms;
            servo_uart_Ms = millis();
            //舵机值赋值，只赋予五个手指的servo
            for(byte i = 1; i < SERVO_NUM; i ++){
              servo_do[i].aim = int(map(data[i-1], 800, 4000, 500, 2500));//建议输入范围在1000-3200之间，这样就可以避免出现舵机卡住· 
              servo_do[i].time1 = time1;
              servo_do[i].inc = 0.8 * (servo_do[i].aim - servo_do[i].cur) / ( time1 / SERVO_TIME_PERIOD);
              Serial.print(i);
              Serial.print(" inc :");
              Serial.println(servo_do[i].inc);

            }
          }
          return;
        }
        //gyro_input_flag == 1时，简单写入陀螺仪值与舵机值
        /*else if(pos = str_contain_str((char *)uart_receive_buf, "$DGT:"), pos) {
        if(sscanf(uart_receive_buf, "$DGT:%d-%d,%d!", &int1, &int2, &int3)) {
            group_num_start = int1;
            group_num_end = int2;
            group_num_times = int3;
            do_time = int3;
            do_start_index = int1;
			  if(int1 == int2) {
				  do_group_once(int1);
			  } else {
            	  group_do_ok = 0;
			  }*/
        if(gyro_input_flag == 1){
          
          gyro_input_flag = 0;
          int data1;
          sscanf(uart_receive_buf, "{%d}", &data1);
          Serial.print("data1:");
          Serial.println(data1);
          switch(data1){
            case 1://剪刀
              group_num_start = 14;
              group_num_end = 14;
              group_num_times = 1;
              do_time = 1;
              do_start_index = 14;
              break;
            case 2://布
              group_num_start = 16;
              group_num_end = 16;
              group_num_times = 1;
              do_time = 1;
              do_start_index = 16; 
              break;
            case 3://六手势 ruaike？
              group_num_start = 12;
              group_num_end = 12;
              group_num_times = 1;
              do_time = 1;
              do_start_index = 12;
              break;
            case 4://自动演示
              group_num_start = 17;
              group_num_end = 81;
              group_num_times = 1;
              do_time = 1;
              do_start_index = 17;
              break;
            case 5://前进
              group_num_start = 95;
              group_num_end = 95;
              group_num_times = 1;
              do_time = 1;
              do_start_index = 95;
              break;
            case 6://左转
              group_num_start = 97;
              group_num_end = 97;
              group_num_times = 1;
              do_time = 1;
              do_start_index = 97;
              break;
            case 7://停止
              group_num_start = 91;
              group_num_end = 91;
              group_num_times = 1;
              do_time = 1;
              do_start_index = 91;
              break;
            case 8://又转
              group_num_start = 98;
              group_num_end = 98;
              group_num_times = 1;
              do_time = 1;
              do_start_index = 98;
              break;
            case 9://后退
              group_num_start = 96;
              group_num_end = 96;
              group_num_times = 1;
              do_time = 1;
              do_start_index = 96;
              break;
            case 10://一二三四五数数
              group_num_start = 1;
              group_num_end = 10;
              group_num_times = 1;
              do_time = 1;
              do_start_index = 1;
              break;
            case 11: //石头
              group_num_start = 15;
              group_num_end = 15;
              group_num_times = 1;
              do_time = 1;
              do_start_index = 15;
              break;

            }
            if(group_num_start == group_num_end){
              do_group_once(group_num_start);
            }
            else{
              group_do_ok = 0;
            }
            return;
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

//获取最大时间   串口信息处理用到！
int getMaxTime(char *str) {
   int i = 0, max_time = 0, tmp_time = 0;
   while(str[i]) {
      if(str[i] == 'T') {
          tmp_time = (str[i+1]-'0')*1000 + (str[i+2]-'0')*100 + (str[i+3]-'0')*10 + (str[i+4]-'0');
          if(tmp_time>max_time)max_time = tmp_time;
          i = i+4;
          continue;
      }
      i++;
   }
   return max_time;
}

//把eeprom_info写入到W25Q64_INFO_ADDR_SAVE_STR位置
void read_eeprom(void) {
    mem.begin(_W25Q64,SPI,SS);
    mem.read( INFO_ADDR_SAVE_STR, (char *)(&eeprom_info), sizeof(eeprom_info_t));
    mem.end();
}

//把eeprom_info写入到INFO_ADDR_SAVE_STR位置
void rewrite_eeprom(void) {
    mem.begin(_W25Q64,SPI,SS);
    mem.eraseSector(INFO_ADDR_SAVE_STR);
    mem.write( INFO_ADDR_SAVE_STR, (char *)(&eeprom_info), sizeof(eeprom_info_t));
    mem.end();
}

//存储动作组
void save_action(char *str) {
  long long action_index = 0, max_time;
  //预存命令处理
  if(str[1] == '$' && str[2] == '!') {
    eeprom_info.pre_cmd[PRE_CMD_SIZE] = 0;
    rewrite_eeprom();
    Serial.println("@CLEAR PRE_CMD OK!");
    return;
  } else if(str[1] == '$') {
    memset(eeprom_info.pre_cmd, 0, sizeof(eeprom_info.pre_cmd));
    strcpy((char *)eeprom_info.pre_cmd, (char *)str+1);
    eeprom_info.pre_cmd[strlen((char *)str) - 2] = '\0';
    eeprom_info.pre_cmd[PRE_CMD_SIZE] = FLAG_VERIFY;
    rewrite_eeprom();
    Serial.println("@SET PRE_CMD OK!");
    Serial.println((char *)eeprom_info.pre_cmd);
    return;
  }
  //<G0000#000P1500T1000!>
  if((str[1] == 'G') && (str[6] == '#')) {
      action_index = (str[2]-'0')*1000 + (str[3]-'0')*100 + (str[4]-'0')*10 + (str[5]-'0') ;
      if(action_index<10000) {
        str[0] = '{';
        str[uart_receive_str_len-1] = '}';
        max_time = getMaxTime(str);
        uart_receive_str_len = uart_receive_str_len+4;
        str[uart_receive_str_len-4] = max_time/1000 + '0';
        str[uart_receive_str_len-3] = max_time%1000/100 + '0';
        str[uart_receive_str_len-2] = max_time%100/10 + '0';
        str[uart_receive_str_len-1] = max_time%10 + '0';
        str[uart_receive_str_len] = '\0';
        mem.begin(_W25Q64,SPI,SS);
        if((action_index*ACTION_SIZE % 4096) == 0){
            mem.eraseSector(action_index*ACTION_SIZE);	delay(5);
        }
        if(uart_receive_str_len<256) {
            mem.write((long long)action_index*ACTION_SIZE, str, uart_receive_str_len);
        } else {
            mem.write((long long)action_index*ACTION_SIZE, str, 256);
        	  delay(5);
            mem.write((long long)action_index*ACTION_SIZE+256, str+256, uart_receive_str_len-256);
        }
        mem.end();
        //返回发送接收成功
        delay(30);
        Serial.println("A");
      }
  }
}

//解析串口动作
void parse_action(u8 *uart_receive_buf) {
    static unsigned int index, time1, pwm1, pwm2, i, len;//声明三个变量分别用来存储解析后的舵机序号，舵机执行时间，舵机PWM
    if((uart_receive_buf[0] == '#') && (uart_receive_buf[4] == 'P') && (uart_receive_buf[5] == '!')) {
        delay(500);
    }
    Serial.println((char *)uart_receive_buf);
    if(zx_read_flag) {
  	  //#001P1500! 回读处理
  	  if((uart_receive_buf[0] == '#') && (uart_receive_buf[4] == 'P') && (uart_receive_buf[9] == '!')) {
    	    index = (uart_receive_buf[1]-'0')*100 +  (uart_receive_buf[2]-'0')*10 +  (uart_receive_buf[3]-'0');
			  if(index == zx_read_id) {
 				  zx_read_flag = 0;
      		  zx_read_value = (uart_receive_buf[5]-'0')*1000 + (uart_receive_buf[6]-'0')*100 +  (uart_receive_buf[7]-'0')*10 +  (uart_receive_buf[8]-'0');
			  }
		}
    //#001PSCK+100! 偏差处理
    } else if((uart_receive_buf[0] == '#') && (uart_receive_buf[4] == 'P') && (uart_receive_buf[5] == 'S') && (uart_receive_buf[6] == 'C') && (uart_receive_buf[7] == 'K')) {
        index = (uart_receive_buf[1]-'0')*100 +  (uart_receive_buf[2]-'0')*10 +  (uart_receive_buf[3]-'0');
        if(index < SERVO_NUM) {
            int bias_tmp = (uart_receive_buf[9]-'0')*100 +  (uart_receive_buf[10]-'0')*10 +  (uart_receive_buf[11]-'0');
            if(bias_tmp < 127) {
            myservo[index].attach(servo_pin[index]);
              if(uart_receive_buf[8] == '+') {
                  servo_do[index].cur = servo_do[index].cur-eeprom_info.dj_bias_pwm[index]+bias_tmp;
                  eeprom_info.dj_bias_pwm[index] = bias_tmp;
              } else if(uart_receive_buf[8] == '-') {
                  servo_do[index].cur = servo_do[index].cur-eeprom_info.dj_bias_pwm[index]-bias_tmp;
                  eeprom_info.dj_bias_pwm[index] = -bias_tmp;
              }
              rewrite_eeprom();
				servo_do[index].cur = 1500;
				servo_do[index].aim = 1500+eeprom_info.dj_bias_pwm[index]; //舵机PWM赋值,加上偏差的值
				servo_do[index].time1 = 100;      //舵机执行时间赋值
				servo_do[index].inc = eeprom_info.dj_bias_pwm[index]/5.000; //根据时间计算舵机PWM增量
              //Serial.print("input bias:");
              //Serial.println(eeprom_info.dj_bias_pwm[index]);
           }
        }
    //停止处理
    } else if((uart_receive_buf[0] == '#') && (uart_receive_buf[4] == 'P') && (uart_receive_buf[5] == 'D') && (uart_receive_buf[6] == 'S') && (uart_receive_buf[7] == 'T')) {
        index = (uart_receive_buf[1]-'0')*100 +  (uart_receive_buf[2]-'0')*10 +  (uart_receive_buf[3]-'0');
        if(index < SERVO_NUM) {
              servo_do[index].inc =  0.001;
              servo_do[index].aim = servo_do[index].cur;
        }
    } else if((uart_receive_buf[0] == '#') || (uart_receive_buf[0] == '{')) {   //解析以“#”或者以“{”开头的指令
        len = strlen(uart_receive_buf);     //获取串口接收数据的长度
        index=0; pwm1=0; time1=0;           //3个参数初始化
        for(i = 0; i < len; i++) {          //
            if(uart_receive_buf[i] == '#') {        //判断是否为起始符“#”
                i++;                        //下一个字符
                while((uart_receive_buf[i] != 'P') && (i<len)) {     //判断是否为#之后P之前的数字字符
                    index = index*10 + (uart_receive_buf[i] - '0');  //记录P之前的数字
                    i++;
                }
                i--;                          //因为上面i多自增一次，所以要减去1个
            } else if(uart_receive_buf[i] == 'P') {   //检测是否为“P”
                i++;
                while((uart_receive_buf[i] != 'T') && (i<len)) {  //检测P之后T之前的数字字符并保存
                    pwm1 = pwm1*10 + (uart_receive_buf[i] - '0');
                    i++;
                }
                i--;
            } else if(uart_receive_buf[i] == 'T') {  //判断是否为“T”
                i++;
                while((uart_receive_buf[i] != '!') && (i<len)) {//检测T之后!之前的数字字符并保存
                    time1 = time1*10 + (uart_receive_buf[i] - '0'); //将T后面的数字保存
                    i++;
                }
                if(time1<SERVO_TIME_PERIOD)time1=SERVO_TIME_PERIOD;//很重要，防止被除数为0
                if((index == 255) && (pwm1 >= 500) && (pwm1 <= 2500) && (time1<10000)) {  //如果舵机号和PWM数值超出约定值则跳出不处理
						for(int i=0;i<SERVO_NUM;i++) {
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
                }
                index = pwm1 = time1 = 0;
            }
        }
    }
}

//解析串口指令
void parse_cmd(u8 *uart_receive_buf) {
    static u8 jxb_r_flag = 0;
    int int1,int2,int3,int4;
    u16 pos;
    if(pos = str_contain_str((char *)uart_receive_buf, "$DRS!"), pos) {
        Serial.println("$DRS!");
    } else if(pos = str_contain_str((char *)uart_receive_buf, "$RST!"), pos) {
        resetFunc();
    } else if(pos = str_contain_str((char *)uart_receive_buf, "$DST!"), pos) {
        Serial.println("#255PDST!");
        for(int i;i<SERVO_NUM;i++) {
          servo_do[i].aim = servo_do[i].cur;
        }
        group_do_ok  = 1;
        Serial.println("@DoStop!");
    }else if(pos = str_contain_str((char *)uart_receive_buf, "$DST:"), pos) {
      if(sscanf(uart_receive_buf, "$DST:%d!", &int1)) {
        	servo_do[int1].aim = servo_do[int1].cur;
		}
      Serial.println("@DoStop!");
    }else if(pos = str_contain_str((char *)uart_receive_buf, "$BON!"), pos) {
      beep_on();
    }else if(pos = str_contain_str((char *)uart_receive_buf, "$BOFF!"), pos) {
      beep_off();
    } else if(sscanf(uart_receive_buf, "$DCR:%d,%d!", &int2, &int3)) {
        car_run(int2,int3);
    } else if(pos = str_contain_str((char *)uart_receive_buf, "$PTG:"), pos) {
        if(sscanf(uart_receive_buf, "$PTG:%d-%d!", &int1, &int2)) {
            Serial.println(F("Your Action:"));
            mem.begin(_W25Q64,SPI,SS);
  		  for(int i=int1; i<=int2; i++) {
            mem.read((unsigned long)i*ACTION_SIZE, uart_receive_buf, RECV_SIZE-1);
 			  if(uart_receive_buf[0] == '{' && uart_receive_buf[1] == 'G') {
					Serial.println((char *)uart_receive_buf);
			  } else {
 					sprintf(uart_receive_buf, "@NoGroup %d!", i);
  			Serial.println((char *)uart_receive_buf);
    		  }
    		  delay(10);
         }
         mem.end();
       }
    } else if(pos = str_contain_str((char *)uart_receive_buf, "$DGT:"), pos) {
        if(sscanf(uart_receive_buf, "$DGT:%d-%d,%d!", &int1, &int2, &int3)) {
            group_num_start = int1;
            group_num_end = int2;
            group_num_times = int3;
            do_time = int3;
            do_start_index = int1;
			  if(int1 == int2) {
				  do_group_once(int1);
			  } else {
            	  group_do_ok = 0;
			  }
        	}
    }
	else if(pos = str_contain_str((char *)uart_receive_buf, "$GETA!"), pos) {
        downLoad = true;
        downLoadSystickMs = millis();
        Serial.println(F("A"));
    }  else {
    }
}

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
        for(byte i = 1; i < SERVO_NUM; i ++){
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
    




//循环执行动作组
void loop_action() {
  static long long systick_ms_bak = 0;
    if(group_do_ok == 0) {
      //循环检测单个动作执行时间是否到位
      if(millis() - systick_ms_bak > action_time) {
        systick_ms_bak =  millis();
        //Serial.println(do_start_index);
 			if(group_num_times != 0 && do_time == 0) {
           group_do_ok = 1;
          Serial.println("@GroupDone!");
          return;
        }
        do_group_once(do_start_index);
          if(group_num_start<group_num_end) {
            if(do_start_index == group_num_end) {
              do_start_index = group_num_start;
              if(group_num_times != 0) {
                do_time--;
              }
              return;
            }
            do_start_index++;
          } else {
            if(do_start_index == group_num_end) {
              do_start_index = group_num_start;
              if(group_num_times != 0) {
                do_time--;
              }
              return;
            }
            do_start_index--;
          }
      }
  } else {
      action_time = 10;
  }
}

//执行动作组单次
void do_group_once(int index) {
  static long long systick_ms_bak = 0;
  int len;
  //读取动作
  mem.begin(_W25Q64,SPI,SS);
  mem.read((unsigned long)index*ACTION_SIZE, uart_receive_buf, RECV_SIZE-1);

  //获取时间
	action_time = getMaxTime(uart_receive_buf);
  parse_action(uart_receive_buf);
  mem.end();
}

//执行动作组多次 只适用于总线舵机
void do_groups_times(int st, int ed, int tm) {
	int myst = st, myed = ed, mytm = tm;
	if(tm<=0)return;
	while(mytm) {
		for(int i=myst;i<=myed;i++) {
			do_group_once(i);
			delay(action_time);
		}
		mytm --;
		if(mytm) {
			myst = st;
			myed = ed;
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
}

;
char cmd_return_tmp[64];

void setup_AI() {
  pinMode(A0, INPUT_PULLUP);
  pinMode(PIN_sound, INPUT_PULLUP);
}

//IRrecv irrecv_PIN_IR(PIN_IR);
//decode_results results_PIN_IR;
// 解析 $MODE00! 到 $MODE10!
void AI_parse_cmd() {
  if (String(uart_receive_str_bak).length() > 2) {
    mode = (String(uart_receive_str_bak).charAt(5) - '0') * 10 + (String(uart_receive_str_bak).charAt(6) - '0');
    uart_receive_str_bak = ""; //数据清零

  }
}
unsigned long irValue;
//##
/*void loop_IR() {
  if (irrecv_PIN_IR.decode(&results_PIN_IR)) {
    irValue= results_PIN_IR.value;
    // 自动演示
    if (irValue == 0xFFA25D) {
      sprintf(cmd_return, "$DGT:%d-%d,%d!", 15, 15, 1); //解析动作组
      	parse_cmd(cmd_return); //解析动作组

    } else if (irValue == 0xFF629D) {
      sprintf(cmd_return, "$DGT:%d-%d,%d!", 14, 14, 1); //解析动作组
      	parse_cmd(cmd_return); //解析动作组
    } else if (irValue == 0xFFE21D) {
      sprintf(cmd_return, "$DGT:%d-%d,%d!", 16, 16, 1); //解析动作组
      	parse_cmd(cmd_return); //解析动作组
    } else if (irValue == 0xFF22DD) {
      sprintf(cmd_return, "$DGT:%d-%d,%d!", 17, 81, 1); //解析动作组
      	parse_cmd(cmd_return); //解析动作组
    } else if (irValue == 0xFF02FD) {
      sprintf(cmd_return, "$DGT:%d-%d,%d!", 95, 95, 1); //解析动作组
      	parse_cmd(cmd_return); //解析动作组
    } else if (irValue == 0xFFC23D) {
      sprintf(cmd_return, "$DGT:%d-%d,%d!", 12, 12, 1); //解析动作组
      	parse_cmd(cmd_return); //解析动作组
    } else if (irValue == 0xFFE01F) {
      sprintf(cmd_return, "$DGT:%d-%d,%d!", 97, 97, 1); //解析动作组
      	parse_cmd(cmd_return); //解析动作组
    } else if (irValue == 0xFFA857) {
      mode = 255;
      sprintf(cmd_return, "$DGT:%d-%d,%d!", 91, 91, 1); //解析动作组
      	parse_cmd(cmd_return); //解析动作组
    } else if (irValue == 0xFF906F) {
      sprintf(cmd_return, "$DGT:%d-%d,%d!", 98, 98, 1); //解析动作组
      	parse_cmd(cmd_return); //解析动作组
    } else if (irValue == 0xFF6897) {
      mode = 1;
    } else if (irValue == 0xFF9867) {
      mode = 11;
      sprintf(cmd_return, "$DGT:%d-%d,%d!", 96, 96, 1); //解析动作组
      	parse_cmd(cmd_return); //解析动作组
    } else if (irValue == 0xFFB04F) {
      sprintf(cmd_return, "$DGT:%d-%d,%d!", 1, 10, 1); //解析动作组
      	parse_cmd(cmd_return); //解析动作组
    } else if (irValue == 0xFF30CF) {
      mode = 2;
    } else if (irValue == 0xFF18E7) {
      mode = 3;
    } else if (irValue == 0xFF7A85) {
      mode = 4;
    } else if (irValue == 0xFF10EF) {
      mode = 5;
    } else if (irValue == 0xFF38C7) {
      delay(2000);
      mode = 6;
    } else if (irValue == 0xFF5AA5) {
      mode = 7;
    } else if (irValue == 0xFF42BD) {
      mode = 8;
    } else if (irValue == 0xFF4AB5) {
      sprintf(cmd_return_tmp, "{#%03dP%04dT%04d!#%03dP%04dT%04d!}",6,1500+300,0,7,1500-300,0); //组合指令
      Serial.println(cmd_return_tmp); //解析ZMotorD指令
      mode = 9;
    } else if (irValue == 0xFF52AD) {
      mode = 10;
      	parse_cmd("10"); //解析命令指令
    }
    irrecv_PIN_IR.resume();
  } else {
    return;

  }
}


//##
void whbianse() {
  if (digitalRead(A0) == LOW) {
    delay(100);
    while (digitalRead(A0) == LOW) {
    }
    num = num + 1;
    if (num > 3) {
      num = 1;

    }

  }
  if (num != num_bak) {
    num_bak = num;
    switch (num) {
     case 1:
      for (int i = 1; i <= 6; i = i + (1)) {
        rgb_display_A3.setPixelColor((i)-1, (0xff0000));
      }
      rgb_display_A3.show();
      break;
     case 2:
      for (int i = 1; i <= 6; i = i + (1)) {
        rgb_display_A3.setPixelColor((i)-1, (0x009900));
      }
      rgb_display_A3.show();
      break;
     case 3:
      for (int i = 1; i <= 6; i = i + (1)) {
        rgb_display_A3.setPixelColor((i)-1, (0x3333ff));
      }
      rgb_display_A3.show();
      break;
    }

  }
}
//##
void AI_yanse_shibie() {
  if (group_do_ok==1 && millis() - systick_ms_yanse > 200) {
    systick_ms_yanse = millis();
    	tcs.setInterrupt(false);
    if (tcs.getC() < 2) {
      	tcs.setInterrupt(true);
      if (tcs.getR() > tcs.getG() && tcs.getR() > tcs.getB()) {
        sprintf(cmd_return, "$DGT:%d-%d,%d!", 111, 112, 3); //解析动作组
        	parse_cmd(cmd_return); //解析动作组
        for (int i = 1; i <= 6; i = i + (1)) {
          rgb_display_A3.setPixelColor((i)-1, (0xff0000));
          rgb_display_A3.show();
        }

      } else if (tcs.getG() > tcs.getR() && tcs.getG() > tcs.getB()) {
        for (int i = 1; i <= 6; i = i + (1)) {
          rgb_display_A3.setPixelColor((i)-1, (0x006600));
        }
        rgb_display_A3.show();
      } else if (tcs.getB() > tcs.getR() && tcs.getB() > tcs.getG()) {
        for (int i = 1; i <= 6; i = i + (1)) {
          rgb_display_A3.setPixelColor((i)-1, (0x3333ff));
        }
        rgb_display_A3.show();
      }

    }

  }
}

void loop_AI() {
  if (mode_bak != mode) {
    mode_bak = mode;
    if (mode >= 0 && mode <= 10) {
      sprintf(cmd_return_tmp, "#%03dP%04dT%04d!",8,1500+(19 + mode),0); //组合指令
      Serial.println(cmd_return_tmp); //解析ZMp3指令

    }

  }
  switch (mode) {
   case 1:
    whbianse();
    break;
   case 2:
    AI_yanse_shibie();
    break;
   case 3:
    hwyanshi();
    break;
   case 4:
    soundshitou();
    break;
   case 5:
    soundRGB();
    break;
   case 6:
    soundbaoshu();
    break;
   case 7:
    cjwq();
    break;
   case 8:
    cejubaoshu();
    break;
   case 9:
    cjbz();
    break;
   case 10:
    xunji();
    break;
   default:
    break;
  }
}
//##
void hwyanshi() {
  if (millis() - systick_ms_hw_bak > systick_ms_hw) {
    systick_ms_hw_bak = millis();
    systick_ms_hw = 200;
    if (digitalRead(A0) == LOW) {
      if (digitalRead(A0) == LOW) {
        systick_ms_hw = 2000;
        if (hw_flag == true) {
          sprintf(cmd_return, "$DGT:%d-%d,%d!", 17, 81, 1); //解析动作组
          	parse_cmd(cmd_return); //解析动作组
          hw_flag = false;

        } else {
          	parse_cmd("$DST!"); //停止动作
          sprintf(cmd_return, "$DGT:%d-%d,%d!", 84, 84, 1); //解析动作组
          	parse_cmd(cmd_return); //解析动作组
          hw_flag = true;

        }

      }

    }

  }
}
//##
void soundshitou() {
  if (millis() - sound_check_bak1 > sound_time) {
    sound_time = 0;
    sound_check_bak1 = millis();
    if (digitalRead(2) == LOW) {
      sound_check_bak = millis();
      while (millis() - sound_check_bak < 1000) {
        if (digitalRead(2) == LOW) {
          sound_num = sound_num + 1;
          delay(100);

        }
      }

    }

  }
  if (sound_num == 1) {
    sound_num = 0;
    sprintf(cmd_return, "$DGT:%d-%d,%d!", 15, 15, 1); //解析动作组
    	parse_cmd(cmd_return); //解析动作组
    sound_time = 2000;

  } else if (sound_num == 2) {
    sound_num = 0;
    sprintf(cmd_return, "$DGT:%d-%d,%d!", 14, 14, 1); //解析动作组
    	parse_cmd(cmd_return); //解析动作组
    sound_time = 2000;
  } else if (sound_num == 3) {
    sound_num = 0;
    sprintf(cmd_return, "$DGT:%d-%d,%d!", 16, 16, 1); //解析动作组
    	parse_cmd(cmd_return); //解析动作组
    sound_time = 2000;
  } else {
    sound_num = 0;

  }
}*/

float checkdistance_A1_A2() {
  digitalWrite(A1, LOW);
  delayMicroseconds(2);
  digitalWrite(A1, HIGH);
  delayMicroseconds(10);
  digitalWrite(A1, LOW);
  float distance = pulseIn(A2, HIGH) / 58.00;
  delay(10);
  return distance;
}
//##
// 测距弯曲
void cjwq() {
  distance = checkdistance_A1_A2();
  if (distance >= 5 && distance <= 33) {
    if (distance != distance_bak) {
      distance_bak = distance;
      pwm[(int)(0)] = (map(distance, 5, 33, 1000, 2000));
      pwm[(int)(1)] = (map(distance, 5, 33, 2000, 1000));
      	sprintf(cmd_return, "#%03dP%04dT%04d!", 1, pwm[(int)(1)], 100); //解析动作组
      	parse_action(cmd_return); //解析动作组
      	sprintf(cmd_return, "#%03dP%04dT%04d!", 2, pwm[(int)(0)], 100); //解析动作组
      	parse_action(cmd_return); //解析动作组
      	sprintf(cmd_return, "#%03dP%04dT%04d!", 3, pwm[(int)(0)], 100); //解析动作组
      	parse_action(cmd_return); //解析动作组
      	sprintf(cmd_return, "#%03dP%04dT%04d!", 4, pwm[(int)(0)], 100); //解析动作组
      	parse_action(cmd_return); //解析动作组
      	sprintf(cmd_return, "#%03dP%04dT%04d!", 5, pwm[(int)(0)], 100); //解析动作组
      	parse_action(cmd_return); //解析动作组

    }

  }
}

boolean mixly_digitalRead(uint8_t pin) {
  pinMode(pin, INPUT);
  boolean _return =  digitalRead(pin);
  pinMode(pin, OUTPUT);
  return _return;
}
//##
void soundbaoshu() {
  if (mixly_digitalRead(PIN_sound) == false) {
    if (mixly_digitalRead(PIN_sound) == false) {
      if (sound_flg == true) {
        sprintf(cmd_return, "$DGT:%d-%d,%d!", 1, 10, 1); //解析动作组
        	parse_cmd(cmd_return); //解析动作组
        sound_flg = false;
        delay(1000);
        sound_time_L = millis();

      }

    }

  }
  if (mixly_digitalRead(PIN_hw) == false) {
    delay(10);
    if (mixly_digitalRead(PIN_hw) == false) {
      	parse_cmd("$DST!"); //停止动作
      delay(100);
      sprintf(cmd_return, "$DGT:%d-%d,%d!", 84, 84, 1); //解析动作组
      	parse_cmd(cmd_return); //解析动作组
      sound_flg = true;

    }
    delay(1000);

  }
  if (millis() - sound_time_L >= 10000) {
    sound_flg = true;

  }
}
//##
void soundRGB() {
  if (digitalRead(2) == LOW) {
    num = num + 1;
    delay(100);
    if (num > 3) {
      num = 1;

    }

  }
  if (num != num_bak) {
    num_bak = num;
    switch (num) {
     case 1:
      for (int i = 1; i <= 6; i = i + (1)) {
        rgb_display_A3.setPixelColor((i)-1, (0xff0000));
      }
      break;
     case 2:
      for (int i = 1; i <= 6; i = i + (1)) {
        rgb_display_A3.setPixelColor((i)-1, (0x009900));
      }
      break;
     case 3:
      for (int i = 1; i <= 6; i = i + (1)) {
        rgb_display_A3.setPixelColor((i)-1, (0x3333ff));
      }
      break;
    }
    rgb_display_A3.show();

  }
}
//##
void cejubaoshu() {
  distance = checkdistance_A1_A2();
  if (distance != distance_bak) {
    if (distance >= 5 && distance <= 6) {
      mode_bs = 1;

    } else if (distance >= 8 && distance <= 9) {
      mode_bs = 2;
    } else if (distance >= 11 && distance <= 12) {
      mode_bs = 3;
    } else if (distance >= 14 && distance <= 15) {
      mode_bs = 4;
    } else if (distance >= 17 && distance <= 18) {
      mode_bs = 5;
    } else if (distance >= 20 && distance <= 21) {
      mode_bs = 6;
    } else if (distance >= 23 && distance <= 24) {
      mode_bs = 7;
    } else if (distance >= 26 && distance <= 27) {
      mode_bs = 8;
    } else if (distance >= 29 && distance <= 30) {
      mode_bs = 9;
    } else if (distance >= 32 && distance <= 33) {
      mode_bs = 10;
    } else {

    }

  }
  if (millis() - baoshu_time > 1000) {
    baoshu_time = millis();
    if (mode_bs != mode_bs_bak) {
      if (mode_bs == 1) {
        sprintf(cmd_return, "$DGT:%d-%d,%d!", 1, 1, 1); //解析动作组
        	parse_cmd(cmd_return); //解析动作组

      } else if (mode_bs == 2) {
        sprintf(cmd_return, "$DGT:%d-%d,%d!", 2, 2, 1); //解析动作组
        	parse_cmd(cmd_return); //解析动作组
      } else if (mode_bs == 3) {
        sprintf(cmd_return, "$DGT:%d-%d,%d!", 3, 3, 1); //解析动作组
        	parse_cmd(cmd_return); //解析动作组
      } else if (mode_bs == 4) {
        sprintf(cmd_return, "$DGT:%d-%d,%d!", 4, 4, 1); //解析动作组
        	parse_cmd(cmd_return); //解析动作组
      } else if (mode_bs == 5) {
        sprintf(cmd_return, "$DGT:%d-%d,%d!", 5, 5, 1); //解析动作组
        	parse_cmd(cmd_return); //解析动作组
      } else if (mode_bs == 6) {
        sprintf(cmd_return, "$DGT:%d-%d,%d!", 6, 6, 1); //解析动作组
        	parse_cmd(cmd_return); //解析动作组
      } else if (mode_bs == 7) {
        sprintf(cmd_return, "$DGT:%d-%d,%d!", 7, 7, 1); //解析动作组
        	parse_cmd(cmd_return); //解析动作组
      } else if (mode_bs == 8) {
        sprintf(cmd_return, "$DGT:%d-%d,%d!", 8, 8, 1); //解析动作组
        	parse_cmd(cmd_return); //解析动作组
      } else if (mode_bs == 9) {
        sprintf(cmd_return, "$DGT:%d-%d,%d!", 9, 9, 1); //解析动作组
        	parse_cmd(cmd_return); //解析动作组
      } else if (mode_bs == 10) {
        sprintf(cmd_return, "$DGT:%d-%d,%d!", 10, 10, 1); //解析动作组
        	parse_cmd(cmd_return); //解析动作组
      } else {

      }
      mode_bs_bak = mode_bs;

    }

  }
}
//##
void xunji() {
  // 假是压线\n真是出线
  if (analogRead(A7) < 512 && analogRead(A6) < 512) {
    if (xj_mode != 1) {
      sprintf(cmd_return_tmp, "{#%03dP%04dT%04d!#%03dP%04dT%04d!}",6,1500+500,0,7,1500-500,0); //组合指令
      Serial.println(cmd_return_tmp); //解析ZMotorD指令
      for (int i = 1; i <= 6; i = i + (1)) {
        rgb_display_A3.setPixelColor((i)-1, (0x33cc00));
      }
      rgb_display_A3.show();
      xj_mode = 1;

    }

  } else if (analogRead(A7) > 512 && analogRead(A6) > 512) {
    if (xj_mode != 2) {
      if (xj_mode == 3) {
        sprintf(cmd_return_tmp, "{#%03dP%04dT%04d!#%03dP%04dT%04d!}",6,1500+300,0,7,1500-700,0); //组合指令
        Serial.println(cmd_return_tmp); //解析ZMotorD指令
        for (int i = 1; i <= 6; i = i + (1)) {
          rgb_display_A3.setPixelColor((i)-1, (0xcc0000));
        }
        rgb_display_A3.show();

      } else if (xj_mode == 4) {
        sprintf(cmd_return_tmp, "{#%03dP%04dT%04d!#%03dP%04dT%04d!}",6,1500+700,0,7,1500-300,0); //组合指令
        Serial.println(cmd_return_tmp); //解析ZMotorD指令
        for (int i = 1; i <= 6; i = i + (1)) {
          rgb_display_A3.setPixelColor((i)-1, (0xcc0000));
        }
        rgb_display_A3.show();
      }
      xj_mode = 2;

    }
  } else if (analogRead(A7) < 512 && analogRead(A6) > 512) {
    if (xj_mode != 3) {
      sprintf(cmd_return_tmp, "{#%03dP%04dT%04d!#%03dP%04dT%04d!}",6,1500+300,0,7,1500-700,0); //组合指令
      Serial.println(cmd_return_tmp); //解析ZMotorD指令
      for (int i = 1; i <= 6; i = i + (1)) {
        rgb_display_A3.setPixelColor((i)-1, (0xcc0000));
      }
      rgb_display_A3.show();
      xj_mode = 3;

    }
  } else if (analogRead(A7) > 512 && analogRead(A6) < 512) {
    if (xj_mode != 4) {
      sprintf(cmd_return_tmp, "{#%03dP%04dT%04d!#%03dP%04dT%04d!}",6,1500+700,0,7,1500-300,0); //组合指令
      Serial.println(cmd_return_tmp); //解析ZMotorD指令
      for (int i = 1; i <= 6; i = i + (1)) {
        rgb_display_A3.setPixelColor((i)-1, (0xcc0000));
      }
      rgb_display_A3.show();
      xj_mode = 4;

    }
  } else {

  }
}
//##
// 测距避障
void cjbz() {
  if (checkdistance_A1_A2() >= 20 && flag == true) {
    sprintf(cmd_return_tmp, "{#%03dP%04dT%04d!#%03dP%04dT%04d!}",6,1500+500,0,7,1500-500,0); //组合指令
    Serial.println(cmd_return_tmp); //解析ZMotorD指令
    flag = false;
    delay(1000);

  } else if (checkdistance_A1_A2() < 20 && flag == false) {
    sprintf(cmd_return_tmp, "{#%03dP%04dT%04d!#%03dP%04dT%04d!}",6,1500+500,0,7,1500-(-500),0); //组合指令
    Serial.println(cmd_return_tmp); //解析ZMotorD指令
    flag = true;
    delay(1000);
  }
}

void setup(){
  Serial.begin(115200);//串口初始化

  setup_w25q(); 				//读取全局变量
	setup_nled();  				//led灯闪烁初始化
	setup_beep();  				//蜂鸣器初始化
	setup_start_pre_cmd();  	//系统启动

  mode = 0;
  mode_bak = 0;
  num = 0;
  num_bak = 0;
  hw_flag = true;
  sound_num = 0;
  sound_check_bak = 0;
  sound_check_bak1 = 0;
  sound_time = 0;
  sound_flg = true;
  sound_time_L = 0;
  distance = 0;
  distance_bak = 0;
  mode_bs = 0;
  mode_bs_bak = 0;
  baoshu_time = 0;
  flag = true;
  xj_mode = 0;
  rgb_display_A3.begin();
  tcs.begin();
  systick_ms_yanse = 0;
  systick_ms_hw = 0;
  systick_ms_hw_bak = 0;
  setup_servo();    //舵机初始化
  rgb_display_A3.setBrightness(50);
  setup_AI();
  sprintf(cmd_return_tmp, "#%03dP%04dT%04d!",8,1500+18,0); //组合指令
  Serial.println(cmd_return_tmp); //解析ZMp3指令
  //irrecv_PIN_IR.enableIRIn();
  pinMode(A0, INPUT);
  pinMode(2, INPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, INPUT);
  pinMode(A7, INPUT);
  pinMode(A6, INPUT);
}

void loop(){
    loop_nled();    //切换LED灯状态实现LED灯闪烁
    loop_uart();    //解析串口接收到的字符串指令
    loop_cmd();     //小车运行时不接受信号时就停止动作，现在为200ms，可更改
    loop_action();  //循环执行是否需要读取数据执行动作组
    if(downLoad)return;
    loop_servo();    //处理模拟舵机增量
    AI_parse_cmd();
    //loop_IR();
    //loop_AI();

    {
    	static byte servo_work = 0, servo_work_bak = 255;
    	servo_work = 0;
    	for(int i=0;i<SERVO_NUM;i++) {
    		if(servo_do[i].inc) {
    			servo_work = 1;
    			break;
    		}
    	}
    	if(servo_work_bak != servo_work) {
    		servo_work_bak = servo_work;
    		if(servo_work) {
    			for(int i=0;i<SERVO_NUM;i++) {
    				myservo[i].attach(servo_pin[i]);
    			}
    		} else {
    			for(int i=0;i<SERVO_NUM;i++) {
    				myservo[i].detach();
    			}
    		}
    	}
    }

}
