#include "includes.h"

CenNode cn{};


int nta = 0;
void setup() 
{
  
  cn.NumTerAcs = 0;
  Serial.begin(9600);
  while (!Serial);                      //打开串口监视器才退出
  while(Serial.read() >= 0){}
  if (!LoRa.begin(417E6))               //设置频段 
  {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSyncWord(LORA_SYNC_WORD);     //设置同步字
  LoRa.setSpreadingFactor(SF);          //更改扩频因子
  LoRa.enableCrc();                     //开启底层CRC
//  //用于测试
//  int tpt[3] = {-1, 2, 1};
//  cn.test_init(tpt, true);
//  cn.NumTerAcs = 0;
}

void loop() 
{
  String cmd;                           //用于接收主机命令
  Message msg;                          //用于接收报文
  
  int i;
//第一阶段：接收主机命令
  while(1)
  {
    if(cn.recvCommmand(&cmd))           //有来自主机0的命令，读一条
    {
////      Serial.print("执行主机命令:");
////      Serial.println(cmd);
      cn.request_to_withdrew(&cmd);     //剔除设备命令
      cn.requestPol(&cmd);              //更新轮询表命令
      cn.perAcs(&cmd);                  //通知允许接入
      cn.requestCF(&cmd);               //更改频率
      
    }
    else
    {
      break;
    }
  }

  delay(50);
//第二阶段：轮询
  for(i = 0;i < 3;i++)
  {
    if(cn.TerPolTbl[i] == -1) continue;
    
    if(cn.TerAcs & cn.AddrMaskMap[cn.TerPolTbl[i]])
    {
      //设备已经接入系统
      if(cn.connectTer(cn.TerPolTbl[i]))
      {
//        Serial.println("已经建立连接，请求数据");
        delay(50);
        //成功建立连接
        cn.getData(cn.TerPolTbl[i]);    //获取数据
        delay(50);
      }
    }
    
  }

//第三阶段：向主机发送数据
  cn.sendFeedback();

//第四阶段：与接入终端保持联系
  cn.keepcontact();

//第五阶段：对可能掉线主机进行询问
  cn.checkTer();

//第六阶段：广播（开始申请接入）
  cn.broadcast();

//第七阶段：通知接入情况
  cn.notifiAcs();
}
