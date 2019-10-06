/*******************************************************************************
*                      ��֡����
********************************************************************************
*ʱ�䣺2018-06-03
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
static volatile unsigned int Vbat_Value=0;      //�ɼ���ص�ѹ����¼

//��ˮ�Ÿ���Ϊȫ�ֱ������ж�ȷ��֡ʱ�ȶ��Ƿ���Ӧȷ��֡
volatile unsigned short FrameCount = 0;            // ����֡����
//���շ������·������֡��ˮ�Ų���¼�������ն�ȷ��֡�ظ�ʱ����ˮ�ţ�
volatile unsigned int gRxFrameCount;
extern unsigned char Signal_Value;			    //�ɼ��ź�ǿ�Ȳ���¼
unsigned char GprsStopSendFlag=GPRS_SEND_ENABLE;
unsigned int Vbat_ValueInMain=0;

extern unsigned char HumidityValue;
extern unsigned char PressValue;
unsigned char SOURCE_ID[6]={0x11,0x11,0x11,0x11,0x11,0x11};
const unsigned char TARGET_ID[6]={0,0,0,0,0,0};
//��ѭ���ж��ڲ�����ص�ѹ����¼��
void BatterVoltJudgmentInmain(void)
{
	static unsigned char LowVbatValueCnt=0;
	Vbat_ValueInMain=GetBatValueAve();
	//����ѹ����3.1V���ж�Ϊ��ѹ���ͽ��Ϸ�������������ʱδ��Ӵ����ƣ�
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
	//����ѹ����3.1V���ж�Ϊ��ѹ���ͽ��Ϸ�������������ʱδ��Ӵ����ƣ�
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
	//ʱ���ǩ	,��ȡ�ڲ�RTC��ʱ�䡣
	RTC_Get();
	Data_Frame[11]=calendar.w_year%100; //��� 0~99
	Data_Frame[10]=calendar.w_month;    //�·� 1~12
	Data_Frame[9]=calendar.w_date;      //�� 1~31
	Data_Frame[8]=calendar.hour;        //ʱ 0~23
	Data_Frame[7]=calendar.min;         //�� 0~59
	Data_Frame[6]=calendar.sec;         //�� 0~59
	Data_Count=12;
	//����1-Zigbee,2-NB��3-WIFI��4-BlueTooth��5-other��
	Data_Frame[Data_Count++]=2;
	//1-GateWay��2-Control��3-Sensor��
	Data_Frame[Data_Count++]=2;
    //Դ��ַ
	memcpy(&Data_Frame[12+2], SOURCE_ID, 6);
//	//Ŀ���ַ
//	memcpy(&Data_Frame[18+2], TARGET_ID, 6);
	Data_Count=20;
	//���ݳ���
	Data_Frame[Data_Count++]=lenth % 256;
	Data_Frame[Data_Count++]=lenth / 256;
	//��������
	Data_Frame[Data_Count++]=2;//���Ƶ�Ԫ�������ֽ�
	//���ͱ�־��
	Data_Frame[Data_Count++]=1;//�ϴ��ն˲ɼ���Ϣ
	//��Ϣ������Ŀ
	Data_Frame[Data_Count++]=5;
	Data_Count=25;
	//��Ϣ�����1
	//��ص�ѹ��
	Data_Frame[Data_Count++]=01;
	Data_Frame[Data_Count++]=02;
	Data_Frame[Data_Count++]=(Vbat_ValueInMain / 100) % 256;
	Data_Frame[Data_Count++]=(Vbat_ValueInMain / 100) / 256;

	//�ź�ǿ��
	Data_Frame[Data_Count++]=02;
	Data_Frame[Data_Count++]=01;
	Data_Frame[Data_Count++]= Signal_Value;
	//����ʪ��
	Data_Frame[Data_Count++]=04;
	Data_Frame[Data_Count++]=01;
	Data_Frame[Data_Count++]= HumidityValue;
	//��ˮ�ƻ��ռ�
	Data_Frame[Data_Count++]=05;
	Data_Frame[Data_Count++]=01;
	Data_Frame[Data_Count++]= ReadFreeSpace();
	//ˮѹ
	Data_Frame[Data_Count++]=06;
	Data_Frame[Data_Count++]=01;
	Data_Frame[Data_Count++]= PressValue;
	
	//У���
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
//�ظ�ȷ��֡
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
	//ʱ���ǩ	,��ȡ�ڲ�RTC��ʱ�䡣	
	RTC_Get();
	Data_Frame[11]=calendar.w_year%100; //��� 0~99
	Data_Frame[10]=calendar.w_month;    //�·� 1~12
	Data_Frame[9]=calendar.w_date;      //�� 1~31
	Data_Frame[8]=calendar.hour;        //ʱ 0~23
	Data_Frame[7]=calendar.min;         //�� 0~59
	Data_Frame[6]=calendar.sec;         //�� 0~59
	//����1-Zigbee,2-NB��3-WIFI��4-BlueTooth��5-other��
	Data_Frame[12]=2;
	//1-GateWay��2-Control��3-Sensor��
	Data_Frame[13]=2;
    //Դ��ַ
	memcpy(&Data_Frame[12+2], SOURCE_ID, 6);
	//Ŀ���ַ
//	memcpy(&Data_Frame[18+2], TARGET_ID, 6);
	//���ݳ���
	Data_Frame[26-6]=lenth % 256;
	Data_Frame[27-6]=lenth / 256;
	//��������
	Data_Frame[28-6]=3;//���Ƶ�Ԫ�������ֽ�,ȷ��
	//У���
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
//ˮ������״̬��־,0δ�򿪣�1��2�����ط�������3�쳣�޷���ˮ����4
extern unsigned char gValveState;
//������ʽ��¼����ʱ����ط�ʱ��
extern _calendar_obj gValveOnCalendar,gValveOffCalendar;
//���ͽ�ˮ��¼
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
	//ʱ���ǩ	,��ȡ�ڲ�RTC��ʱ�䡣
	RTC_Get();
	Data_Frame[11]=calendar.w_year%100; //��� 0~99
	Data_Frame[10]=calendar.w_month;    //�·� 1~12
	Data_Frame[9]=calendar.w_date;      //�� 1~31
	Data_Frame[8]=calendar.hour;        //ʱ 0~23
	Data_Frame[7]=calendar.min;         //�� 0~59
	Data_Frame[6]=calendar.sec;         //�� 0~59
	Data_Count=12;
	//����1-Zigbee,2-NB��3-WIFI��4-BlueTooth��5-other��
	Data_Frame[Data_Count++]=2;
	//1-GateWay��2-Control��3-Sensor��
	Data_Frame[Data_Count++]=2;
    //Դ��ַ
	memcpy(&Data_Frame[12+2], SOURCE_ID, 6);
	//Ŀ���ַ
//	memcpy(&Data_Frame[18+2], TARGET_ID, 6);
	Data_Count=26-6;
	//���ݳ���
	Data_Frame[Data_Count++]=lenth % 256;
	Data_Frame[Data_Count++]=lenth / 256;
	//��������
	Data_Frame[Data_Count++]=2;
	
	//���ͱ�־��
	Data_Frame[Data_Count++]=2;
	//��Ϣ������Ŀ
	Data_Frame[Data_Count++]=1;
	Data_Count=31-6;
	//��Ϣ�����1
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
	//��ˮ��ʼʱ��
	Data_Frame[Data_Count++]=gValveOnCalendar.sec;
	Data_Frame[Data_Count++]=gValveOnCalendar.min;
	Data_Frame[Data_Count++]=gValveOnCalendar.hour;
	Data_Frame[Data_Count++]=gValveOnCalendar.w_date;
	Data_Frame[Data_Count++]=gValveOnCalendar.w_month;
	Data_Frame[Data_Count++]=gValveOnCalendar.w_year%100;
	//��ˮʱ��
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
	//�ط�ԭ��
	Data_Frame[Data_Count++] = gValveState;
	
	//У���
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
����һ�����ݣ���LCDָʾ���͵Ĺ���
���type 1�������Ϸ�  2��ȷ��֡�Ϸ�   3����ˮ��¼�Ϸ�
����     0������ʧ��  1������ע��ɹ�	2�����ͳɹ�
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
					if (open_flag == 2)//����ʧ�ܣ���ʱ�ȴ�������ִ�з��ͣ����ղ���
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
					}else if(open_flag == 3)//����ʧ�ܣ�����ִ�з��ͣ����ղ���
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



