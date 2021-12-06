// 使用BSD许可协议
// 需要使用Arduiono 开发环境，使用2.3.5 版本得Rtc_By_Makuna库
// 部分代码使用了文章中的内容：https://blog.csdn.net/qq_22848695/article/details/118547611
// 部分代码使用了文章中的内容：https://blog.csdn.net/qq_36955622/article/details/117475346
// 写着玩的， 没有注意代码规范和整理，凑合看吧。
// 发布于：https://github.com/redpower1998/DS1302_MC2302D_ESP8266_Clock
#include <ESP8266WiFi.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <time.h>

#ifndef uchar
#define uchar unsigned char 
#endif

// DS1302 CLK/SCLK --> ESP-12F D2
// DS1302 DAT/IO   --> ESP-12F D1
// DS1302 RST/CE   --> ESP-12F D0
int DS1302_RST = D0;
int DS1302_DAT = D1;
int DS1302_CLK = D2;

int DIN = D6;
int CLK = D5;
int STB_1 = D3;
int STB_2 = D4;
uchar f_dimming_1 = 0;  //亮度可调  1-7级   0默认 最亮
uchar f_dimming_2 = 0;  //亮度可调  1-7级   0默认 最亮

uchar Font_Data[10] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F };  //数码管
uchar Font_point[4] = { 0x00, 0x03, 0x01, 0x02 };  //数码管 点   

uchar seg_data_1[14] = { 0 };//缓存数组
uchar seg_data_2[14] = { 0 };//缓存数组

//置位  点亮
//中间杂点   
#define DIS1_SET_1    seg_data_1[10]=(seg_data_1[10]|0X01)
#define DIS1_SET_2    seg_data_1[10]=(seg_data_1[10]|0X02)
#define DIS1_SET_3    seg_data_1[7] =(seg_data_1[7] |0X04)
#define DIS1_SET_4    seg_data_1[10]=(seg_data_1[10]|0X04)
#define DIS1_SET_5    seg_data_1[10]=(seg_data_1[10]|0X08)
#define DIS1_SET_6    seg_data_1[9] =(seg_data_1[9] |0X04)
#define DIS1_SET_7    seg_data_1[0] =(seg_data_1[0] |0X80)
#define DIS1_SET_8    seg_data_1[2] =(seg_data_1[2] |0X80)
#define DIS1_SET_9    seg_data_1[4] =(seg_data_1[4] |0X80)
#define DIS1_SET_10   seg_data_1[5] =(seg_data_1[5] |0X01)
#define DIS1_SET_11   seg_data_1[11]=(seg_data_1[11]|0X04)
#define DIS1_SET_12   seg_data_1[5] =(seg_data_1[5] |0X02)
#define DIS1_SET_13   seg_data_1[10]=(seg_data_1[10]|0X10)
#define DIS1_SET_14   seg_data_1[10]=(seg_data_1[10]|0X20)
#define DIS1_SET_15   seg_data_1[10]=(seg_data_1[10]|0X40)
#define DIS1_SET_16   seg_data_1[10]=(seg_data_1[10]|0X80)
#define DIS1_SET_17   seg_data_1[11]=(seg_data_1[11]|0X01)
#define DIS1_SET_18   seg_data_1[11]=(seg_data_1[11]|0X02)


//左右点
#define DIS2_SET_1    seg_data_1[6]=(seg_data_1[6] |0X01)
#define DIS2_SET_2    seg_data_1[6]=(seg_data_1[6] |0X02)
#define DIS2_SET_3    seg_data_1[6]=(seg_data_1[6] |0X04)
#define DIS2_SET_4    seg_data_1[6]=(seg_data_1[6] |0X08)
#define DIS2_SET_5    seg_data_1[6]=(seg_data_1[6] |0X10)
#define DIS2_SET_6    seg_data_1[6]=(seg_data_1[6] |0X20)
#define DIS2_SET_7    seg_data_1[6]=(seg_data_1[6] |0X40)
#define DIS2_SET_8    seg_data_1[6]=(seg_data_1[6] |0X80)
#define DIS2_SET_9    seg_data_1[7]=(seg_data_1[7] |0X01)
#define DIS2_SET_10   seg_data_1[7]=(seg_data_1[7] |0X02) 
#define DIS2_SET_11   seg_data_1[8]=(seg_data_1[8] |0X01) 
#define DIS2_SET_12   seg_data_1[8]=(seg_data_1[8] |0X02) 
#define DIS2_SET_13   seg_data_1[8]=(seg_data_1[8] |0X04) 
#define DIS2_SET_14   seg_data_1[8]=(seg_data_1[8] |0X08) 
#define DIS2_SET_15   seg_data_1[8]=(seg_data_1[8] |0X10) 
#define DIS2_SET_16   seg_data_1[8]=(seg_data_1[8] |0X20) 
#define DIS2_SET_17   seg_data_1[8]=(seg_data_1[8] |0X40) 
#define DIS2_SET_18   seg_data_1[8]=(seg_data_1[8] |0X80) 
#define DIS2_SET_19   seg_data_1[9]=(seg_data_1[9] |0X01) 
#define DIS2_SET_20   seg_data_1[9]=(seg_data_1[9] |0X02) 

//置0  熄灭           
//中间杂点             
#define DIS1_CLR_1    seg_data_1[10]=(seg_data_1[10]&(~0X01))
#define DIS1_CLR_2    seg_data_1[10]=(seg_data_1[10]&(~0X02))
#define DIS1_CLR_3    seg_data_1[7] =(seg_data_1[7] &(~0X04))
#define DIS1_CLR_4    seg_data_1[10]=(seg_data_1[10]&(~0X04))
#define DIS1_CLR_5    seg_data_1[10]=(seg_data_1[10]&(~0X08))
#define DIS1_CLR_6    seg_data_1[9] =(seg_data_1[9] &(~0X04))
#define DIS1_CLR_7    seg_data_1[0] =(seg_data_1[0] &(~0X80))
#define DIS1_CLR_8    seg_data_1[2] =(seg_data_1[2] &(~0X80))
#define DIS1_CLR_9    seg_data_1[4] =(seg_data_1[4] &(~0X80))
#define DIS1_CLR_10   seg_data_1[5] =(seg_data_1[5] &(~0X01))
#define DIS1_CLR_11   seg_data_1[11]=(seg_data_1[11]&(~0X04))
#define DIS1_CLR_12   seg_data_1[5] =(seg_data_1[5] &(~0X02))
#define DIS1_CLR_13   seg_data_1[10]=(seg_data_1[10]&(~0X10))
#define DIS1_CLR_14   seg_data_1[10]=(seg_data_1[10]&(~0X20))
#define DIS1_CLR_15   seg_data_1[10]=(seg_data_1[10]&(~0X40))
#define DIS1_CLR_16   seg_data_1[10]=(seg_data_1[10]&(~0X80))
#define DIS1_CLR_17   seg_data_1[11]=(seg_data_1[11]&(~0X01))
#define DIS1_CLR_18   seg_data_1[11]=(seg_data_1[11]&(~0X02))


//左右点                                  
#define DIS2_CLR_1    seg_data_1[6]=(seg_data_1[6] &(~0X01))
#define DIS2_CLR_2    seg_data_1[6]=(seg_data_1[6] &(~0X02))
#define DIS2_CLR_3    seg_data_1[6]=(seg_data_1[6] &(~0X04))
#define DIS2_CLR_4    seg_data_1[6]=(seg_data_1[6] &(~0X08))
#define DIS2_CLR_5    seg_data_1[6]=(seg_data_1[6] &(~0X10))
#define DIS2_CLR_6    seg_data_1[6]=(seg_data_1[6] &(~0X20))
#define DIS2_CLR_7    seg_data_1[6]=(seg_data_1[6] &(~0X40))
#define DIS2_CLR_8    seg_data_1[6]=(seg_data_1[6] &(~0X80))
#define DIS2_CLR_9    seg_data_1[7]=(seg_data_1[7] &(~0X01))
#define DIS2_CLR_10   seg_data_1[7]=(seg_data_1[7] &(~0X02))  
#define DIS2_CLR_11   seg_data_1[8]=(seg_data_1[8] &(~0X01))  
#define DIS2_CLR_12   seg_data_1[8]=(seg_data_1[8] &(~0X02))  
#define DIS2_CLR_13   seg_data_1[8]=(seg_data_1[8] &(~0X04))  
#define DIS2_CLR_14   seg_data_1[8]=(seg_data_1[8] &(~0X08))  
#define DIS2_CLR_15   seg_data_1[8]=(seg_data_1[8] &(~0X10))  
#define DIS2_CLR_16   seg_data_1[8]=(seg_data_1[8] &(~0X20))  
#define DIS2_CLR_17   seg_data_1[8]=(seg_data_1[8] &(~0X40))  
#define DIS2_CLR_18   seg_data_1[8]=(seg_data_1[8] &(~0X80))  
#define DIS2_CLR_19   seg_data_1[9]=(seg_data_1[9] &(~0X01))  
#define DIS2_CLR_20   seg_data_1[9]=(seg_data_1[9] &(~0X02)) 


uchar RGY_DIS[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  //红1绿1黄1  红2绿2黄2 

uchar  table0[] = { 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f }; //LED从低位往高位移
uchar  table1[] = { 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe }; //LED从高位往低位移
uchar  table2[] = { 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00 }; //LED从1个亮到8个都点亮(从低位往高位)
uchar  table3[] = { 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0x00 }; //LED从1个亮到8个都点亮(从高位往低位)
uchar  table4[] = { 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff }; //LED从8个全亮到一个都不亮(从低位往高位)
uchar  table5[] = { 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff }; //LED从8个全亮到一个都不亮(从高位往低位)
uchar  table6[] = { 0xfe, 0xfc, 0xfa, 0xf6, 0xee, 0xde, 0xbe, 0x7e }; //LED从低位往高位移(最低位始终为0)
uchar  table7[] = { 0x7f, 0x3f, 0x5f, 0x6f, 0x77, 0x7b, 0x7d, 0x7e }; //LED从高位往低位移(最高位始终为0)
uchar i, j;    //定义循环变量
int tt = 70;    //定义时间指数

void Make_Signal_2()
{
  Make_Signal(STB_2);
}
void Make_Signal_1()
{
  Make_Signal(STB_1);
}

void delay_tim(int time)
{
  Make_Signal_2();
  delay(time);
  seg_data_2[2] = seg_data_2[0];
  seg_data_2[8] = seg_data_2[6];
  Make_Signal_2();
  delay(time);
  seg_data_2[4] = seg_data_2[0];
  seg_data_2[10] = seg_data_2[6];
  Make_Signal_2();
  delay(time);
}

void disp0()      //状态0 所有LED闪烁3次
{
  for (i = 0; i < 3; i++)
  {
    seg_data_2[0] = 0x00; seg_data_2[6] = 0x00;
    delay_tim(300);
    seg_data_2[0] = 0xff; seg_data_2[6] = 0xff;
    delay_tim(300);
  }
}

void disp1()      //状态1 LED顺时针转一圈
{
  for (i = 0; i < 8; i++)
  {
    seg_data_2[0] = table1[i];
    delay_tim(100);
  }
  seg_data_2[0] = 0xff;
  for (i = 0; i < 8; i++)
  {
    seg_data_2[6] = table1[i];
    delay_tim(100);
  }
  seg_data_2[6] = 0xff;

}

void disp2()      //状态2 LED逆时针转一圈
{
  for (i = 0; i < 8; i++)
  {
    seg_data_2[6] = table0[i];
    delay_tim(100);
  }
  seg_data_2[6] = 0xff;
  for (i = 0; i < 8; i++)
  {
    seg_data_2[0] = table0[i];
    delay_tim(100);
  }
  seg_data_2[0] = 0xff;
}

void disp3()      //状态3 4个LED同时顺时、逆时针移动1/4圈
{
  for (i = 0; i < 8; i++)
  {

    seg_data_2[0] = table1[i];
    seg_data_2[6] = table0[i];
    delay_tim(100);
  }
  for (i = 0; i < 8; i++)
  {
    seg_data_2[0] = table0[i];
    seg_data_2[6] = table1[i];
    delay_tim(100);
  }
  seg_data_2[6] = 0xff;
}

void disp4()      //状态4  2个LED同时顺时针移动半圈
{
  for (i = 0; i < 8; i++)
  {
    seg_data_2[0] = table1[i];
    delay_tim(100);
  }
  seg_data_2[0] = 0xff;
  for (i = 0; i < 8; i++)
  {
    seg_data_2[6] = table1[i];
    delay_tim(100);
  }
  seg_data_2[6] = 0xff;
}

void disp5()      //状态5  2个LED同时逆时针移动半圈
{
  for (i = 0; i < 8; i++)
  {
    seg_data_2[6] = table0[i];
    delay_tim(100);
  }
  seg_data_2[6] = 0xff;
  for (i = 0; i < 8; i++)
  {
    seg_data_2[0] = table0[i];
    delay_tim(100);
  }
  seg_data_2[0] = 0xff;
}

void disp6()      //状态6 LED自上而下逐渐点亮(一半点亮一半不亮)
{
  for (i = 0; i < 8; i++)
  {

    seg_data_2[0] = table3[i];
    delay_tim(100);
  }
  seg_data_2[0] = 0xff;
  for (i = 0; i < 8; i++)
  {

    seg_data_2[6] = table3[i];
    delay_tim(100);
  }
  seg_data_2[6] = 0xff;
}

void disp7()      //状态7 LED自下而上逐渐点亮(直到全部点亮)
{
  for (i = 0; i < 8; i++)
  {

    seg_data_2[6] = table2[i];
    delay_tim(100);
  }
  for (i = 0; i < 8; i++)
  {
    seg_data_2[0] = table2[i];
    delay_tim(100);
  }
}

void disp8()      //状态8 间断8格的4个LED亮并逆时针旋转
{
  for (j = 0; j < 2; j++)
  {
    for (i = 0; i < 8; i++)
    {
      seg_data_2[0] = table0[i];
      seg_data_2[6] = table0[i];
      delay_tim(100);
    }
    seg_data_2[0] = 0xff;  seg_data_2[6] = 0xff;
    for (i = 0; i < 8; i++)
    {

      seg_data_2[0] = table0[i];
      seg_data_2[6] = table0[i];
      delay_tim(100);
    }
    seg_data_2[0] = 0xff; seg_data_2[6] = 0xff;
  }
}

void disp9()      //状态9 间断8格的4个LED亮，然后逆时针逐渐点亮(直到全部点亮)
{
  for (i = 0; i < 8; i++)
  {
    seg_data_2[0] = table2[i];
    seg_data_2[6] = table2[i];
    delay_tim(100);
  }
  delay_tim(200);
}

void Make_Signal(int index)
{
  uchar i, j;
  uchar dis_data = 0;

  digitalWrite(index, HIGH);
  digitalWrite(CLK, HIGH);

  for (i = 0; i < 8; i++)                                //command 1 (#0000_0011B) 
  {
    //STB_1=0;  
    digitalWrite(index, LOW);
    //CLK=0;
    digitalWrite(CLK, LOW);
    if (i == 1 || i == 0) {
      digitalWrite(DIN, HIGH);
    }
    else {
      digitalWrite(DIN, LOW);
    }
    digitalWrite(CLK, HIGH);
  }
  digitalWrite(index, HIGH);
  for (i = 0; i < 8; i++)        //command 2 (#0100_0000B)
  {
    digitalWrite(index, LOW);
    digitalWrite(CLK, LOW);
    if (i == 6) {
      digitalWrite(DIN, HIGH);
    }
    else {
      digitalWrite(DIN, LOW);
    }
    digitalWrite(CLK, HIGH);
  }
  digitalWrite(index, HIGH);

  for (i = 0; i < 8; i++)        //command 3 (#1100_0000B)
  {
    digitalWrite(index, LOW);
    digitalWrite(CLK, LOW);
    if (i == 6 || i == 7) {
      digitalWrite(DIN, HIGH);
    }
    else {
      digitalWrite(DIN, LOW);
    }
    digitalWrite(CLK, HIGH);
  }

  for (i = 0; i < 14; i++)
  {
    if (index == STB_1)
      dis_data = seg_data_1[i];
    else
      dis_data = seg_data_2[i];
    for (j = 0; j < 8; j++)
    {
      digitalWrite(index, LOW);
      digitalWrite(CLK, LOW);
      digitalWrite(DIN, dis_data & 0x01);
      digitalWrite(CLK, HIGH);
      dis_data = dis_data >> 1;//右移一位
    }
  }
  digitalWrite(index, HIGH);

  for (i = 0; i < 8; i++)        //command 4 (#1000_0100B)
  {
    digitalWrite(index, LOW);
    digitalWrite(CLK, LOW);
    switch (f_dimming_1)  //显示亮度模式
    {
    case(0):
      if (i == 3 || i == 7 || i == 2 || i == 1 || i == 0)
      {
        digitalWrite(DIN, HIGH);
      }
      else
      {
        digitalWrite(DIN, LOW);
      }
      break;
    case(1):
      if (i == 0 || i == 3 || i == 7)
      {
        digitalWrite(DIN, HIGH);
      }
      else
      {
        digitalWrite(DIN, LOW);
      }
      break;
    case(2):
      if (i == 1 || i == 3 || i == 7)
      {
        digitalWrite(DIN, HIGH);
      }
      else
      {
        digitalWrite(DIN, LOW);
      }
      break;
    case(3):
      if (i == 1 || i == 0 || i == 3 || i == 7)
      {
        digitalWrite(DIN, HIGH);
      }
      else
      {
        digitalWrite(DIN, LOW);
      }
      break;
    case(4):
      if (i == 2 || i == 3 || i == 7)
      {
        digitalWrite(DIN, HIGH);
      }
      else
      {
        digitalWrite(DIN, LOW);
      }
      break;
    case(5):
      if (i == 2 || i == 0 || i == 3 || i == 7)
      {
        digitalWrite(DIN, HIGH);
      }
      else
      {
        digitalWrite(DIN, LOW);
      }
      break;
    case(6):
      if (i == 2 || i == 1 || i == 3 || i == 7)
      {
        digitalWrite(DIN, HIGH);
      }
      else
      {
        digitalWrite(DIN, LOW);
      }
      break;
    case(7):
      if (i == 2 || i == 1 || i == 0 || i == 7)
      {
        digitalWrite(DIN, HIGH);
      }
      else
      {
        digitalWrite(DIN, LOW);
      }
      break;
    }
    digitalWrite(CLK, HIGH);

  }
  digitalWrite(index, HIGH);
}

void display_clock(RtcDateTime& now)
{
  memset(seg_data_1, 0, 14);

  seg_data_1[3] = Font_point[1];

  int hour = now.Hour();
  int hour12 = hour;
  bool is_morning = true;
  if( hour > 12 ) 
  {
    hour12 = hour % 12;
    is_morning = false;
  }

  seg_data_1[1] = Font_point[hour12 / 10];   //时的十位
  seg_data_1[0] = Font_Data[hour12 % 10];    //时的个位
  seg_data_1[2] = Font_Data[now.Minute() / 10];    //分的十位
  seg_data_1[4] = Font_Data[now.Minute() % 10];    //分的个位
  if (is_morning)  //上午下午
  {
    DIS1_SET_1;
    DIS1_SET_2;
    DIS1_SET_3;
  }
  else
  {
    DIS1_SET_4;
    DIS1_SET_5;
    DIS1_SET_6;
  }
  switch (now.DayOfWeek())
  {
  case DayOfWeek_Monday:
    DIS2_SET_1;
    DIS2_SET_11;
    break;
  case DayOfWeek_Tuesday:
    DIS2_SET_1;
    DIS2_SET_12;
    break;
  case DayOfWeek_Wednesday:
    DIS2_SET_3;
    DIS2_SET_13;
    break;
  case DayOfWeek_Thursday:
    DIS2_SET_4;
    DIS2_SET_14;
    break;
  case DayOfWeek_Friday:
    DIS2_SET_5;
    DIS2_SET_15;
    break;
  case DayOfWeek_Saturday:
    DIS2_SET_6;
    DIS2_SET_16;
    break;
  case DayOfWeek_Sunday:
    DIS2_SET_7;
    DIS2_SET_17;
    break;
  }

  Make_Signal(STB_1);
}

ThreeWire myWire(DS1302_DAT, DS1302_CLK, DS1302_RST); // DAT, CLK, RST
RtcDS1302<ThreeWire> Rtc(myWire);

void setup()
{
  WiFi.mode(WIFI_OFF);
  pinMode(DIN, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(STB_1, OUTPUT);
  pinMode(STB_2, OUTPUT);
  Rtc.Begin();
  Serial.begin(115200);

  if (!Rtc.IsDateTimeValid())
  {
    Serial.println("RTC lost confidence in the DateTime!");
    //新建RtcDateTime 对象，并写入日期时间，参数顺序为Year,Month,Day,Hour,Minutes,Seconds
    //新建RTC时间对象 - 2021年6月3日9点整（默认24小时制）
    RtcDateTime DStime_update(2021, 12, 6, 10, 31, 0);

    //调用SetDateTime函数为RTC模块写入时间
    Rtc.SetDateTime(DStime_update);
  }

  Make_Signal(STB_1);
  Make_Signal(STB_2);
}

void loop()
{
  //GetDateTime函数获取RTC模块时间
  RtcDateTime now = Rtc.GetDateTime();

  printDateTime(now);
  Serial.println();
  display_clock(now);
  disp9();  //延迟一秒
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
  char datestring[20];

  snprintf_P(datestring,
    countof(datestring),
    PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
    dt.Month(),
    dt.Day(),
    dt.Year(),
    dt.Hour(),
    dt.Minute(),
    dt.Second());
  Serial.print(datestring);
}