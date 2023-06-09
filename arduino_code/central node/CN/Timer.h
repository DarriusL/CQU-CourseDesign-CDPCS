// @Author : Darrius Lei
// @Email  : darrius.lei@outlook.com
#include "subincludes.h"

#ifndef TIMER_H
#define TIMER_H
class Timer
{
public:
  unsigned long t_start;
  unsigned long t_current;
  unsigned long t_ineterval;

  Timer();//默认构造函数
  
  Timer(unsigned long t_ineterval_in);//构造函数

  void new_start(unsigned long t_ineterval_in);//重置计时

  bool result();//判断定时是否到

  void zero();  //关闭计时器
    
};

#endif
