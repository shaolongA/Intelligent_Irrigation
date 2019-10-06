#include "stm8l15x.h"
#include "adc.h"
#include <stdio.h>
#include <string.h>
extern void delay_ms(unsigned int ms);
/*******************************************************************************
**函数名称：void ADC_Init()
**功能描述：初始化ADC
**入口参数：无
**输出：无
*******************************************************************************/
void ADCConver_Init(void)
{
	GPIO_Init(PORT_BAT_ADCCON, PIN_BAT_ADCCON, GPIO_Mode_Out_PP_High_Fast);	//设置ADC采集的控制引脚为推挽输出。
	GPIO_Init(PORT_BAT_ADCIN , PIN_BAT_ADCIN , GPIO_Mode_In_FL_No_IT);		//设置PD->0 为悬空输入，并中断禁止
	CLK_PeripheralClockConfig(CLK_Peripheral_ADC1 , ENABLE);				//使能ADC1时钟
	ADC_Init(ADC1,
			ADC_ConversionMode_Continuous,									//ASL......?连续or单次
			ADC_Resolution_12Bit,
			ADC_Prescaler_2
			);
	ADC_ChannelCmd(ADC1, ADC_Channel_22, ENABLE);
	ADC_Cmd(ADC1 , ENABLE);
	//关闭AD采集的电压控制引MOS管。采集电池电压前，需要将其使能。
	ADCBat_OFF;
}
void ADCConver_LowPower(void)
{
	GPIO_Init(PORT_BAT_ADCIN , PIN_BAT_ADCIN , GPIO_Mode_Out_PP_High_Fast);
	GPIO_ResetBits(PORT_BAT_ADCIN , PIN_BAT_ADCIN);
	ADCBat_OFF;
	ADC_Cmd(ADC1 , DISABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_ADC1 , DISABLE);
}
/*******************************************************************************
**函数名称：unsigned int ADC_Data_Read(void)
**功能描述：读取ADC完成一次模数转换结果
**入口参数：unsigned int *AD_Value
            *AD_Value ->读取ADC采样数据的指针
**输出：无
*******************************************************************************/
unsigned int ADC_Data_Read(void)
{
	unsigned int retry=0;
	while(ADC_GetFlagStatus(ADC1 , ADC_FLAG_EOC) == 0)
	{
		retry++;
		if(retry > 10000)return 0;
	}
	ADC_ClearFlag(ADC1 , ADC_FLAG_EOC);
	return (ADC_GetConversionValue(ADC1));
}
/*****************************************************************************
* 函数名称: void swap(void)
* 功能描述: 比较大小
* 参    数: a b
* 返回  值: 无
*****************************************************************************/
void swap(unsigned int *a, unsigned int *b)
{
	unsigned int c;
	c = *a;
	*a = *b;
	*b = c;
}
/*****************************************************************************
* 函数名称: void FindMedian(void)
* 功能描述: 求平均数
* 参    数: MedianArr:左移数组 ，len：长度
* 返回  值: 中位数		
要提高准确度，可以去掉最大最小值取平均试试
*****************************************************************************/
unsigned int	FindMedian(unsigned int  *MedianArr,	unsigned char len)
{
	unsigned char	i,j;
	unsigned int  sum = 0;
	unsigned int  FindArr[30];

	if((len>sizeof(FindArr))||(len<=4))return 0;
	
	for(i = 0; i < len; i++)
	{
		FindArr[i] = MedianArr[i];
	}
	i = 0;
	j = 0;
	for(i = 0; i < len - 1; i++)
	{
		for(j = i + 1; j < len; j++)
		{
			if(FindArr[i] > FindArr[j])
			{
				swap(&FindArr[i], &FindArr[j]);
			}
		}
	}

	for(i = 2; i < len-2;i++)
	{
		sum += FindArr[i];
	}
	sum  /= (len - 4); 
	return	(unsigned int)sum;
}
/*******************************************************************************
**函数名称：unsigned int GetBatValueAve(void)
**功能描述：采集10个数去掉最大最小后取平均值
**入口参数：无
**输出：无
*******************************************************************************/
unsigned int GetBatValueAve(void)
{
	unsigned int ADCData[10];
	unsigned int ADCValue=0;
	unsigned char i=0;
	memset(ADCData,0,sizeof(ADCData));
	ADCConver_Init();
	
	ADCBat_ON;
	delay_ms(20);
	ADC_Cmd(ADC1 , ENABLE);
	ADC_SoftwareStartConv(ADC1);
	for(i=0;i<8;i++)
	{
		ADCData[i] = ADC_Data_Read();  //软件启动ADC转换并读取转换结果
		ADCValue += ADCData[i];
	}
	ADCValue = FindMedian(ADCData,	8);

	ADCConver_LowPower();
	return ((unsigned long)ADCValue*3300*3/8192);
}











