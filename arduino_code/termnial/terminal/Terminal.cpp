// @Author : Darrius Lei
// @Email  : darrius.lei@outlook.com
#include "subincludes.h"
#include "Terminal.h"
#include "Timer.h"

//构造函数
Terminal::Terminal(byte addr_in, bool acs)
{
  addr = addr_in;
  Acs = acs;
  Acs_permit = true;
  Link = false;
}

//发送报文
void  Terminal::sendMessage(Message* msg)
{
  delay(DELAY_SEND);
  msg->MCS = this->generateMCS(*msg);
  LoRa.beginPacket();
  LoRa.print((char)msg->DAddr);               //发送目的地址
  LoRa.print((char)msg->SAddr);               //发送源地址
  LoRa.print((char)msg->Function);            //发送功能位
  LoRa.print((char)msg->DataLen);             //发送数据长度
  LoRa.print((char)msg->MCS);                 //发送校验和位
  LoRa.print((msg->MsgData));                 //发送数据
  LoRa.endPacket();

  Serial.print("send:");
  Serial.print((char)msg->DAddr);
  Serial.print((char)msg->SAddr);
  Serial.print((char)msg->Function);
  Serial.print((char)msg->DataLen);
  Serial.print((char)msg->MCS);
  Serial.println((msg->MsgData));
}

//接收报文
bool Terminal::recvMessage(Message* pmsg)
{
  if(LoRa.parsePacket() == 0) return false;   //无数据则返回
  String str;
  int len;
  while(LoRa.available())                     //开始读数据（根据协议可知，不会收到连续的两帧）
  {
    str += (char)LoRa.read();
  }
//  Serial.print("收到的消息，不一定是自己的:");
//  Serial.println(str);
  if(str.charAt(0) == (char)addr || str.charAt(0) == ADDRESS_BC)
  {
    pmsg->DAddr = str.charAt(0);
    pmsg->SAddr = (byte)str.charAt(1);
    pmsg->Function = (byte)str.charAt(2);
    pmsg->DataLen = (byte)str.charAt(3);
    pmsg->MCS = (byte)str.charAt(4);
    len = (int)str.charAt(3) - (int)'0';
    str.remove(0,5);
    //str.remove(len, str.length() - len);
    pmsg->MsgData = str;
    if(this->checkMCS(*pmsg))
    {
      Serial.print("recive:");
      Serial.print((char)pmsg->DAddr);
      Serial.print((char)pmsg->SAddr);
      Serial.print((char)pmsg->Function);
      Serial.print((char)pmsg->DataLen);
      Serial.print((char)pmsg->MCS);
      Serial.println(pmsg->MsgData);
      Serial.print(" len:");
      Serial.println((pmsg->MsgData).length());
      return true;
    }
  }
  return false;
}

//请求接入
bool Terminal::requestAcs()
{
  String str = ACS_PASSWORD;
  Message msg_s;
  Message msg_r;
  Timer timer(0);
  msg_s.DAddr = (byte)ADDRESS;
  msg_s.SAddr = addr;
  msg_s.Function = (byte)'6';
  msg_s.DataLen = (byte)(str.length() + (int)'0');
  (msg_s.MsgData) = str;
  delay(random(200));               //随机延时一段时间
  this->sendMessage(&msg_s);
  timer.new_start(MAX_WAIT_TIME_ACS);
  while(1)
  {
    if(this->recvMessage(&msg_r))
    {
      if(msg_r.DAddr == addr && msg_r.SAddr == (byte)ADDRESS && msg_r.Function == (byte)'6' && (msg_r.MsgData).charAt(0) == CHAR_ACK)
      {
        //确认为相应的ACK
        Acs = true;
        return true;
      }
      if(timer.result())
      {
        Serial.println("请求接入,因为超时失败");
        Acs = false;
        return false;
      }
    }
  }
}

//接收状态
void Terminal::recvState()
{
  Message msg_r;
  Message msg_s;
  String str;
  msg_s.DAddr = (byte)ADDRESS;
  msg_s.SAddr = addr;
  Timer timer_contect(0);                                     //用于联系计时
  while (1)
  {
    if(this->recvMessage(&msg_r))
    {
      if((msg_r.DAddr == addr || msg_r.DAddr == (byte)ADDRESS_BC) && msg_r.SAddr == (byte)ADDRESS)
      {
        //确认是中心节点发给自己的帧
        if(msg_r.Function == (byte)'0' && Acs)                //接到要求退网命令
        {
          Serial.println("接到要求退网命令");
          timer_contect.zero();
          Acs_permit = false;                                 //不允许接入
          Acs = false;                                        //退出接入
          Link = false;
          msg_s.Function = (byte)'0';
          msg_s.DataLen = (byte)'1';
          (msg_s.MsgData) = CHAR_ACK;
          delay(120);
          this->sendMessage(&msg_s);
        }
        else if(msg_r.Function == (byte)'1' && Acs && Acs_permit)         //收到建立连接命令
        {
          
          timer_contect.new_start(MAX_WAIT_TIME_CONTACT);     //重新联系计时
          if((msg_r.MsgData).charAt(0) == CHAR_ACK) continue;
          Serial.println("收到建立连接命令");
          Link = true;
          msg_s.Function = (byte)'1';
          msg_s.DataLen = (byte)'1';
          (msg_s.MsgData) = CHAR_ACK;
          delay(200);
          this->sendMessage(&msg_s);
        }
        else if(msg_r.Function == (byte)'2' && Acs && Link && Acs_permit) //收到发送数据命令
        {
          Serial.println("收到发送数据命令");
          timer_contect.new_start(MAX_WAIT_TIME_CONTACT);     //重新联系计时
          str = this->getData();
          msg_s.Function = (byte)'2';
          msg_s.DataLen = (byte)(str.length() + (int)'0');
          msg_s.MsgData = str;
          delay(120);
          this->sendMessage(&msg_s);
          Link = false;
        }
        else if(msg_r.Function == (byte)'3' && Acs && Acs_permit)         //询问存在
        {
          Serial.println("收到询问存在命令");
          Link = false;
          timer_contect.new_start(MAX_WAIT_TIME_CONTACT);     //重新联系计时
          msg_s.Function = (byte)'3';
          msg_s.DataLen = (byte)'1';
          (msg_s.MsgData) = CHAR_ACK;
          delay(120);
          this->sendMessage(&msg_s);
        }
        else if(msg_r.Function == (byte)'4' && Acs && Acs_permit)         //保持联系
        {
          Serial.println("收到保持联系命令");
          Link = false;
          timer_contect.new_start(MAX_WAIT_TIME_CONTACT);     //重新联系计时
        }
        else if(msg_r.Function == (byte)'5' && Acs == false && Acs_permit)//广播通知接入
        {
          Serial.println("收到广播通知接入并申请接入");
          delay(120);
          if(this->requestAcs())                              //成功接入
          {
            Serial.println("已经成功接入了");
            Acs = true;
            Link = false;
            timer_contect.new_start(MAX_WAIT_TIME_CONTACT);   //重新联系计时
          }
        }
        else if(msg_r.Function == (byte)'7' && Acs_permit == false)//通知同意申请接入
        {
          timer_contect.new_start(MAX_WAIT_TIME_CONTACT);     //重新联系计时
          Serial.println("主机同意申请接入");
          Acs_permit = true;
          msg_s.Function = (byte)'7';
          msg_s.DataLen = (byte)'1';
          msg_s.MsgData = CHAR_ACK;
          delay(130);
          this->sendMessage(&msg_s);
        }
        else if(msg_r.Function == (byte)'8' && Acs)         //更改频率
        {
          timer_contect.new_start(MAX_WAIT_TIME_CONTACT);     //重新联系计时
          Serial.println("主机要求更改频率");
          msg_s.Function = (byte)'8';
          msg_s.DataLen = (byte)'1';
          msg_s.MsgData = CHAR_ACK;
          delay(120);
          this->sendMessage(&msg_s);
          LoRa.setFrequency(msg_r.MsgData.toInt() * 1E6);
        }
      }
    }
    if(timer_contect.result())
    {
      Acs_permit = true;
      Acs = false;
      Link = false;
      timer_contect.zero();
    }
  }
}

String Terminal::getData()
{
  int sensorValue;
  long int long_sensorValue;
  float float_sensorValue;
  char char_buffer[40];
  String pvalue;
  if(addr == (byte)'1' )
  {
    DHT.read(DATA_PIN);
    sensorValue = DHT.temperature;
    float_sensorValue =  (float)sensorValue/1023*5;
  }
  else if(addr == (byte)'2')
  {
    DHT.read(DATA_PIN);
    sensorValue = DHT.humidity;
    float_sensorValue =  (float)sensorValue/1023*5;
  }
  else if(addr == (byte)'3')
  {
    long_sensorValue = analogRead(A0);
    float_sensorValue =  (float)long_sensorValue/1023*5;
  }
  dtostrf(float_sensorValue, 1, 5, char_buffer); 
  pvalue = char_buffer;
  return pvalue;
}

byte Terminal::generateMCS(Message msg)
{
  byte mcs;
  mcs = (msg.DAddr)^(msg.SAddr)^(msg.Function)^(msg.DataLen);
  return mcs;
}

bool Terminal::checkMCS(Message msg)
{
  if(msg.MCS == this->generateMCS(msg))
  {
    return true;
  }
  return false;
}
