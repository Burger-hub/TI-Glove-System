/*
 * SendDemo.cpp
 *
 * Demonstrates sending IR codes in standard format with address and command
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2020-2023 Armin Joachimsmeyer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************************
 */

#include <Arduino.h>

#include "PinDefinitionsAndMore.h"  // Define macros for input and output pin etc.

#define DISABLE_CODE_FOR_RECEIVER // Disables restarting receiver after each send. Saves 450 bytes program memory and 269 bytes RAM if receiving functions are not used.
#define IR_SEND_PIN      3 // 3
//#define EXCLUDE_EXOTIC_PROTOCOLS  // Saves around 240 bytes program memory if IrSender.write is used
//#define SEND_PWM_BY_TIMER         // Disable carrier PWM generation in software and use (restricted) hardware PWM.
//#define USE_NO_SEND_PWM           // Use no carrier PWM, just simulate an active low receiver signal. Overrides SEND_PWM_BY_TIMER definition
//#define NO_LED_FEEDBACK_CODE      // Saves 566 bytes program memory
//#define USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN // Use or simulate open drain output mode at send pin. Attention, active state of open drain is LOW, so connect the send LED between positive supply and send pin!

#include <IRremote.hpp>

#define DELAY_AFTER_SEND 500
#define DELAY_AFTER_LOOP 100

void setup() {
    Serial.begin(115200);
    IrSender.sendNEC(0xEF00, 0x3, 0);//开灯
    delay(200);
    IrSender.sendNEC(0xEF00, 0x2, 0);//关灯
    delay(200);
    IrSender.sendNEC(0xEF00, 0x3, 0);//开灯
    delay(200);
    IrSender.sendNEC(0xEF00, 0x2, 0);//关灯
    delay(200);

#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/|| defined(SERIALUSB_PID) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    // Just to know which program is running on my Arduino
    //Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

#if defined(IR_SEND_PIN)
    IrSender.begin(); // Start with IR_SEND_PIN as send pin and enable feedback LED at default feedback LED pin
#  if defined(IR_SEND_PIN_STRING)
    //Serial.println(F("Send IR signals at pin " IR_SEND_PIN_STRING));
#  else
    //Serial.println(F("Send IR signals at pin " STR(IR_SEND_PIN)));
#  endif
#else
    IrSender.begin(3, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN); // Specify send pin and enable feedback LED at default feedback LED pin
    //Serial.println(F("Send IR signals at pin 3"));
#endif
  
}

/*
 * Set up the data to be sent.
 * For most protocols, the data is build up with a constant 8 (or 16 byte) address
 * and a variable 8 bit command.
 * There are exceptions like Sony and Denon, which have 5 bit address.
 */
uint16_t sAddress = 0xEF00;
uint8_t sCommand = 0x3;
int sRepeats=0;
char var;
float num;
bool state=0;

void loop() { 
    /*
    Serial.println();
    Serial.print(F("address=0x"));
    Serial.print(sAddress, HEX);
    Serial.print(F(" command=0x"));
    Serial.print(sCommand, HEX);
    Serial.print(F(" repeats="));
    Serial.println(sRepeats);
    Serial.println();
    Serial.flush();//等待串口缓冲区中的数据发送完成，用于发送数据之后
    IrSender.sendNEC(sAddress, sCommand, sRepeats);//开灯
    Serial.println(F("渐亮"));
    Serial.flush();
    */
    /*
    while(Serial.available()){
      sCommand=Serial.read();
      Serial.println(sCommand);
      IrSender.sendNEC(sAddress, sCommand, 0);
      delay(6);// delay must be greater than 5 ms (RECORD_GAP_MICROS), otherwise the receiver sees it as one long signal
    }
  
  */  
  while(Serial.available()>0)//当有信号的时候
    {
      var=Serial.read();
      //Serial.print(var);
      if((var=='0')&(state==1)){
        IrSender.sendNEC(0xEF00, 0x0, 0);//亮度增加
        state=1;
      }
      if((var=='1')&(state==1)){
        IrSender.sendNEC(0xEF00, 0x1, 0);//亮度减小
        state=1;
      }
      if((var=='2')&(state==1)){
        IrSender.sendNEC(0xEF00, 0x2, 0);//关灯
        state=0;
      }
      if((var=='3')&(state==0)){
        IrSender.sendNEC(0xEF00, 0x3, 0);//开灯
        state=1;
      }
      if((var=='4')&(state==1)){
        IrSender.sendNEC(0xEF00, 0x4, 0);//红色
        state=1;
      }
      if((var=='5')&(state==1)){
        IrSender.sendNEC(0xEF00, 0x5, 0);//绿色
        state=1;
      }
      if((var=='6')&(state==1)){
        IrSender.sendNEC(0xEF00, 0x6, 0);//蓝色
        state=1;
      }
      if((var=='7')&(state==1)){
        IrSender.sendNEC(0xEF00, 0x7, 0);//白色
        state=1;
      }
      delay(5);//延时5ms

    }
   
}

