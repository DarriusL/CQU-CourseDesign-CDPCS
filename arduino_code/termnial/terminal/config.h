#ifndef CONFIG_H
#define CONFIG_H

#define LORA_SYNC_WORD 0xcd
#define SF 7

#define ADDRESS '0'
#define ADDRESS_1 '1'
#define ADDRESS_2 '2'
#define ADDRESS_3 '3'
#define ADDRESS_BC '4'
#define CHAR_SYNC '#'
#define CHAR_END '*'
#define CHAR_ACK '@'
#define CHAR_ACS '^'

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
