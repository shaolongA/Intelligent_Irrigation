#include "stm8l15x.h"
#include "Sensor.h"
#include "uart.h"
#include <stdio.h>
#include <string.h>
//����ʪ�ȣ�
unsigned char HumidityValue=0;
//�����¶ȣ�
unsigned char TempValue=0;
//ˮѹ��
unsigned char PressValue=0;
stUart uart_sensor;
//��ȡ��ʪ������
//unsigned char readcmd10[]={0x01,0x03,0x00,0x12,0x00,0x04,0xE4,0x0C};//����������
unsigned char readcmd10[]={0x02,0x03,0x00,0x12,0x00,0x04,0xE4,0x3f};//����������
unsigned char readcmd11[]={0x01,0x03,0x00,0x00,0x00,0x01,0x84,0x0a};//ˮѹ
extern void delay_ms(unsigned int ms);

//�������ɼ���ʼ����
void SensorInit(void)
{
	GPIO_Init(PORT_V485_EN , PIN_V485_EN , GPIO_Mode_Out_PP_High_Fast);
	GPIO_Init(PORT_RS485_CTRL2 , PIN_RS485_CTRL2 , GPIO_Mode_Out_PP_High_Fast);
//	GPIO_Init(PORT_RS485_CTRL3 , PIN_RS485_CTRL3 , GPIO_Mode_Out_PP_High_Fast);
	V485_EN_HIGH;
	RS485_CTRL2_LOW;
//	RS485_CTRL3_LOW;
	USART2_Init(9600);
//	USART3_Init(9600);
}
//�������ɼ��͹��ģ�
void LowPower_Sensor(void)
{
	GPIO_Init(PORT_V485_EN , PIN_V485_EN , GPIO_Mode_Out_PP_High_Fast);
	GPIO_Init(PORT_RS485_CTRL2 , PIN_RS485_CTRL2 , GPIO_Mode_Out_PP_High_Fast);
//	GPIO_Init(PORT_RS485_CTRL3 , PIN_RS485_CTRL3 , GPIO_Mode_Out_PP_High_Fast);
	V485_EN_LOW;
	RS485_CTRL2_LOW;
//	RS485_CTRL3_LOW;
	USART2_LowPower();
//	USART3_LowPower();
}
void RS485U2_sendbuf(unsigned char *Str,unsigned int lenth)
{
	RS485_CTRL2_HIGH;
	delay_ms(1);
	USART2_sendbuf(Str,lenth);
//	delay_ms(1);
	RS485_CTRL2_LOW;
}
void RS485U3_sendbuf(unsigned char *Str,unsigned int lenth)
{
//	RS485_CTRL3_HIGH;
//	delay_ms(1);
	USART3_sendbuf(Str,lenth);
//	delay_ms(1);
//	RS485_CTRL3_LOW;
}
void RS485PrInit(void)
{
//	GPIO_Init(PORT_V485_EN , PIN_V485_EN , GPIO_Mode_Out_PP_High_Fast);
//	GPIO_Init(PORT_RS485_CTRL3 , PIN_RS485_CTRL3 , GPIO_Mode_Out_PP_High_Fast);
//	USART3_Init(115200);
//	V485_EN_HIGH;
//	RS485_CTRL3_HIGH;
}
void RS485PrDeinit(void)
{
//	V485_EN_LOW;
//	RS485_CTRL3_LOW;
//	USART3_LowPower();
}
void RS485PrintStr(unsigned char *Str,unsigned int lenth)
{
	RS485U3_sendbuf(Str,lenth);
}
void RS485PrintCh(unsigned char Ch)
{
	USART3_sendchar(Ch);
}
unsigned char UART5_COMAND_SCAN(unsigned char step);

void V_SENSOR_ENABLE(void);
void V_SENSOR_DISABLE(void);
void V12_Charge(void);
//��������ʱ��϶̣���ţ�ms
unsigned char GetSensorValue(void)
{
	//����������״̬��־��Ϊ0�ɼ�������Ч��Ϊ1�ɼ�������Ч��
	unsigned char flag=0;
//	V12_Charge();
	V_SENSOR_ENABLE();
	SensorInit();
	
	delay_ms(5000);
	for(unsigned char i=0;i<3;i++)
	{
		//ѹ����������ȡ����֮��5s���Ҳ����յ����ݣ�
		RS485U2_sendbuf(readcmd11,8);
		ResetUartBuf(&uart_sensor);
//		delay_ms(6000);
                delay_ms(1000);
		if(UART5_COMAND_SCAN(1)==1)
		{
			flag = 0x01;
			break;
		}
	}
	for(unsigned char i=0;i<3;i++)
	{
		RS485U2_sendbuf(readcmd10,8);
		//800MSʪ����ʾΪ0,����ʾ�������¶ȣ�2s����ʪ�Ȳɼ����ݵ���ʪ��ֵ����3s���ϲɼ�����ȷ��ֵ��
		ResetUartBuf(&uart_sensor);
                delay_ms(1000);
		if(UART5_COMAND_SCAN(2)==2)
		{
			flag |= 0x02;
			break;
		}
	}
	V_SENSOR_DISABLE();
	LowPower_Sensor();
	return flag;
}
unsigned int crc16(unsigned char *str,unsigned int num)//str�����ַ�����num����������Ԫ�ص�������
//����int�����ݣ����Ϊ2λ��	txd[6]=crc&0x00ff; txd[7]=crc>>8;
{
//	unsigned int crc16(unsigned char *str,unsigned int num); 
	unsigned int ddd,eee,bbb,crc; 
	crc=0xffff; 
	for (ddd=0;ddd<num;ddd++) 
	{ 
		bbb=str[ddd]&0x00ff; 
		crc^=bbb; 
		for (eee=0;eee<8;eee++) 
		{ 
			if (crc&0x0001) 
			{ 
				crc>>=1; 
				crc^=0xa001; 
			} 
			else 
				crc>>=1; 
		} 
	} 
	return(crc); 
}
unsigned char crc_check(unsigned char* data,unsigned char lenth)//crcУ�����ݡ�
{
	unsigned char test[20]={0x00};
	unsigned char i=0;
	unsigned int crc;
	if(lenth>20)return 0;
	for(i=0;i<lenth-2;i++)
	{
		test[i]=data[i];
	}
	crc=crc16(test,lenth-2); 
	test[lenth-2]=crc&0x00ff; 
	test[lenth-1]=crc>>8;	
	if( (test[lenth-1]==data[lenth-1])&&(test[lenth-2]==data[lenth-2]) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
void sensor_analyze(unsigned char* data,unsigned int size)//�������ɼ��������ݽ��н�����
{
// 0  1  2  3  4  5  6  7  8  9 10 11 12		
//01 03 08 01 5D 00 B9 00 00 00 00 45 05		
//01 5D//ʪ��
//00 B9//�¶�
	HumidityValue = (data[3]*256 +data[4])/10;
	TempValue = (data[5]*256 +data[6])/10;
}
void press_analyze(unsigned char* data,unsigned int size)
{
	//Ĭ�ϵ�λΪPa��
	PressValue = data[3]*256 +data[4];
}
//step==1��ˮѹ��������step==2��ʪ�ȴ�������
//return=0����ȡ����ʧ�ܣ�return 1����ȡˮѹ���ݳɹ���return 2����ȡʪ�����ݳɹ���
unsigned char UART5_COMAND_SCAN(unsigned char step)//ˢ�¶�ȡ����3���ݡ��鿴�Ƿ������������Ҫ���� 
{
	unsigned char crc_flag=0;
	unsigned int i=0;
	unsigned int j=0;
	unsigned int lenth=0;
	unsigned int buflenth=0;
	unsigned char UART5_RXD_analyze[RX_BUF_LEN];
	for(i=0;((step==1)&&(i<800))||((step==2)&&(i<100));i++)
	{
		delay_ms(10);
		lenth=UartGetRxLen(&uart_sensor);
		if((lenth>0)&&(lenth<50))
		{
			for(j=0;((buflenth!=lenth)&&(j<50));j++)			//��ֹ���ݶ�ȡ�б����
			{
				buflenth=lenth;
				delay_ms(10);
				lenth = UartGetRxLen(&uart_sensor);
			}
			UartRead(&uart_sensor, UART5_RXD_analyze, lenth);
			ResetUartBuf(&uart_sensor);
			crc_flag=crc_check(UART5_RXD_analyze,lenth);
			if(crc_flag==1)
			{
				if((UART5_RXD_analyze[0]==0x01)&&(UART5_RXD_analyze[1]==0x03)&&(UART5_RXD_analyze[2]==0x02))
				{
					press_analyze(UART5_RXD_analyze,lenth);
					return 1;
				}
				else if(UART5_RXD_analyze[0]==0x02)
				{
					sensor_analyze(UART5_RXD_analyze,lenth);//������ʪ��ֵ��
					return 2;
				}
				return 0;
			}
			else
			{
				lenth=0;
			}
		}
	}
	return 0;
}
/************************************************************************************/	









