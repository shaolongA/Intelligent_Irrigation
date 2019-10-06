/*******************************************************************************
*                      组帧函数
********************************************************************************
*时间：2018-06-03
*******************************************************************************/
#include "stm8l15x.h"
#include "gprs.h"
#include "adc.h"
#include "rtc.h"
#include "eeprom.h"
#include <string.h>
//#include "function.h"
#include "frame.h"

unsigned char const SofeVerIntegral = 0x01;
unsigned char const SofeVerDecimal = 0x00;
static volatile unsigned int Vbat_Value=0;      //采集电池电压并记录

//流水号更改为全局变量，判定确认帧时比对是否相应确认帧
volatile unsigned short FrameCount = 0;            // 数据帧计数
//接收服务器下发命令的帧流水号并记录，用于终端确认帧回复时的流水号；
volatile unsigned int gRxFrameCount;
extern unsigned char Signal_Value;			    //采集信号强度并记录
unsigned char GprsStopSendFlag=GPRS_SEND_ENABLE;
unsigned int Vbat_ValueInMain=0;

extern unsigned char HumidityValue;
extern unsigned char PressValue;
unsigned char SOURCE_ID[6]={0x11,0x11,0x11,0x11,0x11,0x11};
const unsigned char TARGET_ID[6]={0,0,0,0,0,0};
//主循环中定期采样电池电压并记录；
void BatterVoltJudgmentInmain(void)
{
	static unsigned char LowVbatValueCnt=0;
	Vbat_ValueInMain=GetBatValueAve();
	//若电压低于3.1V，判定为电压过低将上发进行锁死；暂时未添加此限制；
	if(Vbat_ValueInMain<3200)
	{
		LowVbatValueCnt++;
		if(LowVbatValueCnt>5)
		{
			LowVbatValueCnt=6;
			GprsStopSendFlag=GPRS_SEND_LOCK;
		}
	}else
	{
		LowVbatValueCnt=0;
	}
}
void BatterVoltJudgmentInSend(void)
{
	static unsigned char LowVbatValueCnt=0;
	Vbat_Value=GetBatValueAve();
	//若电压低于3.1V，判定为电压过低将上发进行锁死；暂时未添加此限制；
	if(Vbat_Value<3100)
	{
		LowVbatValueCnt++;
		if(LowVbatValueCnt>5)
		{
			LowVbatValueCnt=6;
			GprsStopSendFlag=GPRS_SEND_LOCK;
		}
	}else
	{
		LowVbatValueCnt=0;
	}
}

void SendAnologyQuantity(void)
{
	unsigned int i;
	unsigned char Data_Frame[TX_MAX_LEN];
	unsigned int  Data_Count=0;
	unsigned char Check_Sum=0;
	unsigned int lenth = 18;
	memset(Data_Frame,0,sizeof(Data_Frame));
	// Header
	Data_Frame[0]=0xA5;
	Data_Frame[1]=0xA5;
	// Count
	Data_Frame[2]=FrameCount & 0xFF;
	Data_Frame[3]=(FrameCount >> 8) & 0xFF;

	// Version
	Data_Frame[4]=SofeVerIntegral;
	Data_Frame[5]=SofeVerDecimal;
	//时间标签	,读取内部RTC的时间。
	RTC_Get();
	Data_Frame[11]=calendar.w_year%100; //年份 0~99
	Data_Frame[10]=calendar.w_month;    //月份 1~12
	Data_Frame[9]=calendar.w_date;      //日 1~31
	Data_Frame[8]=calendar.hour;        //时 0~23
	Data_Frame[7]=calendar.min;         //分 0~59
	Data_Frame[6]=calendar.sec;         //秒 0~59
	Data_Count=12;
	//类型1-Zigbee,2-NB，3-WIFI，4-BlueTooth，5-other；
	Data_Frame[Data_Count++]=2;
	//1-GateWay，2-Control，3-Sensor；
	Data_Frame[Data_Count++]=2;
    //源地址
	memcpy(&Data_Frame[12+2], SOURCE_ID, 6);
//	//目标地址
//	memcpy(&Data_Frame[18+2], TARGET_ID, 6);
	Data_Count=20;
	//数据长度
	Data_Frame[Data_Count++]=lenth % 256;
	Data_Frame[Data_Count++]=lenth / 256;
	//命令类型
	Data_Frame[Data_Count++]=2;//控制单元的命令字节
	//类型标志符
	Data_Frame[Data_Count++]=1;//上传终端采集信息
	//信息对象数目
	Data_Frame[Data_Count++]=5;
	Data_Count=25;
	//信息体对象1
	//电池电压，
	Data_Frame[Data_Count++]=01;
	Data_Frame[Data_Count++]=02;
	Data_Frame[Data_Count++]=(Vbat_ValueInMain / 100) % 256;
	Data_Frame[Data_Count++]=(Vbat_ValueInMain / 100) / 256;

	//信号强度
	Data_Frame[Data_Count++]=02;
	Data_Frame[Data_Count++]=01;
	Data_Frame[Data_Count++]= Signal_Value;
	//土壤湿度
	Data_Frame[Data_Count++]=04;
	Data_Frame[Data_Count++]=01;
	Data_Frame[Data_Count++]= HumidityValue;
	//浇水计划空间
	Data_Frame[Data_Count++]=05;
	Data_Frame[Data_Count++]=01;
	Data_Frame[Data_Count++]= ReadFreeSpace();
	//水压
	Data_Frame[Data_Count++]=06;
	Data_Frame[Data_Count++]=01;
	Data_Frame[Data_Count++]= PressValue;
	
	//校验和
	Check_Sum=0;
	for(i=2;i<23+lenth;i++)
	{
		Check_Sum=Check_Sum + Data_Frame[i];
	}
	Data_Frame[23+lenth]=Check_Sum;
	Data_Frame[24+lenth]=0xbe;	
	Data_Frame[25+lenth]=0xef;
	BC95_Tx_Frame(Data_Frame,26+lenth);
	return ;
}
//回复确认帧
void SendAckFrame(unsigned char FrameCnt)
{
	unsigned int i;
	unsigned char Data_Frame[TX_MAX_LEN];
	unsigned char Check_Sum=0;
	unsigned int lenth = 0;
	// Header
	Data_Frame[0]=0xA5;
	Data_Frame[1]=0xA5;
	// Count
	Data_Frame[2]=FrameCnt & 0xFF;
	Data_Frame[3]=(FrameCnt >> 8) & 0xFF;
	// Version
	Data_Frame[4]=SofeVerIntegral;
	Data_Frame[5]=SofeVerDecimal;
	//时间标签	,读取内部RTC的时间。	
	RTC_Get();
	Data_Frame[11]=calendar.w_year%100; //年份 0~99
	Data_Frame[10]=calendar.w_month;    //月份 1~12
	Data_Frame[9]=calendar.w_date;      //日 1~31
	Data_Frame[8]=calendar.hour;        //时 0~23
	Data_Frame[7]=calendar.min;         //分 0~59
	Data_Frame[6]=calendar.sec;         //秒 0~59
	//类型1-Zigbee,2-NB，3-WIFI，4-BlueTooth，5-other；
	Data_Frame[12]=2;
	//1-GateWay，2-Control，3-Sensor；
	Data_Frame[13]=2;
    //源地址
	memcpy(&Data_Frame[12+2], SOURCE_ID, 6);
	//目标地址
//	memcpy(&Data_Frame[18+2], TARGET_ID, 6);
	//数据长度
	Data_Frame[26-6]=lenth % 256;
	Data_Frame[27-6]=lenth / 256;
	//命令类型
	Data_Frame[28-6]=3;//控制单元的命令字节,确认
	//校验和
	Check_Sum=0;
	for(i=2;i<29-6+lenth;i++)
	{		
		Check_Sum=Check_Sum + Data_Frame[i];
	}
	Data_Frame[29-6+lenth]=Check_Sum;
	Data_Frame[30-6+lenth]=0xbe;	
	Data_Frame[31-6+lenth]=0xef;
	BC95_Tx_Frame(Data_Frame,32-6+lenth);
	return ;
}
//水阀操作状态标志,0未打开，1、2正常关阀操作，3异常无法打开水阀，4
extern unsigned char gValveState;
//日历形式记录开阀时间与关阀时间
extern _calendar_obj gValveOnCalendar,gValveOffCalendar;
//发送浇水记录
void SendRecordFrame(void)
{
	unsigned int i;
	unsigned char Data_Frame[TX_MAX_LEN];
	unsigned char Check_Sum=0;
	unsigned int lenth = 0x0c;
	unsigned char Data_Count =0;
	unsigned long bufOn,bufOff;
	
	// Header
	Data_Frame[0]=0xA5;
	Data_Frame[1]=0xA5;
	// Count
	Data_Frame[2]=FrameCount & 0xFF;
	Data_Frame[3]=(FrameCount >> 8) & 0xFF;
	// Version
	Data_Frame[4]=SofeVerIntegral;
	Data_Frame[5]=SofeVerDecimal;
	//时间标签	,读取内部RTC的时间。
	RTC_Get();
	Data_Frame[11]=calendar.w_year%100; //年份 0~99
	Data_Frame[10]=calendar.w_month;    //月份 1~12
	Data_Frame[9]=calendar.w_date;      //日 1~31
	Data_Frame[8]=calendar.hour;        //时 0~23
	Data_Frame[7]=calendar.min;         //分 0~59
	Data_Frame[6]=calendar.sec;         //秒 0~59
	Data_Count=12;
	//类型1-Zigbee,2-NB，3-WIFI，4-BlueTooth，5-other；
	Data_Frame[Data_Count++]=2;
	//1-GateWay，2-Control，3-Sensor；
	Data_Frame[Data_Count++]=2;
    //源地址
	memcpy(&Data_Frame[12+2], SOURCE_ID, 6);
	//目标地址
//	memcpy(&Data_Frame[18+2], TARGET_ID, 6);
	Data_Count=26-6;
	//数据长度
	Data_Frame[Data_Count++]=lenth % 256;
	Data_Frame[Data_Count++]=lenth / 256;
	//命令类型
	Data_Frame[Data_Count++]=2;
	
	//类型标志符
	Data_Frame[Data_Count++]=2;
	//信息对象数目
	Data_Frame[Data_Count++]=1;
	Data_Count=31-6;
	//信息体对象1
	if(gValveState==4)
	{
		Data_Frame[Data_Count++]=01;
	}
	else if(gValveState==3)
	{
		Data_Frame[Data_Count++]=02;
	}else if(gValveState==1)
	{
		Data_Frame[Data_Count++]=0x10;
	}else
	{
		Data_Frame[Data_Count++]=00;
	}
	//浇水开始时间
	Data_Frame[Data_Count++]=gValveOnCalendar.sec;
	Data_Frame[Data_Count++]=gValveOnCalendar.min;
	Data_Frame[Data_Count++]=gValveOnCalendar.hour;
	Data_Frame[Data_Count++]=gValveOnCalendar.w_date;
	Data_Frame[Data_Count++]=gValveOnCalendar.w_month;
	Data_Frame[Data_Count++]=gValveOnCalendar.w_year%100;
	//浇水时长
	bufOn=RTC_GetSec(gValveOnCalendar.w_year, gValveOnCalendar.w_month, gValveOnCalendar.w_date, \
						gValveOnCalendar.hour, gValveOnCalendar.min, gValveOnCalendar.sec);
	bufOff=RTC_GetSec(gValveOffCalendar.w_year, gValveOffCalendar.w_month, gValveOffCalendar.w_date, \
						gValveOffCalendar.hour, gValveOffCalendar.min, gValveOffCalendar.sec);
	if(bufOff >= bufOn)
	{
		Data_Frame[Data_Count++] = (bufOff-bufOn)/60%256;
		Data_Frame[Data_Count++] = (bufOff-bufOn)/60/256;
	}
	else
	{
		Data_Frame[Data_Count++] = 0;
		Data_Frame[Data_Count++] = 0;
	}
	//关阀原因
	Data_Frame[Data_Count++] = gValveState;
	
	//校验和
	Check_Sum=0;
	for(i=2;i<29-6+lenth;i++)
	{		
		Check_Sum=Check_Sum + Data_Frame[i];
	}
	Data_Frame[29-6+lenth]=Check_Sum;
	Data_Frame[30-6+lenth]=0xbe;
	Data_Frame[31-6+lenth]=0xef;
	BC95_Tx_Frame(Data_Frame,32-6+lenth);
	return ;
}
/***********************************************************
发送一包数据，并LCD指示发送的过程
入口type 1：心跳上发  2：确认帧上发   3：浇水记录上发
出口     0：发送失败  1：联网注册成功	2：发送成功
***********************************************************/
unsigned char Send_Frame(unsigned char type)
{
	unsigned char flag = 0;
	unsigned char open_flag = 0;
	unsigned char i = 0;

	if(GprsStopSendFlag == GPRS_SEND_LOCK)
	{
	}
	else
	{
		OpenGPRS();
		for (i = 0; i < 3; i++)
		{
			open_flag = gprs_send_start();
			if (open_flag == 0)
			{
				ReadTimerFromServer();
				flag=1;
				for(unsigned char j=0;j<2;j++)
				{
					if(type == 1)
					{
						SendAnologyQuantity();
					}
					else if(type == 2)
					{
                                          delay_ms(2000);
//						SendAckFrame(gRxFrameCount);
                                          SendCompleteAction();
                                          delay_ms(2000);
                                          return 2;
					}
					else if(type == 3)
					{
						SendRecordFrame();
					}
					BatterVoltJudgmentInSend();
					open_flag = gprs_send_end();
//					return 2;
					if (open_flag == 2)//发送失败，延时等待，重新执行发送，接收操作
					{
						if(j == 0)
						{
							delay_ms(2000);
						}
						else if(i == 1)
						{
							FrameCount++;
							reset_gprs();
							break;
						}
					}else if(open_flag == 3)//接收失败，重新执行发送，接收操作
					{
						FrameCount++;
						reset_gprs();
						break;
					}else if (open_flag == 0)
					{
						FrameCount++;
						//SendedTimeStop();
						SendCompleteAction();
						delay_ms(2000);
						return 2;
					}
				}
			}
			//else
			//continue;
		}
		//SendedTimeStop();
		SendCompleteAction();
	}
	delay_ms(2000);
	return flag;
}



