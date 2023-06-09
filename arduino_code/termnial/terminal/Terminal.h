// @Author : Darrius Lei
// @Email  : darrius.lei@outlook.com
#ifndef TERMINAL_H
#define TERMNIAL_H

class Terminal
{
public:
  byte addr;                                                            //地址
  bool Acs;                                                             //是否接入了网络
  bool Acs_permit;                                                      //是否允许入网
  bool Link;                                                            //是否与主机建立了连接
  DHT11 DHT;
  
  Terminal(byte addr_in, bool acs);                                     //构造函数

  void sendMessage(Message* msg);                                       //发送报文（无线）
  bool recvMessage(Message* pmsg);                                      //接收报文（无线）

  void recvState();                                                     //接收状态

  bool requestAcs();                                                    //请求接入
  String getData();                                                      //获取数据
  byte generateMCS(Message msg);                                        //生成报文校验和
  bool checkMCS(Message msg);                                           //检验报文
  
};

#endif
