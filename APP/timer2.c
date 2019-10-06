#include "stm8l15x.h"
#include "timer2.h"
//#include "usart1.h"

//��ʱ��2ͨ��1���벶������
//arr,psc.
//Tout= ((arr+1)*(psc+1))/Tclk��	 
//Tclk�� TIM3 ������ʱ��Ƶ�ʣ���λΪ Mhz����
//Tout�� TIM3 ���ʱ�䣨��λΪ s����
//���ӣ�	 TIM3_Int_Init(4999,7199);
//��Tout=����4900+1��*��7199+1����/72000000=0.5S
//������һ�ζ�ʱ���жϵ�ʱ��Ϊ0.5�룬��500ms��	 	 
//	TIM3_Int_Init(130,71);//10Khz�ļ���Ƶ�ʣ�������5000Ϊ500ms  
//TIM3_Int_Init(99,71);//100΢�����һ�Ρ�10Khz��	 
//TIM3_Int_Init(99,719);//1000΢�루1ms������һ�Ρ�1Khz��
//TIM3_Int_Init(499,719);//5000΢�루5ms������һ�Ρ�500hz��	 
//TIM3_Int_Init(999,719);//10000΢��(10ms����һ�Ρ�100hz��
//T2�������򿪵�ʱ���¼��
unsigned int Timier2Cnt=0;
//T2�������򿪹ر�״̬��־��Ϊ1�򿪣�Ϊ0�رգ�
unsigned int Timier2Flag=0;
//T3�������򿪵�ʱ���¼��
unsigned int Timier3Cnt=0;
//T3�������򿪹ر�״̬��־��Ϊ1�򿪣�Ϊ0�رգ�
unsigned int Timier3Flag=0;
/*******************************************************************************
**�������ƣ�void TIM2_Init()     Name: void TIM2_Init()
**������������ʼ����ʱ��2
**��ڲ�������
**�������
*******************************************************************************/
//void TIM2_Init(unsigned short arr,unsigned short psc)
void TIM2_Init(void)//1ms����һ��
{
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, ENABLE);
    TIM2_DeInit();
    // ����Timer3��ز�����ʱ��Ϊ16/16 = 1MHZ����ʱʱ�� = 1000/1000000 = 1ms
    TIM2_TimeBaseInit(TIM2_Prescaler_16, TIM2_CounterMode_Up, 30000);
    TIM2_ITConfig(TIM2_IT_Update, ENABLE);
    TIM2_Cmd(ENABLE);
	Timier2Cnt=0;
	Timier2Flag=1;
}

void TIM2_Stop(void)//�رն�ʱ���ж�
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
	//T2��������100s��δ�ر����Զ��رգ�
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
**�������ƣ�void TIM3_Init()     Name: void TIM3_Init()
**������������ʼ����ʱ��2
**��ڲ�������
**�������
*******************************************************************************/
void TIM3_Init(void)
{
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM3, ENABLE);
    TIM3_DeInit();
    // ����Timer3��ز�����ʱ��Ϊ16/16 = 1MHZ����ʱʱ�� = 1000/1000000 = 1ms
    TIM3_TimeBaseInit(TIM3_Prescaler_16, TIM3_CounterMode_Up, 30000);
    TIM3_ITConfig(TIM3_IT_Update, ENABLE);
    TIM3_Cmd(ENABLE);
	Timier3Cnt=0;
	Timier3Flag=1;
}
void TIM3_Stop(void)//�رն�ʱ���ж�
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
	//T3�������򿪳��10s��ʼ���п���������δ�ر����Զ��رգ�
	if(Timier3Cnt>50*100)
	{
		TIM3_Stop();
	}
	else
	{
		
	}
}


