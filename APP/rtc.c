#include "stm8l15x.h"
#include "stm8l15x_itc.h"
#include <stdio.h>
#include "rtc.h"
#include "eeprom.h"

extern void delay_ms(unsigned int n);
static vu32 RTCSecCounter=0;
_calendar_obj calendar;
//*************电池采集**************
//电池采集周期，单位min；
unsigned int BatCollectPeriod = 20;	//N2
//电池采集标志
unsigned char GetBat_Flag=0;
//**************传感器采集***********
//上发心跳周期，单位min；
unsigned int HeartBeatPeriod=60;		//N1
//上发心跳标志；
unsigned char HeartBeatFlag=0;
//*************浇水计划****************
//浇水计划使能标志；为1使能，浇水计划生效；为0，浇水计划无效；
unsigned char g_WaterPlanEnFlag[TOTAL_SPACE];
//浇水计划起始时间，通过下发浇水计划起始时间的年月日时分秒得出的sec的值；
unsigned long g_WaterPlanStarttime[TOTAL_SPACE];
//浇水动作执行的总时长，通过下发的浇水总时长得出的sec,24小时；
unsigned long g_WaterPlanTotaltime[TOTAL_SPACE];
//浇水动作的周期，12小时
unsigned long g_WaterPlanPeriod[TOTAL_SPACE];
//浇水动作的时长，10min
unsigned long g_WaterPlanONtime[TOTAL_SPACE];
//浇水模式；为1，周期性浇水；为0，单次浇水，浇水完成清除浇水计划；
extern unsigned char g_PlanMode[TOTAL_SPACE];
//本次浇水的结束时间
//static unsigned long WaterOvertime[TOTAL_SPACE];
//***************手动浇水************************
//手动浇水计数，单位s
unsigned int ManualWateringCount = 0;
//***************浇水动作与状态**********************
//浇水动作执行标志
unsigned char g_WaterOnFlag=0;
//浇水动作结束标志
unsigned char g_WaterOffFlag=0;
//水阀工作状态;为1阀门打开状态；为0阀门关断状态；
//在打开水阀后置位标志，关闭水阀后清零标志；
unsigned char WaterState = 0;
//**********重发控制********************
//重发间隔，1min；
unsigned int Resendtime=1*60;
//重发标志，
unsigned char  ResendFlag=0;
//在开阀浇水过程中对传感器定期采集的标志位；
unsigned char WaterOnSenorFlag=0;
void RTC_CLOCK_Init(void)
{
	unsigned int retry = 0;
	CLK_LSEConfig(CLK_LSE_ON);				//打开芯片内部的低速振荡器LSE
	while(retry>=5)
	{
		delay_ms(10);
		retry++;
	}
	retry=0;
	while(CLK_GetFlagStatus(CLK_FLAG_LSERDY) == RESET)// 等待振荡器稳定
	{
		delay_ms(10);
		retry++;
		if(retry >= 200)
		break;
	}
	RTC_DeInit();
	CLK_PeripheralClockConfig(CLK_Peripheral_RTC , ENABLE);    //使能实时时钟RTC时钟
    
	if(retry >= 200)
	{
	CLK_LSEConfig(CLK_LSE_OFF);
				CLK_RTCClockConfig(CLK_RTCCLKSource_LSI ,		//选择外部LSI时钟源作为RTC时钟源
				CLK_RTCCLKDiv_1				//设置为1分频
					);
	}
	else
	{
		CLK_RTCClockConfig(CLK_RTCCLKSource_LSE , //选择外部LSE时钟源作为RTC时钟源
						 CLK_RTCCLKDiv_1        //设置为1分频
						 );
	}
	RTC_WakeUpCmd(DISABLE);
	RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);//16*2048=32768唤醒时间为1s
	RTC_ITConfig(RTC_IT_WUT, ENABLE);
	ITC_SetSoftwarePriority(RTC_IRQn, ITC_PriorityLevel_3);//优先级
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
	//***************采集上发时间判定*********************************
	if(RTCSecCounter % (BatCollectPeriod*60L)==3)
	{
		GetBat_Flag=1;
	}
	if(RTCSecCounter % (HeartBeatPeriod*60L)==5)
	{
		HeartBeatFlag=1;
	}
	//*****************浇水计划时间判定**************************
	if(RTCSecCounter >=1262304000UL)	//RTC_GetSec(2010, 1, 1, 0, 0, 0)=1262304000;
	{
	for(i=0;i<TOTAL_SPACE;i++)
	{
		if(g_WaterPlanEnFlag[i] != 0)
		{
			//大于起始时间，同时小于停止时间则属于工作总周期，此时浇水计划有效；
			//到达浇水停止时间浇水计划清除(RTCSecCounter >= g_WaterPlanStarttime[i])&&
			if((RTCSecCounter < (g_WaterPlanStarttime[i] + g_WaterPlanTotaltime[i] )))
			{
//				g_WaterPlanEnFlag[i] = 1;
			}
			else
			{
				g_WaterPlanEnFlag[i] = 0;
				Update_EEPROM(i,g_WaterPlanEnFlag[i]);
			}
			//判定是否到达浇水周期，若到达浇水周期则进行浇水标志置位
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
	//***************手动浇水计划时间判定***********************************
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
		//浇水过程中对采集传感器进行读取，若达到湿度值，停止浇水
		if(ManualWateringCount%60==15)
		{
			WaterOnSenorFlag=1;
		}
	}
}
//判断是否是闰年函数
//月份   1  2  3  4  5  6  7  8  9  10 11 12
//闰年   31 29 31 30 31 30 31 31 30 31 30 31
//非闰年 31 28 31 30 31 30 31 31 30 31 30 31
//输入:年份
//输出:该年份是不是闰年.1,是.0,不是
u8 Is_Leap_Year(u16 year)
{
    if(year % 4 == 0) //必须能被4整除
    {
        if(year % 100 == 0)
        {
            if(year % 400 == 0)
			{
				return 1; //如果以00结尾,还要能被400整除
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
//设置时钟
//把输入的时钟转换为秒钟
//以1970年1月1日为基准
//1970~2099年为合法年份
//返回值:0,成功;其他:错误代码.
//月份数据表
u8 const table_week[12] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5}; //月修正数据表
const u8 mon_table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; //平年的月份日期表

u8 RTC_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)
{
	u16 t;
	u32 seccount = 0;
	if(syear < 1970 || syear > 2099)
	{
		return 1;
	}
	for(t = 1970; t < syear; t++)			//把所有年份的秒钟相加
	{
		if(Is_Leap_Year(t))
		{
			seccount += 31622400;			//闰年的秒钟数
		}
		else 
		{
			seccount += 31536000;			//平年的秒钟数
		}
	}
	smon -= 1;
	for(t = 0; t < smon; t++)				//把前面月份的秒钟数相加
	{
		seccount += (u32)mon_table[t] * 86400; //月份秒钟数相加
		if(Is_Leap_Year(syear) && t == 1)
		{
			seccount += 86400; //闰年2月份增加一天的秒钟数
		}
	}
	seccount += (u32)(sday - 1) * 86400;	//把前面日期的秒钟数相加
	seccount += (u32)hour * 3600;			//小时秒钟数
	seccount += (u32)min * 60;				//分钟秒钟数
	seccount += sec;						//最后的秒钟加上去
	RTC_SetCounter(seccount);				//设置RTC计数器的值
	return 0;
}
//获得现在是星期几
//功能描述:输入公历日期得到星期(只允许1901-2099年)
//输入参数：公历年月日
//返回值：星期号
u8 RTC_Get_Week(u16 year, u8 month, u8 day)
{
	u16 temp2;
	u8 yearH, yearL;

	yearH = year / 100;
	yearL = year % 100;
	// 如果为21世纪,年份数加100
	if (yearH > 19)
	{
		yearL += 100;
	}
	// 所过闰年数只算1900年之后的
	temp2 = yearL + yearL / 4;
	temp2 = temp2 % 7;
	temp2 = temp2 + day + table_week[month - 1];
	if (yearL % 4 == 0 && month < 3)
	{
		temp2--;
	}
	return(temp2 % 7);
}
//得到当前的时间
//返回值:0,成功;其他:错误代码.
u8 RTC_Get(void)
{
	static u16 daycnt = 0;
	u32 timecount = 0;
	u32 temp = 0;
	u16 temp1 = 0;
	timecount = RTC_GetCounter();
	temp = timecount / 86400;				//得到天数(秒钟数对应的)
	if(daycnt != temp)						//超过一天了
	{
		daycnt = temp;
		temp1 = 1970;						//从1970年开始
		while(temp >= 365)
		{
			if(Is_Leap_Year(temp1))			//是闰年
			{
				if(temp >= 366)
				{
					temp -= 366;			//闰年的秒钟数
				}
				else
				{
					temp1++;
					break;
				}
			}
			else temp -= 365;				//平年
			temp1++;
		}
		calendar.w_year = temp1;			//得到年份
		temp1 = 0;
		while(temp >= 28)					//超过了一个月
		{
			if(Is_Leap_Year(calendar.w_year) && temp1 == 1) //当年是不是闰年/2月份
			{
				if(temp >= 29)temp -= 29;	//闰年的秒钟数
				else break;
			}
			else
			{
				if(temp >= mon_table[temp1])temp -= mon_table[temp1]; //平年
				else break;
			}
			temp1++;
		}
		calendar.w_month = temp1 + 1;	//得到月份
		calendar.w_date = temp + 1;  	//得到日期
	}
	temp = timecount % 86400;     		//得到秒钟数
	calendar.hour = temp / 3600;     	//小时
	calendar.min = (temp % 3600) / 60; 	//分钟
	calendar.sec = (temp % 3600) % 60; 	//秒钟
//	calendar.week = RTC_Get_Week(calendar.w_year, calendar.w_month, calendar.w_date); //获取星期
	return 0;
}

//将格林威治时间转换成sec返回
unsigned long RTC_GetSec(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)
{
	u16 t;
	u32 seccount = 0;
	//换算成s
	if(syear < 1970 || syear > 2099)
	{
		return 0;
	}
	for(t = 1970; t < syear; t++)			//把所有年份的秒钟相加
	{
		if(Is_Leap_Year(t))
		{
			seccount += 31622400;			//闰年的秒钟数
		}
		else 
		{
			seccount += 31536000;			//平年的秒钟数
		}
	}
	smon -= 1;
	for(t = 0; t < smon; t++)				//把前面月份的秒钟数相加
	{
		seccount += (u32)mon_table[t] * 86400; //月份秒钟数相加
		if(Is_Leap_Year(syear) && t == 1)
		{
			seccount += 86400; //闰年2月份增加一天的秒钟数
		}
	}
	seccount += (u32)(sday - 1) * 86400;	//把前面日期的秒钟数相加
	seccount += (u32)hour * 3600;			//小时秒钟数
	seccount += (u32)min * 60;				//分钟秒钟数
	seccount += sec;						//最后的秒钟加上去
	return seccount;
}

//若时间相差10min则调整并打印信息，否则不校时
void RTC_Adjust(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)
{
	u32 seccount = 0;
	static u32 seccountbuf = 0;
	seccount=RTC_GetSec(syear, smon, sday, hour, min, sec);
//	seccount += 8*60*60;					//增加格林威治时间8小时的时差；
	
	//读取RTC计数器的值
	//若读取的sec小于设置值，并且差大于10*60s，校时
	//若读取的sec大于设置值，并且差大于10*60s，校时
	seccountbuf=RTC_GetCounter();
	if(((seccount>seccountbuf)&&(seccount-seccountbuf>600))||((seccountbuf>seccount)&&(seccountbuf-seccount>600)))
	{
		RTC_SetCounter(seccount);			//设置RTC计数器的值
	}
}

//将计数timecount转换为年月日时分秒；
//返回值:0,成功;其他:错误代码.
u8 RTC_Get_calendar(u32 timecount,_calendar_obj *prcalendar)
{
	u16 daycnt = 0;
	u32 temp = 0;
	u16 temp1 = 0;

	temp = timecount / 86400;				//得到天数(秒钟数对应的)
	if(daycnt != temp)						//超过一天了
	{
		daycnt = temp;
		temp1 = 1970;						//从1970年开始
		while(temp >= 365)
		{
			if(Is_Leap_Year(temp1))			//是闰年
			{
				if(temp >= 366)
				{
					temp -= 366;			//闰年的秒钟数
				}
				else
				{
					temp1++;
					break;
				}
			}
			else temp -= 365;				//平年
			temp1++;
		}
		prcalendar->w_year = temp1;			//得到年份
		temp1 = 0;
		while(temp >= 28)					//超过了一个月
		{
			if(Is_Leap_Year(prcalendar->w_year) && temp1 == 1) //当年是不是闰年/2月份
			{
				if(temp >= 29)temp -= 29;	//闰年的秒钟数
				else break;
			}
			else
			{
				if(temp >= mon_table[temp1])temp -= mon_table[temp1]; //平年
				else break;
			}
			temp1++;
		}
		prcalendar->w_month = temp1 + 1;	//得到月份
		prcalendar->w_date = temp + 1;  	//得到日期
	}
	temp = timecount % 86400;     		//得到秒钟数
	prcalendar->hour = temp / 3600;     	//小时
	prcalendar->min = (temp % 3600) / 60; 	//分钟
	prcalendar->sec = (temp % 3600) % 60; 	//秒钟
	return 0;
}