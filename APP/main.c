#include "stm8l15x.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rtc.h"
#include "adc.h"
#include "frame.h"
#include "gprs.h"
#include "external.h"
#include "uart.h"
#include "eeprom.h"
#include "timer2.h"
#include "sensor.h"

#define NB_PRINTF	1
//看门狗
#define PORT_DOG_EN		GPIOB
#define PIN_DOG_EN		GPIO_Pin_0
#define PORT_FEED_DOG	GPIOD
#define PIN_FEED_DOG	GPIO_Pin_3

//开关阀动作
#define PORT_PWM1		GPIOD
#define PIN_PWM1		GPIO_Pin_7
#define PWM1_HIGH		GPIO_SetBits(PORT_PWM1, PIN_PWM1);
#define PWM1_LOW		GPIO_ResetBits(PORT_PWM1, PIN_PWM1);

#define PORT_PWM2		GPIOD
#define PIN_PWM2		GPIO_Pin_6
#define PWM2_HIGH		GPIO_SetBits(PORT_PWM2, PIN_PWM2);
#define PWM2_LOW		GPIO_ResetBits(PORT_PWM2, PIN_PWM2);

#define PORT_V12_EN		GPIOA
#define PIN_V12_EN		GPIO_Pin_5
#define V12_EN_HIGH		GPIO_SetBits(PORT_V12_EN, PIN_V12_EN);
#define V12_EN_LOW		GPIO_ResetBits(PORT_V12_EN, PIN_V12_EN);

#define PORT_V24_EN		GPIOA
#define PIN_V24_EN		GPIO_Pin_7
#define V24_EN_HIGH		GPIO_SetBits(PORT_V24_EN, PIN_V24_EN);
#define V24_EN_LOW		GPIO_ResetBits(PORT_V24_EN, PIN_V24_EN);
//***************全局变量声明************************
extern unsigned char g_WaterOnFlag;
extern unsigned char g_WaterOffFlag;
extern unsigned char WaterState;
extern unsigned int ManualWateringCount;
extern unsigned int Vbat_ValueInMain;
extern unsigned char GprsStopSendFlag;
extern unsigned char HumidityValue;
extern short TempValue;
extern unsigned char PressValue;
//在开阀浇水过程中对传感器定期采集的标志位；
extern unsigned char WaterOnSenorFlag;
//**************变量定义************************************
unsigned char SendAck_Flag=0;
unsigned char SendRecord_Flag=0;
//unsigned char SendEn_Flag=1;
//水阀操作状态标志,0未打开，1、2正常关阀操作，3异常无法打开水阀，4
unsigned char gValveState=0;
//日历形式记录开阀时间与关阀时间
_calendar_obj gValveOnCalendar,gValveOffCalendar;
//unsigned char g_RxInterruptFlag=0;
//按键key2按下标志位；
unsigned char gKey2=0;
unsigned int  Press_Time_Count1=0;


//下发控制参数
unsigned char N8 =2;		//按键按下N8秒有效，打开水阀；
unsigned char N9=2;		//按键按下N9秒有效，关闭水阀；
unsigned char N10=60;		//开水阀后N10秒自动关阀；
//浇水湿度的上限，大于此值禁止浇水开阀
unsigned char HumidityMAX=100;
//浇水压力的下限，小于此值禁止浇水开阀
unsigned char PressMIN=0;
//浇水电压下限，小于此值禁止浇水开阀
unsigned int Vbat_ValueMIN=3200;
void GpioInit(void)
{
	GPIO_Init(GPIOA , GPIO_Pin_All , GPIO_Mode_Out_PP_Low_Slow);
	GPIO_Init(GPIOB , GPIO_Pin_All , GPIO_Mode_Out_PP_Low_Slow);
	GPIO_Init(GPIOC , GPIO_Pin_All , GPIO_Mode_Out_PP_Low_Slow);
	GPIO_Init(GPIOC , GPIO_Pin_5|GPIO_Pin_6 , GPIO_Mode_In_FL_No_IT);
	GPIO_Init(GPIOD , GPIO_Pin_All , GPIO_Mode_Out_PP_Low_Slow);
	GPIO_Init(GPIOE , GPIO_Pin_All , GPIO_Mode_Out_PP_Low_Slow);
	GPIO_Init(GPIOF , GPIO_Pin_All , GPIO_Mode_Out_PP_Low_Slow);

	GPIO_Write(GPIOA, 0x00);
	GPIO_Write(GPIOB, 0x00);
	GPIO_Write(GPIOC, 0x60);
	GPIO_Write(GPIOD, 0x00);
	GPIO_Write(GPIOE, 0xC4);
	GPIO_Write(GPIOF, 0x00);
}
void FeedDog(void)
{
	unsigned char i=0;
	GPIO_SetBits(PORT_FEED_DOG, PIN_FEED_DOG);
	for(i=0;i<10;i++)
	{
		nop();
		nop();
	}
	GPIO_ResetBits(PORT_FEED_DOG, PIN_FEED_DOG);
}
void WDT_Init(void)
{
	GPIO_Init(PORT_DOG_EN , PIN_DOG_EN , GPIO_Mode_Out_PP_Low_Slow);
	GPIO_Init(PORT_FEED_DOG , PIN_FEED_DOG , GPIO_Mode_Out_PP_Low_Slow);
	GPIO_ResetBits(PORT_DOG_EN, PIN_DOG_EN);
	GPIO_ResetBits(PORT_FEED_DOG, PIN_FEED_DOG);
}
//------------------------------休眠---------------------------------------//  
void power_enter_sleep()
{
	for(;;)
	{
		FeedDog();
		if((gKey2 == 1)||(g_WaterOnFlag != 0)||(g_WaterOffFlag != 0)||(GetBat_Flag == 1)||\
			(HeartBeatFlag == 1)||(SendAck_Flag == 1)||(SendRecord_Flag == 1)||((WaterState==1)&&(WaterOnSenorFlag==1)))
		{
			break;
		}
		else
		{
			halt();
		}
		FeedDog();
	}
}
void power_exit_sleep()
{
	disableInterrupts();
	
	enableInterrupts();
}
void LED_Init(void)
{
    GPIO_Init(PORT_LED , PIN_LED , GPIO_Mode_Out_PP_Low_Slow);
	LED0_OFF();
}
void KEY2_Handle(void)
{
	unsigned char KEY_Status1=0x02;

	if(gKey2 == 1)
	{
		if((Press_Time_Count1/17)%2==0)
		{
			LED0_ON();
		}
		else
		{
			LED0_OFF();
		}
		KEY_Status1=ReadKeyVal;
		KEY_Status1=KEY_Status1&RepBat_KEY;

		//按键时长大于20秒，自动退出，防止卡死
		if((KEY_Status1==0x00)&&(Press_Time_Count1<=33*20))
		{
			Press_Time_Count1++;
			return ;
		}
//		{
//			delay_ms(30);
//			KEY_Status1=ReadKeyVal;
//			KEY_Status1=KEY_Status1&RepBat_KEY;
//			Press_Time_Count1++;
//		}
		//关阀状态下手动开阀；开发状态下手动关阀
//                #if	NB_PRINTF
//		RS485PrInit();
//		USART3_sendstr("....abcdefg....\r\n");
//		RS485PrDeinit();
//#endif
		if(WaterState==0)
		{
			if(Press_Time_Count1>=33*N8)
			{
				g_WaterOnFlag=1;
				ManualWateringCount = N10;
			}
		}
		else if(WaterState==1)
		{
			if(Press_Time_Count1>=33*N9)
			{
				g_WaterOffFlag=1;
			}
		}
		gKey2 = 0;
		Press_Time_Count1=0;
		LED0_OFF();
		TIM2_Stop();
	}else
	{
		TIM2_Stop();
	}
}
void V_SENSOR_ENABLE(void)
{
	V24_EN_HIGH;
}
void V_SENSOR_DISABLE(void)
{
	V24_EN_LOW;
}
void V12_Charge(void)
{
	unsigned int i=0;
	for(i=0;i<250;i++)	//第一次充电可以从0V充到10V，之后每次放电从12V消耗至8V左右；
	{
		V12_EN_HIGH;
		delay_ms(1);
		V12_EN_LOW;
		delay_ms(4);
		V12_EN_HIGH;
		delay_ms(1);
		V12_EN_LOW;
		delay_ms(4);
	}
}
void OpenValve(void)
{
	V12_Charge();
//        V12_Charge();
	PWM1_LOW;
	PWM2_HIGH;
	delay_ms(20);
	PWM2_LOW;
#if	NB_PRINTF
		RS485PrInit();
		USART3_sendstr("....OpenValve....\r\n");
		RS485PrDeinit();
#endif
}
void CloseValve(void)
{
	V12_Charge();
//        V12_Charge();
	PWM2_LOW;
	PWM1_HIGH;
	delay_ms(20);
	PWM1_LOW;
#if	NB_PRINTF
		RS485PrInit();
		USART3_sendstr("....CloseValve....\r\n");
		RS485PrDeinit();
#endif
}
extern unsigned char SOURCE_ID[];
//下发控制参数
extern unsigned char N8;		//按键按下N8秒有效，打开水阀；
extern unsigned char N9;		//按键按下N9秒有效，关闭水阀；
extern unsigned char N10;	//开水阀后N10秒自动关阀；
//浇水湿度的上限，大于此值禁止浇水开阀
extern unsigned char HumidityMAX;
//浇水压力的下限，小于此值禁止浇水开阀
extern unsigned char PressMIN;
//电池采集周期，单位min；
extern unsigned int BatCollectPeriod;	//N2
//上发心跳周期，单位min；
extern unsigned int HeartBeatPeriod;		//N1
//浇水电压下限，小于此值禁止浇水开阀
extern unsigned int Vbat_ValueMIN;



//ASL...........修改了本字节的意义，为0浇水计划失效，不为0浇水计划有效
extern unsigned char g_WaterPlanEnFlag[TOTAL_SPACE] ;
//浇水计划起始时间，通过下发浇水计划起始时间的年月日时分秒得出的sec的值；
extern unsigned long g_WaterPlanStarttime[TOTAL_SPACE];
//浇水动作执行的总时长，单位为天，1-365，通过下发的浇水总时长得出的sec,24小时；
extern unsigned long g_WaterPlanTotaltime[TOTAL_SPACE];
//浇水动作的周期，单位小时，1-720
extern unsigned long g_WaterPlanPeriod[TOTAL_SPACE];
//浇水动作的时长，单位分钟，1-360
extern unsigned long g_WaterPlanONtime[TOTAL_SPACE];
//浇水模式；为1，周期性浇水；为0，单次浇水，浇水完成清除浇水计划；
extern unsigned char g_PlanMode[TOTAL_SPACE];
void PrintReboot(void)
{
	USART3_Init(115200);
	RS485PrInit();
	RS485PrintStr("System Reboot,V1.00_20190215\r\n",sizeof("System Reboot,V1.00_20190215\r\n")-1);
        RS485PrDeinit();
}

void PrintInfor(void)
{
	unsigned char i=0;
	unsigned char data[80];
	unsigned int buf=0;
    _calendar_obj calendarbuf;
	USART3_Init(115200);
	RS485PrInit();
//	RS485PrintStr("System Reboot,V1.00_20190215\r\n",sizeof("System Reboot,V1.00_20190215\r\n")-1);
	//源地址
	RS485PrintStr("Local_ID:",sizeof("Local_ID:")-1);
	memset(data,0,sizeof(data));
	for(i=0;i<6;i++)
	{
		data[2*i] = SOURCE_ID[i]/16;
		data[2*i+1] = SOURCE_ID[i]&0x0f;
	}
	for(i=0;i<12;i++)
	{
		if(data[i]<=9)
		{
			data[i]+='0';
		}else if((data[i]>=10)&&(data[i]<=15))
		{
			data[i]+='A';
		}else break;
	}
	RS485PrintStr(data,12);
	RS485PrintStr("\r\n",2);
	//配置信息；
	//电池电压下限
	RS485PrintStr("Vbat_ValueMIN:",sizeof("Vbat_ValueMIN:")-1);
	memset(data,0,sizeof(data));
	buf=Vbat_ValueMIN;
	for(i=0;i<6;i++)
	{
		data[5-i]=buf%10+'0';
		buf/=10;
	}
	RS485PrintStr(data,6);
	RS485PrintStr("mV\r\n",4);
	//心跳周期
	RS485PrintStr("HeartBeatPeriod:",sizeof("HeartBeatPeriod:")-1);
	memset(data,0,sizeof(data));
	buf=HeartBeatPeriod;
	for(i=0;i<6;i++)
	{
		data[5-i]=buf%10+'0';
		buf/=10;
	}
	RS485PrintStr(data,6);
	RS485PrintStr("min\r\n",5);
	//电池采集周期
	RS485PrintStr("BatCollectPeriod:",sizeof("BatCollectPeriod:")-1);
	memset(data,0,sizeof(data));
	buf=BatCollectPeriod;
	for(i=0;i<6;i++)
	{
		data[5-i]=buf%10+'0';
		buf/=10;
	}
	RS485PrintStr(data,6);
	RS485PrintStr("min\r\n",5);
	//湿度上限
	RS485PrintStr("HumidityMAX:",sizeof("HumidityMAX:")-1);
	memset(data,0,sizeof(data));
	buf=HumidityMAX;
	for(i=0;i<3;i++)
	{
		data[2-i]=buf%10+'0';
		buf/=10;
	}
	RS485PrintStr(data,3);
	RS485PrintStr("%RH\r\n",5);
	//压力下限
	RS485PrintStr("PressMIN:",sizeof("PressMIN:")-1);
	memset(data,0,sizeof(data));
	buf=PressMIN;
	for(i=0;i<3;i++)
	{
		data[2-i]=buf%10+'0';
		buf/=10;
	}
	RS485PrintStr(data,3);
	RS485PrintStr("0kPa\r\n",6);
	//N8
	RS485PrintStr("N8:",sizeof("N8:")-1);
	memset(data,0,sizeof(data));
	buf=N8;
	for(i=0;i<3;i++)
	{
		data[2-i]=buf%10+'0';
		buf/=10;
	}
	RS485PrintStr(data,3);
	RS485PrintStr(".0s\r\n",5);
	//浇水计划
	RS485PrintStr("Plan_Total:",sizeof("Plan_Total:")-1);
	memset(data,0,sizeof(data));
	buf=ReadFreeSpace();
	for(i=0;i<3;i++)
	{
		data[2-i]=buf%10+'0';
		buf/=10;
	}
	RS485PrintStr(data,3);
	RS485PrintStr("\r\n",2);
    
	for(i=0;i<TOTAL_SPACE;i++)
	{
		memset(data,0,sizeof(data));
        //          计划序号-开始时间       -模式  -总时长-周期  -   时长；
        //           0       8             22      30             45
        memcpy(data,"PLAN -  /  /     :  :  -  Mode-   day-   hour-   min\r\n",\
            sizeof("PLAN -  /  /     :  :  -  Mode-   day-   hour-   min\r\n")-1);
		if(g_WaterPlanEnFlag[i] != 0)
		{
            data[4]=i+'0';
            RTC_Get_calendar(g_WaterPlanStarttime[i],&calendarbuf);
            data[6]=(calendarbuf.w_year%100)/10+'0';
            data[7]=calendarbuf.w_year%10+'0';
            data[9]=(calendarbuf.w_month%100)/10+'0';
            data[10]=calendarbuf.w_month%10+'0';
            data[12]=(calendarbuf.w_date%100)/10+'0';
            data[13]=calendarbuf.w_date%10+'0';
            
            data[15]=(calendarbuf.hour%100)/10+'0';
            data[16]=calendarbuf.hour%10+'0';
            data[18]=(calendarbuf.min%100)/10+'0';
            data[19]=calendarbuf.min%10+'0';
            data[21]=(calendarbuf.sec%100)/10+'0';
            data[22]=calendarbuf.sec%10+'0';
            
            data[24]=g_PlanMode[i]%10+'0';
            
            data[31]=(g_WaterPlanTotaltime[i]/3600/24%1000)/100+'0';
            data[32]=(g_WaterPlanTotaltime[i]/3600/24%100)/10+'0';
            data[33]=g_WaterPlanTotaltime[i]/3600/24%10+'0';
            
            data[38]=(g_WaterPlanPeriod[i]/3600%1000)/100+'0';
            data[39]=(g_WaterPlanPeriod[i]/3600%100)/10+'0';
            data[40]=g_WaterPlanPeriod[i]/3600%10+'0';
            
            data[46]=(g_WaterPlanONtime[i]/60%1000)/100+'0';
            data[47]=(g_WaterPlanONtime[i]/60%100)/10+'0';
            data[48]=g_WaterPlanONtime[i]/60%10+'0';
            RS485PrintStr(data,sizeof("PLAN -  /  /     :  :  -  Mode-   day-   hour-   min\r\n")-1);
		}
	}
	RS485PrDeinit();
}
void main(void)
{
	unsigned char flag=0;
	disableInterrupts();
    CLK_DeInit();
	CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);	//内部时钟为1分频 = 16Mhz 
    CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);
    CLK_HSICmd(ENABLE);
	GpioInit();
	LED_Init();
	WDT_Init();
	NB_Init();
	RTC_CLOCK_Init();
	External_Interrupt_Input_Init();
	enableInterrupts();
	Read_EEPROM();
#if NB_PRINTF
        PrintReboot();
	PrintInfor();
#endif
	CloseValve();
	while(1)
	{
		if((g_WaterOnFlag)&&(WaterState==0))
		{
			g_WaterOnFlag=0;
            #if	NB_PRINTF
					RS485PrInit();
					USART3_sendstr("OpenValveActJudge;\r\n");
					RS485PrDeinit();
			#endif
			//采集电压，判定是否足够浇水，若足够浇水则执行浇水动作，
			//否则不执行浇水动作，并上发电池电量或报警
			BatterVoltJudgmentInmain();
			flag=GetSensorValue();
			#if	NB_PRINTF
					RS485PrInit();
					USART3_sendstr("Vbat_ValueInMain:");
					USART3_sendchar((Vbat_ValueInMain/1000)%10+'0');
					USART3_sendchar((Vbat_ValueInMain/100)%10+'0');
					USART3_sendchar((Vbat_ValueInMain/10)%10+'0');
					USART3_sendchar(Vbat_ValueInMain%10+'0');
					USART3_sendstr("mV\r\n");
					USART3_sendstr("GetSensorValue.flag:");
					USART3_sendchar(flag%10+'0');
					USART3_sendstr("	HumidityValue:");
					USART3_sendchar((HumidityValue/100)%10+'0');
					USART3_sendchar((HumidityValue/10)%10+'0');
					USART3_sendchar(HumidityValue%10+'0');
					USART3_sendstr("%RH\r\n");
					USART3_sendstr("	TemperatureValue:");
                    if(TempValue>=0)
                    {
                        USART3_sendchar((TempValue/100)%10+'0');
                        USART3_sendchar((TempValue/10)%10+'0');
                        USART3_sendchar(TempValue%10+'0');
                        USART3_sendstr("℃\r\n");
                    }
                    else
                    {
                        USART3_sendchar('-');
                        USART3_sendchar((abs(TempValue)/100)%10+'0');
                        USART3_sendchar((abs(TempValue)/10)%10+'0');
                        USART3_sendchar(abs(TempValue)%10+'0');
                        USART3_sendstr("℃\r\n");
                    }
					USART3_sendstr("	PressValue:");
					USART3_sendchar((PressValue/100)%10+'0');
					USART3_sendchar((PressValue/10)%10+'0');
					USART3_sendchar(PressValue%10+'0');
					USART3_sendstr("Bar\r\n");
					RS485PrDeinit();
			#endif
			//传感器采集正常，情况下读取暂存值，进行比较；
//			if((flag==0x03)&&(Vbat_ValueInMain >= Vbat_ValueMIN)&&(HumidityValue<HumidityMAX)&&(PressValue>=PressMIN))
			//才寄出来的PressValue单位为KPa；
			if(Vbat_ValueInMain >= Vbat_ValueMIN)//&&(HumidityValue<=HumidityMAX)&&(PressValue>=PressMIN))
			{
//                          unsigned char HumidityMAX=100;
//浇水压力的下限，小于此值禁止浇水开阀
//unsigned char PressMIN=0;
				OpenValve();
				WaterState=1;
				//记录开阀时间，开阀状态；
				gValveState = 4;
				RTC_Get();
				memcpy((char *)&gValveOnCalendar,(char *)&calendar,sizeof(calendar));
			}
			else
			{
				//开阀失败，记录失败原因，置位上发标志
				gValveState = 3;
				RTC_Get();
				memcpy((char *)&gValveOnCalendar,(char *)&calendar,sizeof(calendar));
				SendRecord_Flag = 1;
			}
		}
		if((WaterState==1)&&(WaterOnSenorFlag==1))
		{
			WaterOnSenorFlag=0;
			//出水反馈判定？
			//采集湿度，若超标则关端水阀；
			flag=GetSensorValue();
			//传感器采集正常，情况下读取暂存值，进行比较；
			#if	NB_PRINTF
					RS485PrInit();
					USART3_sendstr("GetSensorValue.flag:");
					USART3_sendchar(flag%10+'0');
					USART3_sendstr("	HumidityValue:");
					USART3_sendchar((HumidityValue/100)%10+'0');
					USART3_sendchar((HumidityValue/10)%10+'0');
					USART3_sendchar(HumidityValue%10+'0');
					USART3_sendstr("%RH\r\n");
					USART3_sendstr("	TemperatureValue:");
                    if(TempValue>=0)
                    {
                        USART3_sendchar((TempValue/100)%10+'0');
                        USART3_sendchar((TempValue/10)%10+'0');
                        USART3_sendchar(TempValue%10+'0');
                        USART3_sendstr("℃\r\n");
                    }
                    else
                    {
                        USART3_sendchar('-');
                        USART3_sendchar((abs(TempValue)/100)%10+'0');
                        USART3_sendchar((abs(TempValue)/10)%10+'0');
                        USART3_sendchar(abs(TempValue)%10+'0');
                        USART3_sendstr("℃\r\n");
                    }
					USART3_sendstr("	PressValue:");
					USART3_sendchar((PressValue/100)%10+'0');
					USART3_sendchar((PressValue/10)%10+'0');
					USART3_sendchar(PressValue%10+'0');
					USART3_sendstr("Bar\r\n");
					RS485PrDeinit();
			#endif
			if((HumidityValue>=HumidityMAX))//(flag==0x03)&&
			{
				g_WaterOffFlag=1;
			}
		}
		if(g_WaterOffFlag)
		{
			g_WaterOffFlag=0;
			//执行关阀动作
			CloseValve();
			WaterState=0;
			gValveState = 1;
			RTC_Get();
			memcpy((char *)&gValveOffCalendar,(char *)&calendar,sizeof(calendar));
			//置位浇水记录上发标志
			SendRecord_Flag = 1;
		}
		//周期采集标志等判定
		if(GetBat_Flag == 1)
		{
			GetBat_Flag=0;
			//采集电压及传感器，
			BatterVoltJudgmentInmain();
			#if	NB_PRINTF
					RS485PrInit();
					USART3_sendstr("Vbat_ValueInMain:");
					USART3_sendchar((Vbat_ValueInMain/1000)%10+'0');
					USART3_sendchar((Vbat_ValueInMain/100)%10+'0');
					USART3_sendchar((Vbat_ValueInMain/10)%10+'0');
					USART3_sendchar(Vbat_ValueInMain%10+'0');
					USART3_sendstr("mV\r\n");
					RS485PrDeinit();
			#endif
		}
		//上发标志判定
		if(GprsStopSendFlag != GPRS_SEND_LOCK)
		{
			if(SendRecord_Flag == 1)
			{
				SendRecord_Flag = 0;
				Send_Frame(3);
			}
			else if(HeartBeatFlag == 1)
			{
				HeartBeatFlag = 0;
				Send_Frame(1);
			}
			else if(SendAck_Flag == 1)
			{
				SendAck_Flag=0;
				Send_Frame(2);
			}
		}
		//当收到下发的数据下降沿事唤醒设备并开始读取数据直到数据下发完成
		//休眠、唤醒、喂狗判定是否有事件发生
		power_enter_sleep();
	}
}
