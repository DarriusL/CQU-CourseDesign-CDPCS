// @Author : Darrius Lei
// @Email  : darrius.lei@outlook.com
#ifndef SUBINCLUDES_H
#define SUBINCLUDES_H

#include <LoRa.h>
#include <SPI.h>
#include <arduino.h>
#include "config.h"

typedef struct MESSAGE
{
  byte DAddr;       //目的地址
  byte SAddr;       //源地址 
  byte Function;    //功能位
  byte DataLen;     //数据长度（二进制）：显示  (int)msg->DataLen - (int)'0'
  byte MCS;         //报文校验和（使用首地址，源地址，功能位，数据长度位）
  String MsgData;   //数据首地址
} Message;

#endif
