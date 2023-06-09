#include "Timer.h"
#include "subincludes.h"

//默认构造函数
Timer::Timer():t_start(millis()),t_current(millis()){}

//构造函数
Timer::Timer(unsigned long t_ineterval_in):t_start(millis()),t_current(millis()), t_ineterval(t_ineterval_in){}

void Timer::new_start(unsigned long t_ineterval_in)//重置计时
{
  t_start = millis();
  t_ineterval = t_ineterval_in;
}

bool Timer::result()//判断定时是否到
{

  t_current = millis();
  if((t_current - t_start) >= t_ineterval)
  {
    return true;  //说明计时到了
  }
  else
    return false;
}
