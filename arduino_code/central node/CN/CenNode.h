// @Author : Darrius Lei
// @Email  : darrius.lei@outlook.com
#include "subincludes.h"

#ifndef CENNODE_H
#define CENNODE_H

class CenNode
{
public:
  byte addr = (byte)ADDRESS;
  int NumTerAcs;                                                        //接入网络的终端的数量
  long int channelFre = 417E6;                                          //信道频率
  int TerPolTbl[3] = {-1, -1, -1};                                      //轮询终端编号集（为-1表示补位，无意义），有顺序
  int TerReqAcsTbl[3] = {-1, -1, -1};                                   //申请接入的终端列表
  int NumTerPol;                                                        //轮询的终端数量
  byte TerAcs = 0x00;                                                   //接入网络的终端映射表（0-对应本机，1~3对应终端），
  byte TerUnAcsFail = 0x00;                                             //可能断开连接的终端（不会出现在接入网络映射表中），在断开连接时失败
  byte TerAcsFail = 0x00;                                               //可能断开连接的终端（不会出现在接入网络映射表中），在请求连接时失败

  String TerData[3] = {"", "", ""};                                     //用于存来自终端的数据（读取时记得减1）
  const byte AddrMap[4] = {(byte)'0', (byte)'1', (byte)'2', (byte)'3'}; //地址位映射表
                                                                        //掩码表 
  const byte AddrMaskMap[4] = {(byte)0x00, (byte)0x01, (byte)0x02, (byte)0x04};                  

  CenNode();                                                            //默认构造（认为网络中没有终端接入）
  void test_init(int tpt[3], bool allAcs);                              //输入轮询终端编号集（用于测试）  
  
  void sendMessage(Message* msg);                                       //发送报文（无线）
  bool recvMessage(Message* pmsg);                                      //接收报文（无线）
  bool recvCommmand(String* pCommand);                                  //接收来自主机的命令（有线）
  void sendFeedback();                                                  //向主机发送命令（有线）  

  void request_to_withdrew(String *pCommand);                           //主机要求终端退网
  void requestPol(String* pCommand);                                    //主机要求的数据（更新轮询表）
  void perAcs(String* pCommand);                                        //主机同意终端申请入网
  void requestCF(String* pCommand);                                     //主机广播通知更改频率（对已经入网的）

  bool connectTer(int addr);                                            //中心节点与终端设备建立连接
  bool getData(int int_addr);                                           //获取对应数据

  void keepcontact();                                                   //和终端保持联系
  void checkTer();                                                      //对可能掉线主机询问
  void broadcast();                                                     //广播信息
  void notifiAcs();                                                     //通知接入情况

  void polzero();                                                       //重置轮询表
  byte generateMCS(Message msg);                                        //生成报文校验和
  bool checkMCS(Message msg);                                           //检验报文

  String generateFCS(String cmd);                                       //生成奇偶检验序列
  bool  checkFCS(String cmd);                                           //校验
};

#endif
