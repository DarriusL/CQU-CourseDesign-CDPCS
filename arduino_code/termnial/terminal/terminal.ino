// @Author : Darrius Lei
// @Email  : darrius.lei@outlook.com
#include "includes.h"

const char TerAddress = ADDRESS_3;

Terminal ter((byte)TerAddress, false);

void setup() 
{
  Serial.begin(9600);
  while (!Serial);                  //打开串口监视器才退出
  while(Serial.read() >= 0){}
  if (!LoRa.begin(417E6))           //设置频段 
  {
    Serial.println("Starting LoRa failed!");
    while (1);
  }  
  LoRa.setSyncWord(LORA_SYNC_WORD);  //设置同步字
  LoRa.setSpreadingFactor(SF);      //更改扩频因子
  LoRa.enableCrc();                 //开启底层CRCv
}

void loop() 
{
  ter.recvState();


//测试
//  Message msg;
//  if(ter.recvMessage(&msg))
//  {
//    
//  }
}
