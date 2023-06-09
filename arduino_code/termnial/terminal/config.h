// @Author : Darrius Lei
// @Email  : darrius.lei@outlook.com
#ifndef CONFIG_H
#define CONFIG_H

#define LORA_SYNC_WORD 0xcd
#define SF 7

#define ADDRESS '0'
#define ADDRESS_1 '1'
#define ADDRESS_2 '2'
#define ADDRESS_3 '3'
//广播地址
#define ADDRESS_BC '4'
//同步位
#define CHAR_SYNC '#'
//结束位
#define CHAR_END '*'
//ACK
#define CHAR_ACK '@'
//请求接入网络信号
#define CHAR_ACS '^'
//数据读取管脚
//ADDRESS_1对应温度
//ADDRESS_2对应湿度
//ADDRESS_3对应光强
//也可自行更改代码Terminal.cpp中getData()函数更改功能
#define DATA_PIN A0
//中心节点
#define MAX_WAIT_TIME 500
#define MAX_WAIT_TIME_BROADCAST 2000
//终端
#define MAX_WAIT_TIME_ACS 1500
#define MAX_WAIT_TIME_TER 500
#define MAX_WAIT_TIME_CONTACT 10000
#define DELAY_SEND 50

#define ACS_PASSWORD "password"

#endif
