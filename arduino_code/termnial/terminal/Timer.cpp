#include "Timer.h"
#include "subincludes.h"

//默认构造函数
Timer::Timer():flag(false),t_start(millis()),t_current(millis()){}

//构造函数
Timer::Timer(int t_ineterval_in):flag(false), t_start(millis()),t_current(millis()), t_ineterval(t_ineterval_in){}

void Timer::new_start(int t_ineterval_in)//重置计时
{
  flag = true;
  t_start = millis();
  t_ineterval = t_ineterval_in;
}

bool Timer::result()//判断定时是否到
{
  if(flag)
  {
    t_current = millis();
    if((t_current - t_start) >= t_ineterval)
    {
      flag = false; //关闭计时器
      return true;  //说明计时到了
    }
    else
      return false;
  }
}

void Timer::zero() //关闭计时器
{
  flag = false;
}
