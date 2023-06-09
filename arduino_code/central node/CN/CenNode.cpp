// @Author : Darrius Lei
// @Email  : darrius.lei@outlook.com
#include "subincludes.h"
#include "CenNode.h"
#include "Timer.h"

//默认构造函数
CenNode::CenNode()
{
  NumTerAcs = 0;                                            //默认无终端接入网络       
  NumTerPol = 0;
}

//测试初始化：用于测试
void CenNode::test_init(int tpt[3], bool allAcs)
{
  NumTerAcs = 0;
  NumTerPol = 0;
  //使用输入的轮询表进行初始化
  for(int i = 0;i < 3;i++)
  {
    if (tpt[i] == -1) continue;
    TerPolTbl[i] = tpt[i];
    TerAcs |= AddrMaskMap[tpt[i]];
    NumTerAcs ++;
    NumTerPol ++;
  }
  if(allAcs)
  {
    NumTerAcs = 3;
    TerAcs = (byte)7;
  }
  TerData[0] = "a";
  TerData[1] = "bb";
  TerData[2] = "ccc";
}

//发送报文（无线）
void CenNode::sendMessage(Message* msg)
{
  msg->MCS = this->generateMCS(*msg);
  LoRa.beginPacket();
  LoRa.print((char)msg->DAddr);                             //发送目的地址
  LoRa.print((char)msg->SAddr);                             //发送源地址
  LoRa.print((char)msg->Function);                          //发送功能位
  LoRa.print((char)msg->DataLen);                           //发送数据长度
  LoRa.print((char)msg->MCS);                               //发送校验和位
  LoRa.print((msg->MsgData));                               //发送数据
  LoRa.endPacket();
  
//  Serial.print("send:");
//  Serial.print((char)msg->DAddr);
//  Serial.print((char)msg->SAddr);
//  Serial.print((char)msg->Function);
//  Serial.print((char)msg->DataLen);
//  Serial.print((char)msg->MCS);
//  Serial.println((msg->MsgData));
}

//接收报文
bool CenNode::recvMessage(Message* pmsg)
{
  if(LoRa.parsePacket() == 0) return false;
  String str;
  int len;
  while(LoRa.available())                                   //开始读数据（根据协议可知，不会收到连续的两帧）
  {
    str += (char)LoRa.read();                               //读取收到的所有数据
  }
//  Serial.println(str);
  if(str.charAt(0) == (char)addr)                           //确认是发送给本机，则开始对报文结构体属性赋值
  {
    pmsg->DAddr = addr;                                     
    pmsg->SAddr = (byte)str.charAt(1);                      //源地址为第二位（索引为1）
    pmsg->Function = (byte)str.charAt(2);                   //功能位为第三位（索引为2）
    pmsg->DataLen = (byte)str.charAt(3);                    //数据长度位为第四位（索引为3）
    pmsg->MCS = (byte)str.charAt(4);                        //校验和位为第五位（索引为4）
    len = (int)str.charAt(3) - (int)'0';
    str.remove(0,5);
    //str.remove(len, str.length() - len);
    pmsg->MsgData = str;                                    //删除前5位后剩下的字节全为数据
    if(this->checkMCS(*pmsg))                               //进行校验和检验
    {
//      Serial.print("recive:");
//      Serial.print((char)pmsg->DAddr);
//      Serial.print((char)pmsg->SAddr);
//      Serial.print((char)pmsg->Function);
//      Serial.print((char)pmsg->DataLen);
//      Serial.print((char)pmsg->MCS);
//      Serial.println(pmsg->MsgData);
//      Serial.print(" len:");
//      Serial.println((pmsg->MsgData).length());
      return true;
    }
  }
  return false;
}

//接收来自主机的命令（有线） 返回值包含功能位
bool CenNode::recvCommmand(String* pCommand)
{
  int length_r;
  char buffer_r[100];
  String buffer_rc;
  if(Serial.available())                                    //串口缓冲区有数据
  {
    delay(100);                                             //延迟100ms
    //找同步位
    length_r = Serial.readBytesUntil(CHAR_SYNC, buffer_r, 100);
    if(length_r == 0)                                       //找到同步位
    {
      //一直读到结束位
      length_r = Serial.readBytesUntil(CHAR_END, buffer_r, 100);
      Serial.println(length_r);
      buffer_r[length_r] = '\0';
      buffer_rc = buffer_r;
      Serial.println(buffer_rc);
      if(this->checkFCS(buffer_rc) == false) return false;
      buffer_rc.remove(0,2);
      *pCommand = buffer_rc;
//      Serial.print("收到的命令:");
//      Serial.println(*pCommand);
      //是否为有效命令（来自主机的帧，对应的功能位为字符0，1，2，8）
      if(buffer_r[2] == '0' || buffer_r[2] == '1' || buffer_r[2] == '2' || buffer_r[2] == '8')
      {
        return true;
      }
    }
  }
  return false;
}

//向主机发送反馈
void CenNode::sendFeedback()
{
  String mybuffer;
  //设置静态变量，在网络接入状态发生变化和信道参数发生变化（这里主要指频率）时，再向主机进行反馈                                         
  static byte Acs_tem = 10;
  static long int CF = 0;
  int int_addr;
  char char_func;
  //生成接入状态字符串
  for(int i = 1;i < 4;i++)
  {
    if(TerAcs & AddrMaskMap[i])                           //检查接入
    {
      mybuffer += '1';
    }
    else
    {
      mybuffer += '0';
    }
  }

  
  //第一个反馈：在网络中的终端接入状态（发生变化时才发送）
  if(Acs_tem != TerAcs)
  {
    Acs_tem = TerAcs;  
    Serial.print(CHAR_SYNC);                              //发送同步位
    Serial.print((String)this->generateFCS('3'+mybuffer));        //发送校验位
    Serial.print('3');                                    //发送功能位
    Serial.print(mybuffer);                               //发送在网络中的终端（数据）
    Serial.println(CHAR_END);                             //发送终止位
    delay(100);
  }

  //第二个反馈：网络中的信道参数（发生变化时才发送）
  if(CF != channelFre)
  {
    CF = channelFre;
    mybuffer = String(channelFre);
    mybuffer.remove(3,6);
    Serial.print(CHAR_SYNC);                              //发送同步位
    Serial.print(this->generateFCS('7'+mybuffer));        //发送校验位
    Serial.print('7');                                    //发送功能位
    Serial.print(mybuffer);                               //发送在网络中的终端（数据）
    Serial.println(CHAR_END);                             //发送终止位
    delay(100);
  }
  
  //根据主机需要发送数据相应的在网终端采集到的数据
  for(int i = 0;i < 3;i++)
  {
    if(TerPolTbl[i] == -1) continue;
    if((TerAcs & AddrMaskMap[TerPolTbl[i]]) == 0) continue;
    int_addr = TerPolTbl[i];
    if(int_addr <= 0 && int_addr >=4) continue;
    //轮询号有效且相应终端在网
    char_func = (char)((int)AddrMap[int_addr] + 3);                       //功能位

    Serial.print(CHAR_SYNC);                                              //发送同步位
    //发送校验位
    Serial.print(this->generateFCS(char_func+ (char)((TerData[int_addr - 1]).length() + (int)'0') + TerData[int_addr - 1]));                  
    Serial.print(char_func);                                              //发送功能位
    Serial.print( (char)((TerData[int_addr - 1]).length() + (int)'0') );  //发送长度
    Serial.print(TerData[int_addr - 1]);                                  //发送在网络中的终端（数据）
    Serial.println(CHAR_END);                                             //发送终止位
    delay(100);
  }
  
}

//主机要求终端退网（在轮询阶段使用），只发送请求
void CenNode::request_to_withdrew(String* pCommand)
{
  if((*pCommand).charAt(0) != '0') return;                                //防止可能错误调用函数
  String cmd = *pCommand;                                                 //备份，防止更改原数据
  cmd.remove(0, 1);                                                       //删掉功能位
  Message msg_s;
  msg_s.SAddr = addr;                                                     //源地址为中心节点地址
  msg_s.Function = (byte)'0';                                             //功能为要求退网
  Timer timer(0);                                                         //定时器
  int int_addr;                                                           //地址
  char char_addr;                                                         //地址    
  int len = cmd.length();                                                 //收到的命令的长度
  String msgdata = " ";
  Message msg_r;
  for(int i = 0;i < len;i++)
  {
    int_addr = (int)cmd.charAt(i) - (int)'0' ;
    char_addr = cmd.charAt(i);
    if(int_addr > 0 && int_addr <4)           //合法地址
    {
      byte a = TerAcs & AddrMaskMap[int_addr];
      byte b =0;
      if(a == b) //未接入
      {
        continue;
      }
      else
      {
//        Serial.print("主机已要求退网");
        
        TerAcs &= ~AddrMaskMap[int_addr] ;                              //退出
        NumTerAcs--;                                                    //接入数减1
        msg_s.DAddr = (byte)char_addr;                                  //目的地址
        //数据长度
        msg_s.DataLen = (byte)(msgdata.length() + (int)'0') ;    
        msg_s.MsgData = msgdata;                                        //数据
        this -> sendMessage(&msg_s);                                    //发送数据
        timer.new_start(MAX_WAIT_TIME);                                 //开始计时
        while (1)
        {
          if(timer.result())                                            //定时到，没有收到相应的确认
          {
            TerUnAcsFail |= AddrMaskMap[int_addr];                      //加入相应的退出失败位图
            break;
          }
          if(this -> recvMessage(&msg_r))
          {
            //是对应终端发给中心节点关于退网通知的确认
            if(msg_r.DAddr == addr && msg_r.SAddr == (byte)char_addr && msg_r.Function == (byte)'0' && (msg_r.MsgData).charAt(0) == CHAR_ACK)    
            {
              break;
            }
          }
        }
      }
    }
  }
}

//主机要求的数据（更新轮询表）
void CenNode::requestPol(String* pCommand)
{
  if((*pCommand).charAt(0) != '1') return;                            //防止可能错误调用函数
  String cmd = *pCommand;                                             //备份，防止更改原数据
  cmd.remove(0, 1);                                                   //删掉功能位
  this -> polzero();                                                  //重置轮询表
  int len = cmd.length();                                             //读取长度
//  int int_addr;
//  for(int i = 0;i < len;i++)
//  {
//    int_addr = (int)cmd.charAt(i) - (int)'0';       //读取
//    TerPolTbl[i] = int_addr;
//    NumTerPol++;
//  }
  for(int i = 0;i < len;i++)
  {
    if(cmd.charAt(i) == '1')
    {
      TerPolTbl[i] = i + 1;
      NumTerPol++;
    }
  }
}

//主机同意终端申请入网
void CenNode::perAcs(String* pCommand)
{
  if((*pCommand).charAt(0) != '2') return;                          //防止可能错误调用函数
  String cmd = *pCommand;                                           //备份，防止更改原数据
  cmd.remove(0, 1);                                                 //删掉功能位
  Message msg_s;
  msg_s.SAddr = addr;                                               //源地址为中心节点地址
  msg_s.Function = (byte)'7';                                       //功能为允许申请入网
  Timer timer(0);                                                   //定时器
  int int_addr;                                                     //地址
  char char_addr;                                                   //地址    
  int len = cmd.length();                                           //收到的命令的长度
  String msgdata = " ";
  Message msg_r;
  for(int i = 0;i < len;i++)
  {
    int_addr = (int)cmd.charAt(i) - (int)'0' ;
    char_addr = cmd.charAt(i);
    if(int_addr > 0 && int_addr <4)                                 //合法地址
    {
      msg_s.DAddr = (byte)char_addr;
      msg_s.DataLen = (byte)'1';
      msg_s.MsgData = msgdata;
//      Serial.print("已发送允许申请入网");
      this->sendMessage(&msg_s);
      timer.new_start(MAX_WAIT_TIME);
      while(1)
      {
        if(this -> recvMessage(&msg_r))
        {
          if(msg_r.DAddr == addr && msg_r.SAddr == (byte)char_addr && msg_r.Function == (byte)'7' && (msg_r.MsgData).charAt(0) == CHAR_ACK)
          {
//            Serial.print("收到ACK回复");
            return;
          }
        }
        if(timer.result())
        {
//          Serial.print("超时未收到回复");
          return;
        }
      }
    }
  }
}

//主机广播通知更改频率（对已经入网的）
void CenNode::requestCF(String* pCommand)
{
  if((*pCommand).charAt(0) != '8') return;                        //防止可能错误调用函数 
//  Serial.println("接到更改频率命令");
  String cmd = *pCommand;                                         //备份，防止更改原数据
  cmd.remove(0, 1);                                               //删掉功能位
  Message msg_s;
  msg_s.SAddr = addr;                                             //源地址为中心节点地址
  msg_s.Function = (byte)'8';                                     //功能为更改频率
  msg_s.DataLen = (byte)'3';
  msg_s.MsgData = cmd;
  Timer timer(0);                                                 //定时器
  Message msg_r; 
  char char_addr;                   
  for(int i = 1;i < 4;i++)
  {
    char_addr = (char)(i + (int)'0');
    if(TerAcs & AddrMaskMap[i])                                   //终端i在网
    {
      msg_s.DAddr = (byte)char_addr;                              //更改目的地址
      this->sendMessage(&msg_s);                                  //发送报文 
      timer.new_start(MAX_WAIT_TIME_CF);                          //设置定时器
      while(1)
      {
        if(this->recvMessage(&msg_r))
        {
          if(msg_r.DAddr == addr && msg_r.SAddr == (byte)char_addr && msg_r.Function == (byte)'8' && (msg_r.MsgData).charAt(0) == CHAR_ACK)
          {
//            Serial.println("更改频率成功");
            break;
          }
        }
        if(timer.result())
        {
//          Serial.print("超时未收到回复");
          break;
        }
      }     
    }
  }
  channelFre = cmd.toInt() * 1E6;
  LoRa.setFrequency(channelFre);
}

//中心节点与终端建立连接，使用前需要确保终端已经接入网络
bool CenNode::connectTer(int int_addr)
{
  char char_addr = (char)(int_addr + (int)'0');
  Message msg_s;
  Message msg_r;
  Timer timer(0);
  msg_s.DAddr = (byte)char_addr;                              //目的地址
  msg_s.SAddr = addr;                                         //源地址
  msg_s.Function = (byte)'1';                                 //功能位：建立连接
  msg_s.DataLen = (byte)'1';                                  //数据长度
  (msg_s.MsgData) = " ";                                      //数据
  this->sendMessage(&msg_s);                                  //发送数据
  delay(100);
  timer.new_start(MAX_WAIT_TIME);                             //开始计时
  while(1)
  {
    if(this->recvMessage(&msg_r))                             //接收到数据   
    {
      //确定为来自目标的ACK
      if(msg_r.DAddr == addr && msg_r.SAddr == (byte)char_addr && msg_r.Function == (byte)'1' && (msg_r.MsgData).charAt(0) == CHAR_ACK)
      {
        (msg_s.MsgData) = CHAR_ACK;
        this->sendMessage(&msg_s);                            //发送确认消息
//        Serial.print("连接建立成功");
        return true;                                          //连接建立成功
      }
    }
    if(timer.result())                                        //定时到
    {
//      Serial.print("建立连接失败未收到回复");
      TerAcs &= ~AddrMaskMap[int_addr];                       //先退出接入
      NumTerAcs--;                                            //接入数减1
      TerAcsFail |= AddrMaskMap[int_addr];                    //加入可能退出表
      return false;
    }
  }
    
}

//获取对应数据
bool CenNode::getData(int int_addr)
{
  char char_addr = (char)AddrMap[int_addr];                 //目的地址
  int len;
  Message msg_s;                                            //发送报文
  Message msg_r;                                            //接收报文 
  Timer timer(0);
  msg_s.SAddr = addr;                                       //源地址
  msg_s.DAddr = (byte)char_addr;                            //目的地址
  msg_s.Function = (byte)'2';                               //功能位（数据）
  msg_s.DataLen = (byte)'1';                
  (msg_s.MsgData) = " ";
  this->sendMessage(&msg_s);
  timer.new_start(MAX_WAIT_TIME);                           //开启定时器
  while(1)
  {
    if(this->recvMessage(&msg_r))
    {
      //接收到报文
      if(msg_r.DAddr == addr && msg_r.SAddr == (byte)char_addr && msg_r.Function == (byte)'2')
      {
        len = (int)msg_r.DataLen - (int)'0';
        TerData[int_addr - 1] = (msg_r.MsgData);
        return true;
      }
    }
    if(timer.result())                                      //定时到
    {
      TerAcs &= ~AddrMaskMap[int_addr];                     //暂时退出接入
      NumTerAcs--;
      TerAcsFail |= AddrMaskMap[int_addr];                  //加入可能断开连接的终端
      return false;
    }
  }
                           
}

//和终端保持联系
void CenNode::keepcontact()                       
{
  if(NumTerAcs == 3 && NumTerPol == 3) return;            //跳过条件：所有终端入网且轮询数为3
  Message msg_s;
  msg_s.SAddr = (byte)addr;                               //源地址
  msg_s.Function = (byte)'4';                             //保持联系
  msg_s.DataLen = (byte)'1';                              //数据长度
  msg_s.MsgData = " ";

  for(int i = 1;i < 4;i++)                                
  {
    if(TerAcs & AddrMaskMap[i])
    {
      msg_s.DAddr = AddrMap[i];
      this->sendMessage(&msg_s);
      delay(100);
    }
  }
}

//检测终端
void CenNode::checkTer()
{
  Message msg_s;
  Message msg_r;
  Timer timer(0);
  msg_s.SAddr = (byte)'0';
  msg_s.DataLen = (byte)'1';
  (msg_s.MsgData) = " ";
  
//首先检查在断开连接时失败
  if(TerUnAcsFail != 0)
  {
    msg_s.Function = (byte)'0';                           //功能位为要求退网
    for(int i = 1;i < 4;i++)
    {
      if(TerUnAcsFail & AddrMaskMap[i])                   //仅对在退网时可能断开连接的终端执行
      {
        msg_s.DAddr = AddrMap[i];
        this->sendMessage(&msg_s);
        timer.new_start(MAX_WAIT_TIME);                   //开启定时器
        while(1)
        {
          if(this->recvMessage(&msg_r))
          {
            if(msg_r.DAddr == addr && msg_r.SAddr == AddrMap[i] && msg_r.Function == (byte)'0' && (msg_r.MsgData).charAt(0) == CHAR_ACK)
            {
              //确认收到ACK
              TerUnAcsFail &= ~AddrMaskMap[i];            //退出退网时可能断开连接的终端位图
              break; 
            }
          }
          if(timer.result())
          {
            //计时到
            TerUnAcsFail &= ~AddrMaskMap[i];              //计时到还未收到回复，则认为终端已经退网
            break;
          }
        }
      }
    }
  }

//检查在请求连接时失败
  if(TerAcsFail != 0)
  {
    msg_s.Function = (byte)'3';                           //询问存在
    for(int i = 1;i < 4;i++)
    {
      if(TerAcsFail & AddrMaskMap[i])                     //仅对在在网时可能断开连接的终端执行
      {
        msg_s.DAddr = AddrMap[i];
        this->sendMessage(&msg_s);
        timer.new_start(MAX_WAIT_TIME);                   //开启一个新计时器
        while(1)
        {
          if(this->recvMessage(&msg_r))
          {
            if(msg_r.DAddr == addr && msg_r.SAddr == AddrMap[i] && msg_r.Function == (byte)'3' && (msg_r.MsgData).charAt(0) == CHAR_ACK)
            {
              //确认为对应的ACK
              TerAcsFail &= ~AddrMaskMap[i];            //退出在网状态时可能断开连接位图
              TerAcs |= AddrMaskMap[i];                 //加入在网位图，继续保持接入
              NumTerAcs++;                              //接入数加一
              break;
            }
          }
          if(timer.result())                            
          {
            TerAcsFail &= ~AddrMaskMap[i];              //计时器到还未收到回复，认为终端已经退网
            break;
          }
        }
      }
    }
  }
  
}

//开始广播
void CenNode::broadcast()
{
  if(NumTerAcs == 3) 
  {
    //Serial.println("没有广播，因为满了");
    return;
  }
  //跳过条件：系统的终端容量满了
  Message msg_s;
  Message msg_r;
  Timer timer(0);
  int int_addr;
  msg_s.DAddr = (byte)ADDRESS_BC;                     //目的地址设置为广播地址
  msg_s.SAddr = addr;
  msg_s.Function = (byte)'5';                         //功能位为广播
  msg_s.DataLen = (byte)'1';
  msg_s.MsgData = String(CHAR_ACS);                   //数据部分可以加入其它功能，这里只使用开始申请接入
  this->sendMessage(&msg_s);  
  timer.new_start(MAX_WAIT_TIME_BROADCAST);           //开始计时
  while(1)
  {
    if(this->recvMessage(&msg_r))
    {
      if(msg_r.DAddr == addr && msg_r.Function == (byte)'6' && (msg_r.MsgData) == ACS_PASSWORD)
      {
        //确认为申请入网报文
        int_addr = (int)msg_r.SAddr - (int)'0';       
        TerReqAcsTbl[int_addr - 1] = int_addr;        //将全部申请入网的终端号（且接入密码正确）
      }
    }
    if(timer.result())
    {
      return;
    }
  }
}

//通知接入情况：对相应的终端申请入网终端列表中的终端进行回复
void CenNode::notifiAcs()
{
  if(NumTerAcs == 3) 
  {
    //Serial.println("没有通知接入，因为满了");
    return;
  }
  //跳过条件：系统的终端容量满了
  Message msg_s;
  int int_addr;
  msg_s.SAddr = addr;                               
  msg_s.Function = (byte)'6';                       
  msg_s.DataLen = (byte)'1';
  (msg_s.MsgData) = CHAR_ACK;
  for(int i = 0;i < 3;i++)
  {
    int_addr = TerReqAcsTbl[i];

    if(int_addr != -1 && int_addr > 0 && int_addr <4)
    {
      //Serial.print("TerAcs:");
      //Serial.println(TerAcs);
      //Serial.println(TerAcs & AddrMaskMap[int_addr]);
      byte a = (TerAcs & AddrMaskMap[int_addr]);
      byte b = 0;
      if(a == b) 
      {
        TerAcs |= AddrMaskMap[int_addr];
        NumTerAcs++;
//        Serial.print("成功接入：1，");
//        Serial.print("现有接入：");
//        Serial.println(NumTerAcs);
      }
//      Serial.print("发送确认信息");
      msg_s.DAddr = AddrMap[int_addr];
      this->sendMessage(&msg_s);
    }
  }
  //清空列表
  for(int i = 0;i < 3;i++)
  {
    TerReqAcsTbl[i] = -1;
  }
}

//重置轮询表
void CenNode::polzero()
{
  NumTerPol = 0;
  for(int i = 0;i < 3;i++)
  {
    TerPolTbl[i] = -1;
  }
}

byte CenNode::generateMCS(Message msg)
{
  byte mcs;
  mcs = (msg.DAddr)^(msg.SAddr)^(msg.Function)^(msg.DataLen);
  return mcs;
}

bool CenNode::checkMCS(Message msg)
{
  if(msg.MCS == this->generateMCS(msg))
  {
    return true;
  }
  return false;
}

//生成奇偶检验序列(输入不包含FCS)
String CenNode::generateFCS(String cmd)
{
  int len = cmd.length();
  long int mybuffer = 0;
  String FCS;
  while(len)
  {
    mybuffer += (int)cmd.charAt(0);
    cmd.remove(0,1);
    len--;                          
  }
  FCS = (char)(mybuffer/128);
  FCS += (char)(mybuffer%128);
  return FCS;
}

//校验FCS（输入包含FCS）
bool  CenNode::checkFCS(String cmd)
{
  String FCS = String(cmd.charAt(0));
  FCS += cmd.charAt(0);
  cmd.remove(0,2);
  if(FCS == (this->generateFCS(cmd)))
  {
    return true;
  }
  return false;
}
