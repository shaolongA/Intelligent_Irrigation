#include "stm8l15x.h"
#include "stm8l15x_itc.h"
#include <stdio.h>
#include "rtc.h"
#include "eeprom.h"

extern void delay_ms(unsigned int n);
static vu32 RTCSecCounter=0;
_calendar_obj calendar;
//*************��زɼ�**************
//��زɼ����ڣ���λmin��
unsigned int BatCollectPeriod = 20;	//N2
//��زɼ���־
unsigned char GetBat_Flag=0;
//**************�������ɼ�***********
//�Ϸ��������ڣ���λmin��
unsigned int HeartBeatPeriod=60;		//N1
//�Ϸ�������־��
unsigned char HeartBeatFlag=0;
//*************��ˮ�ƻ�****************
//��ˮ�ƻ�ʹ�ܱ�־��Ϊ1ʹ�ܣ���ˮ�ƻ���Ч��Ϊ0����ˮ�ƻ���Ч��
unsigned char g_WaterPlanEnFlag[TOTAL_SPACE];
//��ˮ�ƻ���ʼʱ�䣬ͨ���·���ˮ�ƻ���ʼʱ���������ʱ����ó���sec��ֵ��
unsigned long g_WaterPlanStarttime[TOTAL_SPACE];
//��ˮ����ִ�е���ʱ����ͨ���·��Ľ�ˮ��ʱ���ó���sec,24Сʱ��
unsigned long g_WaterPlanTotaltime[TOTAL_SPACE];
//��ˮ���������ڣ�12Сʱ
unsigned long g_WaterPlanPeriod[TOTAL_SPACE];
//��ˮ������ʱ����10min
unsigned long g_WaterPlanONtime[TOTAL_SPACE];
//��ˮģʽ��Ϊ1�������Խ�ˮ��Ϊ0�����ν�ˮ����ˮ��������ˮ�ƻ���
extern unsigned char g_PlanMode[TOTAL_SPACE];
//���ν�ˮ�Ľ���ʱ��
//static unsigned long WaterOvertime[TOTAL_SPACE];
//***************�ֶ���ˮ************************
//�ֶ���ˮ��������λs
unsigned int ManualWateringCount = 0;
//***************��ˮ������״̬**********************
//��ˮ����ִ�б�־
unsigned char g_WaterOnFlag=0;
//��ˮ����������־
unsigned char g_WaterOffFlag=0;
//ˮ������״̬;Ϊ1���Ŵ�״̬��Ϊ0���Źض�״̬��
//�ڴ�ˮ������λ��־���ر�ˮ���������־��
unsigned char WaterState = 0;
//**********�ط�����********************
//�ط������1min��
unsigned int Resendtime=1*60;
//�ط���־��
unsigned char  ResendFlag=0;
//�ڿ�����ˮ�����жԴ��������ڲɼ��ı�־λ��
unsigned char WaterOnSenorFlag=0;
void RTC_CLOCK_Init(void)
{
	unsigned int retry = 0;
	CLK_LSEConfig(CLK_LSE_ON);				//��оƬ�ڲ��ĵ�������LSE
	while(retry>=5)
	{
		delay_ms(10);
		retry++;
	}
	retry=0;
	while(CLK_GetFlagStatus(CLK_FLAG_LSERDY) == RESET)// �ȴ������ȶ�
	{
		delay_ms(10);
		retry++;
		if(retry >= 200)
		break;
	}
	RTC_DeInit();
	CLK_PeripheralClockConfig(CLK_Peripheral_RTC , ENABLE);    //ʹ��ʵʱʱ��RTCʱ��
    
	if(retry >= 200)
	{
	CLK_LSEConfig(CLK_LSE_OFF);
				CLK_RTCClockConfig(CLK_RTCCLKSource_LSI ,		//ѡ���ⲿLSIʱ��Դ��ΪRTCʱ��Դ
				CLK_RTCCLKDiv_1				//����Ϊ1��Ƶ
					);
	}
	else
	{
		CLK_RTCClockConfig(CLK_RTCCLKSource_LSE , //ѡ���ⲿLSEʱ��Դ��ΪRTCʱ��Դ
						 CLK_RTCCLKDiv_1        //����Ϊ1��Ƶ
						 );
	}
	RTC_WakeUpCmd(DISABLE);
	RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);//16*2048=32768����ʱ��Ϊ1s
	RTC_ITConfig(RTC_IT_WUT, ENABLE);
	ITC_SetSoftwarePriority(RTC_IRQn, ITC_PriorityLevel_3);//���ȼ�
	RTC_SetWakeUpCounter(2047);
	RTC_WakeUpCmd(ENABLE);
}
void RTC_SetCounter(u32 counter)
{
	RTCSecCounter=counter;
}
u32 RTC_GetCounter(void)
{
	return RTCSecCounter;
}

void RTC_UpdateCounter(void)
{
	unsigned char i=0;
	RTCSecCounter++;
	//***************�ɼ��Ϸ�ʱ���ж�*********************************
	if(RTCSecCounter % (BatCollectPeriod*60L)==3)
	{
		GetBat_Flag=1;
	}
	if(RTCSecCounter % (HeartBeatPeriod*60L)==5)
	{
		HeartBeatFlag=1;
	}
	//*****************��ˮ�ƻ�ʱ���ж�**************************
	if(RTCSecCounter >=1262304000UL)	//RTC_GetSec(2010, 1, 1, 0, 0, 0)=1262304000;
	{
	for(i=0;i<TOTAL_SPACE;i++)
	{
		if(g_WaterPlanEnFlag[i] != 0)
		{
			//������ʼʱ�䣬ͬʱС��ֹͣʱ�������ڹ��������ڣ���ʱ��ˮ�ƻ���Ч��
			//���ｽˮֹͣʱ�佽ˮ�ƻ����(RTCSecCounter >= g_WaterPlanStarttime[i])&&
			if((RTCSecCounter < (g_WaterPlanStarttime[i] + g_WaterPlanTotaltime[i] )))
			{
//				g_WaterPlanEnFlag[i] = 1;
			}
			else
			{
				g_WaterPlanEnFlag[i] = 0;
				Update_EEPROM(i,g_WaterPlanEnFlag[i]);
			}
			//�ж��Ƿ񵽴ｽˮ���ڣ������ｽˮ��������н�ˮ��־��λ
			if((RTCSecCounter >= g_WaterPlanStarttime[i] )&&((RTCSecCounter - g_WaterPlanStarttime[i] )%g_WaterPlanPeriod[i]  == 0)&&(WaterState == 0)&&\
				(RTCSecCounter + g_WaterPlanONtime[i] < (g_WaterPlanStarttime[i] + g_WaterPlanTotaltime[i] )))
			{
				g_WaterOnFlag=1;
//				WaterOvertime[i]  = RTCSecCounter + g_WaterPlanONtime[i] ;
				ManualWateringCount=g_WaterPlanONtime[i];
                                if(g_PlanMode[i] == 0)
                                {
                                    g_WaterPlanEnFlag[i] = 0;
                                    Update_EEPROM(i,g_WaterPlanEnFlag[i]);
                                }
			}
//			if(((RTCSecCounter == WaterOvertime[i] )||(g_WaterPlanEnFlag[i] == 0))&&(WaterState == 1))
//			{
//				g_WaterOffFlag=1;
//			}
		}
	}
	}
	//***************�ֶ���ˮ�ƻ�ʱ���ж�***********************************
	if((ManualWateringCount != 0)&&(WaterState == 1))
	{
		ManualWateringCount--;
		if(ManualWateringCount == 0)
		{
			g_WaterOffFlag=1;
		}
		else
		{
		}
		//��ˮ�����жԲɼ����������ж�ȡ�����ﵽʪ��ֵ��ֹͣ��ˮ
		if(ManualWateringCount%60==15)
		{
			WaterOnSenorFlag=1;
		}
	}
}
//�ж��Ƿ������꺯��
//�·�   1  2  3  4  5  6  7  8  9  10 11 12
//����   31 29 31 30 31 30 31 31 30 31 30 31
//������ 31 28 31 30 31 30 31 31 30 31 30 31
//����:���
//���:������ǲ�������.1,��.0,����
u8 Is_Leap_Year(u16 year)
{
    if(year % 4 == 0) //�����ܱ�4����
    {
        if(year % 100 == 0)
        {
            if(year % 400 == 0)
			{
				return 1; //�����00��β,��Ҫ�ܱ�400����
			}
            else 
			{
				return 0;
			}
        }
        else return 1;
    }
    else return 0;
}
//����ʱ��
//�������ʱ��ת��Ϊ����
//��1970��1��1��Ϊ��׼
//1970~2099��Ϊ�Ϸ����
//����ֵ:0,�ɹ�;����:�������.
//�·����ݱ�
u8 const table_week[12] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5}; //���������ݱ�
const u8 mon_table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; //ƽ����·����ڱ�

u8 RTC_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)
{
	u16 t;
	u32 seccount = 0;
	if(syear < 1970 || syear > 2099)
	{
		return 1;
	}
	for(t = 1970; t < syear; t++)			//��������ݵ��������
	{
		if(Is_Leap_Year(t))
		{
			seccount += 31622400;			//�����������
		}
		else 
		{
			seccount += 31536000;			//ƽ���������
		}
	}
	smon -= 1;
	for(t = 0; t < smon; t++)				//��ǰ���·ݵ����������
	{
		seccount += (u32)mon_table[t] * 86400; //�·����������
		if(Is_Leap_Year(syear) && t == 1)
		{
			seccount += 86400; //����2�·�����һ���������
		}
	}
	seccount += (u32)(sday - 1) * 86400;	//��ǰ�����ڵ����������
	seccount += (u32)hour * 3600;			//Сʱ������
	seccount += (u32)min * 60;				//����������
	seccount += sec;						//�������Ӽ���ȥ
	RTC_SetCounter(seccount);				//����RTC��������ֵ
	return 0;
}
//������������ڼ�
//��������:���빫�����ڵõ�����(ֻ����1901-2099��)
//�������������������
//����ֵ�����ں�
u8 RTC_Get_Week(u16 year, u8 month, u8 day)
{
	u16 temp2;
	u8 yearH, yearL;

	yearH = year / 100;
	yearL = year % 100;
	// ���Ϊ21����,�������100
	if (yearH > 19)
	{
		yearL += 100;
	}
	// ����������ֻ��1900��֮���
	temp2 = yearL + yearL / 4;
	temp2 = temp2 % 7;
	temp2 = temp2 + day + table_week[month - 1];
	if (yearL % 4 == 0 && month < 3)
	{
		temp2--;
	}
	return(temp2 % 7);
}
//�õ���ǰ��ʱ��
//����ֵ:0,�ɹ�;����:�������.
u8 RTC_Get(void)
{
	static u16 daycnt = 0;
	u32 timecount = 0;
	u32 temp = 0;
	u16 temp1 = 0;
	timecount = RTC_GetCounter();
	temp = timecount / 86400;				//�õ�����(��������Ӧ��)
	if(daycnt != temp)						//����һ����
	{
		daycnt = temp;
		temp1 = 1970;						//��1970�꿪ʼ
		while(temp >= 365)
		{
			if(Is_Leap_Year(temp1))			//������
			{
				if(temp >= 366)
				{
					temp -= 366;			//�����������
				}
				else
				{
					temp1++;
					break;
				}
			}
			else temp -= 365;				//ƽ��
			temp1++;
		}
		calendar.w_year = temp1;			//�õ����
		temp1 = 0;
		while(temp >= 28)					//������һ����
		{
			if(Is_Leap_Year(calendar.w_year) && temp1 == 1) //�����ǲ�������/2�·�
			{
				if(temp >= 29)temp -= 29;	//�����������
				else break;
			}
			else
			{
				if(temp >= mon_table[temp1])temp -= mon_table[temp1]; //ƽ��
				else break;
			}
			temp1++;
		}
		calendar.w_month = temp1 + 1;	//�õ��·�
		calendar.w_date = temp + 1;  	//�õ�����
	}
	temp = timecount % 86400;     		//�õ�������
	calendar.hour = temp / 3600;     	//Сʱ
	calendar.min = (temp % 3600) / 60; 	//����
	calendar.sec = (temp % 3600) % 60; 	//����
//	calendar.week = RTC_Get_Week(calendar.w_year, calendar.w_month, calendar.w_date); //��ȡ����
	return 0;
}

//����������ʱ��ת����sec����
unsigned long RTC_GetSec(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)
{
	u16 t;
	u32 seccount = 0;
	//�����s
	if(syear < 1970 || syear > 2099)
	{
		return 0;
	}
	for(t = 1970; t < syear; t++)			//��������ݵ��������
	{
		if(Is_Leap_Year(t))
		{
			seccount += 31622400;			//�����������
		}
		else 
		{
			seccount += 31536000;			//ƽ���������
		}
	}
	smon -= 1;
	for(t = 0; t < smon; t++)				//��ǰ���·ݵ����������
	{
		seccount += (u32)mon_table[t] * 86400; //�·����������
		if(Is_Leap_Year(syear) && t == 1)
		{
			seccount += 86400; //����2�·�����һ���������
		}
	}
	seccount += (u32)(sday - 1) * 86400;	//��ǰ�����ڵ����������
	seccount += (u32)hour * 3600;			//Сʱ������
	seccount += (u32)min * 60;				//����������
	seccount += sec;						//�������Ӽ���ȥ
	return seccount;
}

//��ʱ�����10min���������ӡ��Ϣ������Уʱ
void RTC_Adjust(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)
{
	u32 seccount = 0;
	static u32 seccountbuf = 0;
	seccount=RTC_GetSec(syear, smon, sday, hour, min, sec);
//	seccount += 8*60*60;					//���Ӹ�������ʱ��8Сʱ��ʱ�
	
	//��ȡRTC��������ֵ
	//����ȡ��secС������ֵ�����Ҳ����10*60s��Уʱ
	//����ȡ��sec��������ֵ�����Ҳ����10*60s��Уʱ
	seccountbuf=RTC_GetCounter();
	if(((seccount>seccountbuf)&&(seccount-seccountbuf>600))||((seccountbuf>seccount)&&(seccountbuf-seccount>600)))
	{
		RTC_SetCounter(seccount);			//����RTC��������ֵ
	}
}

//������timecountת��Ϊ������ʱ���룻
//����ֵ:0,�ɹ�;����:�������.
u8 RTC_Get_calendar(u32 timecount,_calendar_obj *prcalendar)
{
	u16 daycnt = 0;
	u32 temp = 0;
	u16 temp1 = 0;

	temp = timecount / 86400;				//�õ�����(��������Ӧ��)
	if(daycnt != temp)						//����һ����
	{
		daycnt = temp;
		temp1 = 1970;						//��1970�꿪ʼ
		while(temp >= 365)
		{
			if(Is_Leap_Year(temp1))			//������
			{
				if(temp >= 366)
				{
					temp -= 366;			//�����������
				}
				else
				{
					temp1++;
					break;
				}
			}
			else temp -= 365;				//ƽ��
			temp1++;
		}
		prcalendar->w_year = temp1;			//�õ����
		temp1 = 0;
		while(temp >= 28)					//������һ����
		{
			if(Is_Leap_Year(prcalendar->w_year) && temp1 == 1) //�����ǲ�������/2�·�
			{
				if(temp >= 29)temp -= 29;	//�����������
				else break;
			}
			else
			{
				if(temp >= mon_table[temp1])temp -= mon_table[temp1]; //ƽ��
				else break;
			}
			temp1++;
		}
		prcalendar->w_month = temp1 + 1;	//�õ��·�
		prcalendar->w_date = temp + 1;  	//�õ�����
	}
	temp = timecount % 86400;     		//�õ�������
	prcalendar->hour = temp / 3600;     	//Сʱ
	prcalendar->min = (temp % 3600) / 60; 	//����
	prcalendar->sec = (temp % 3600) % 60; 	//����
	return 0;
}