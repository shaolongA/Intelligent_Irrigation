#include "stm8l15x.h"
#include "adc.h"
#include <stdio.h>
#include <string.h>
extern void delay_ms(unsigned int ms);
/*******************************************************************************
**�������ƣ�void ADC_Init()
**������������ʼ��ADC
**��ڲ�������
**�������
*******************************************************************************/
void ADCConver_Init(void)
{
	GPIO_Init(PORT_BAT_ADCCON, PIN_BAT_ADCCON, GPIO_Mode_Out_PP_High_Fast);	//����ADC�ɼ��Ŀ�������Ϊ���������
	GPIO_Init(PORT_BAT_ADCIN , PIN_BAT_ADCIN , GPIO_Mode_In_FL_No_IT);		//����PD->0 Ϊ�������룬���жϽ�ֹ
	CLK_PeripheralClockConfig(CLK_Peripheral_ADC1 , ENABLE);				//ʹ��ADC1ʱ��
	ADC_Init(ADC1,
			ADC_ConversionMode_Continuous,									//ASL......?����or����
			ADC_Resolution_12Bit,
			ADC_Prescaler_2
			);
	ADC_ChannelCmd(ADC1, ADC_Channel_22, ENABLE);
	ADC_Cmd(ADC1 , ENABLE);
	//�ر�AD�ɼ��ĵ�ѹ������MOS�ܡ��ɼ���ص�ѹǰ����Ҫ����ʹ�ܡ�
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
**�������ƣ�unsigned int ADC_Data_Read(void)
**������������ȡADC���һ��ģ��ת�����
**��ڲ�����unsigned int *AD_Value
            *AD_Value ->��ȡADC�������ݵ�ָ��
**�������
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
* ��������: void swap(void)
* ��������: �Ƚϴ�С
* ��    ��: a b
* ����  ֵ: ��
*****************************************************************************/
void swap(unsigned int *a, unsigned int *b)
{
	unsigned int c;
	c = *a;
	*a = *b;
	*b = c;
}
/*****************************************************************************
* ��������: void FindMedian(void)
* ��������: ��ƽ����
* ��    ��: MedianArr:�������� ��len������
* ����  ֵ: ��λ��		
Ҫ���׼ȷ�ȣ�����ȥ�������Сֵȡƽ������
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
**�������ƣ�unsigned int GetBatValueAve(void)
**�����������ɼ�10����ȥ�������С��ȡƽ��ֵ
**��ڲ�������
**�������
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
		ADCData[i] = ADC_Data_Read();  //�������ADCת������ȡת�����
		ADCValue += ADCData[i];
	}
	ADCValue = FindMedian(ADCData,	8);

	ADCConver_LowPower();
	return ((unsigned long)ADCValue*3300*3/8192);
}











