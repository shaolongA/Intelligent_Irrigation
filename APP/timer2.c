#include "stm8l15x.h"
#include "timer2.h"
//#include "usart1.h"

//定时器2通道1输入捕获配置
//arr,psc.
//Tout= ((arr+1)*(psc+1))/Tclk；	 
//Tclk： TIM3 的输入时钟频率（单位为 Mhz）。
//Tout： TIM3 溢出时间（单位为 s）。
//例子：	 TIM3_Int_Init(4999,7199);
//则，Tout=（（4900+1）*（7199+1））/72000000=0.5S
//即进入一次定时器中断的时间为0.5秒，即500ms。	 	 
//	TIM3_Int_Init(130,71);//10Khz的计数频率，计数到5000为500ms  
//TIM3_Int_Init(99,71);//100微秒进入一次。10Khz。	 
//TIM3_Int_Init(99,719);//1000微秒（1ms）进入一次。1Khz。
//TIM3_Int_Init(499,719);//5000微秒（5ms）进入一次。500hz。	 
//TIM3_Int_Init(999,719);//10000微秒(10ms进入一次。100hz。
//T2计数器打开的时间记录；
unsigned int Timier2Cnt=0;
//T2计数器打开关闭状态标志，为1打开，为0关闭；
unsigned int Timier2Flag=0;
//T3计数器打开的时间记录；
unsigned int Timier3Cnt=0;
//T3计数器打开关闭状态标志，为1打开，为0关闭；
unsigned int Timier3Flag=0;
/*******************************************************************************
**函数名称：void TIM2_Init()     Name: void TIM2_Init()
**功能描述：初始化定时器2
**入口参数：无
**输出：无
*******************************************************************************/
//void TIM2_Init(unsigned short arr,unsigned short psc)
void TIM2_Init(void)//1ms进入一次
{
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, ENABLE);
    TIM2_DeInit();
    // 配置Timer3相关参数，时钟为16/16 = 1MHZ，定时时间 = 1000/1000000 = 1ms
    TIM2_TimeBaseInit(TIM2_Prescaler_16, TIM2_CounterMode_Up, 30000);
    TIM2_ITConfig(TIM2_IT_Update, ENABLE);
    TIM2_Cmd(ENABLE);
	Timier2Cnt=0;
	Timier2Flag=1;
}

void TIM2_Stop(void)//关闭定时器中断
{
    TIM2_Cmd(DISABLE);
    TIM2_DeInit();
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, DISABLE);
    TIM2_Cmd(DISABLE);
	Timier2Cnt=0;
	Timier2Flag=0;
}
void KEY2_Handle(void);
void TIM2_interrupt(void)
{
	//T2计数器打开100s，未关闭则自动关闭；
//	if(Timier2Cnt>50*100)
//	{
//		TIM2_Stop();
//	}
//	else
//	{
//		
//	}
	KEY2_Handle();
}
/*******************************************************************************
**函数名称：void TIM3_Init()     Name: void TIM3_Init()
**功能描述：初始化定时器2
**入口参数：无
**输出：无
*******************************************************************************/
void TIM3_Init(void)
{
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM3, ENABLE);
    TIM3_DeInit();
    // 配置Timer3相关参数，时钟为16/16 = 1MHZ，定时时间 = 1000/1000000 = 1ms
    TIM3_TimeBaseInit(TIM3_Prescaler_16, TIM3_CounterMode_Up, 30000);
    TIM3_ITConfig(TIM3_IT_Update, ENABLE);
    TIM3_Cmd(ENABLE);
	Timier3Cnt=0;
	Timier3Flag=1;
}
void TIM3_Stop(void)//关闭定时器中断
{
    TIM3_Cmd(DISABLE);
    TIM3_DeInit();
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM3, DISABLE);
    TIM3_Cmd(DISABLE);
	Timier3Cnt=0;
	Timier3Flag=0;
}
void TIM3_interrupt(void)
{
	//T3计数器打开充电10s后开始进行开阀动作，未关闭则自动关闭；
	if(Timier3Cnt>50*100)
	{
		TIM3_Stop();
	}
	else
	{
		
	}
}


