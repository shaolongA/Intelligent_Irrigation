#ifndef __TIMER2_H
#define __TIMER2_H
void TIM2_Init(void);//20ms进入一次
void TIM2_Stop(void);//关闭定时器中断
void TIM2_interrupt(void);
#endif
