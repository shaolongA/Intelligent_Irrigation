#include "stm8l15x.h"
#include "eeprom.h"
#include "rtc.h"
#include <string.h>


//ASL...........�޸��˱��ֽڵ����壬Ϊ0��ˮ�ƻ�ʧЧ����Ϊ0��ˮ�ƻ���Ч
extern unsigned char g_WaterPlanEnFlag[TOTAL_SPACE] ;
//��ˮ�ƻ���ʼʱ�䣬ͨ���·���ˮ�ƻ���ʼʱ���������ʱ����ó���sec��ֵ��
extern unsigned long g_WaterPlanStarttime[TOTAL_SPACE];
//��ˮ����ִ�е���ʱ������λΪ�죬1-365��ͨ���·��Ľ�ˮ��ʱ���ó���sec,24Сʱ��
extern unsigned long g_WaterPlanTotaltime[TOTAL_SPACE];
//��ˮ���������ڣ���λСʱ��1-720
extern unsigned long g_WaterPlanPeriod[TOTAL_SPACE];
//��ˮ������ʱ������λ���ӣ�1-360
extern unsigned long g_WaterPlanONtime[TOTAL_SPACE];
//��ˮģʽ��Ϊ1�������Խ�ˮ��Ϊ0�����ν�ˮ����ˮ��������ˮ�ƻ���
unsigned char g_PlanMode[TOTAL_SPACE];


//�·����Ʋ���
extern unsigned char N8;		//��������N8����Ч����ˮ����
extern unsigned char N9;		//��������N9����Ч���ر�ˮ����
extern unsigned char N10;	//��ˮ����N10���Զ��ط���
//��ˮʪ�ȵ����ޣ����ڴ�ֵ��ֹ��ˮ����
extern unsigned char HumidityMAX;
//��ˮѹ�������ޣ�С�ڴ�ֵ��ֹ��ˮ����
extern unsigned char PressMIN;
//��زɼ����ڣ���λmin��
extern unsigned int BatCollectPeriod;	//N2
//�Ϸ��������ڣ���λmin��
extern unsigned int HeartBeatPeriod;		//N1
//��ˮ��ѹ���ޣ�С�ڴ�ֵ��ֹ��ˮ����
extern unsigned int Vbat_ValueMIN;
extern unsigned char SOURCE_ID[];

//eepromaddress = 0x1000;//STM8L051F3P6��EEPROM����ʼ��ַΪ��0x001000��������ַΪ��0x0010FF
//�н�ˮ�ƻ���ɺ�����˽�ˮ�ƻ��ı�־����Ҫ��EEPROM���и��£�
//i����ˮ�ƻ�����Ч��־д��data
void Update_EEPROM(unsigned char i,unsigned char data)
{
	EEPROM_Byte_Write(EEPROM_PLAN_ADDR + i*PLAN_BYTES_LEN, data);
}

//��ȡȫ���Ľ�ˮ�ƻ������·��Ŀ��Ʋ�����
void Read_EEPROM(void)
{
	unsigned char data[20];
	unsigned char i;
//	Update_EEPROM(0,1);
	//��ȡ������Ϣ��
	ReadConfigEEPROM();
	//��ȡSOURCE_ID
	memset(data,0,sizeof(data));
	EEPROM_Read_str(EEPROM_SOURCE_ID_ADDR, data, 6);
	memcpy( SOURCE_ID,data,6);
	for(i=0;i<TOTAL_SPACE;i++)
	{
		memset(data,0,sizeof(data));
		EEPROM_Read_str(EEPROM_PLAN_ADDR + i*PLAN_BYTES_LEN, data, PLAN_BYTES_LEN);
		g_WaterPlanEnFlag[i] = data[0];
		//data[0]��Ϊ0��������Ϣ��Ч,��ȡ��Ϣ�����洢�������У��������RTC�ж��н��бȽϣ�
		//data[0]Ϊ0�������ƻ��Ѿ�ʧЧ�����ٶ�ȡ��Ϣ����Ϣ��ֱ��д0
		if(g_WaterPlanEnFlag[i] != 0)
		{
			g_WaterPlanStarttime[i] = RTC_GetSec(2000 + data[6], data[5], data[4], data[3], data[2], data[1]);
			g_PlanMode[i] = data[7];
			g_WaterPlanTotaltime[i] = (data[8] + data[9]*256)*24*60*60L;
			g_WaterPlanPeriod[i] = (data[10] + data[11]*256)*60*60L;
			g_WaterPlanONtime[i] = (data[12] + data[13]*256)*60L;
		}
		else
		{
			g_WaterPlanStarttime[i] = 0;
			g_PlanMode[i] = 0;
			g_WaterPlanTotaltime[i] = 0;
			g_WaterPlanPeriod[i] = 0;
			g_WaterPlanONtime[i] = 0;
		}
	}
}
void Clear_PlanEEPROM(void)
{
	unsigned char data[20];
	unsigned char i;
	for(i=0;i<TOTAL_SPACE;i++)
	{
		memset(data,0,sizeof(data));
		EEPROM_Write_str(EEPROM_PLAN_ADDR + i*PLAN_BYTES_LEN, data, PLAN_BYTES_LEN);
	}
}
//�洢1����ˮ�ƻ���
void Write_EEPROM(unsigned char *data)
{
	//��¼��һ�δ洢��λ�ã��´�ֱ�ӴӴ˴���ʼ�鿴����������洢�����д�Ĵ���
	static unsigned char wr=0;
	unsigned long Starttime;
	unsigned long Totaltime;
	unsigned long Secbuf;
	unsigned char i=0;
	
	//��ˮ��Чֱ���˳�����ˮʱ���Ѿ�����ֱ���˳�
	if(data[0]!=0)
	{
		Starttime = RTC_GetSec(2000 + data[6], data[5], data[4], data[3], data[2], data[1]);
		Totaltime = (data[8] + data[9]*256)*24*60*60L;
		Secbuf = RTC_GetCounter();
		if(Secbuf > (Starttime+Totaltime)) return ;
	}
	else return ;
	//�洢��Ч�Ľ�ˮ�ƻ�
	for(i=0;i<TOTAL_SPACE;i++)
	{
		if(g_WaterPlanEnFlag[wr]==0)
		{
			EEPROM_Write_str(EEPROM_PLAN_ADDR + wr*PLAN_BYTES_LEN, data, PLAN_BYTES_LEN);
			
			g_WaterPlanEnFlag[wr] = data[0];
			g_WaterPlanStarttime[wr] = RTC_GetSec(2000 + data[6], data[5], data[4], data[3], data[2], data[1]);
			g_PlanMode[wr] = data[7];
			g_WaterPlanTotaltime[wr] = (data[8] + data[9]*256)*24*60*60L;
			g_WaterPlanPeriod[wr] = (data[10] + data[11]*256)*60*60L;
			g_WaterPlanONtime[wr] = (data[12] + data[13]*256)*60L;
			wr++;
			if(wr >= TOTAL_SPACE)wr=0;
			break;
		}
		else
		{
			wr++;
			if(wr >= TOTAL_SPACE)wr=0;
		}
	}
}
//��ȡ��ˮ�ƻ�ʣ��ռ�
unsigned char ReadFreeSpace(void)
{
	unsigned char i=0;
	unsigned char count=0;
	
	for(i=0;i<TOTAL_SPACE;i++)
	{
		if(g_WaterPlanEnFlag[i]==0) count++;
	}
	return count;
}
//��Ӧ��int����λ��ǰ����λ�ں�
//��ʽ			0-1				2				3				4-5				6-7
//����		Vbat_ValueMIN	HumidityMAX		PressMIN		HeartBeatPeriod		BatCollectPeriod
//����		��ص�����ֵ	ʪ����ֵ		ѹ����ֵ		��������			��زɼ�����
//			8					9
//			N8					N9
//	��������ȥ����ʱ��		�ط�ȥ����ʱ��
void ReadConfigEEPROM(void)
{
	unsigned char data[20];
	unsigned char flag=0;
	
	memset(data,0,sizeof(data));
	EEPROM_Read_str(EEPROM_CTRL_ADDR, data, CTRL_CFG_LEN);
	Vbat_ValueMIN = data[1]*256+data[0];
    if(Vbat_ValueMIN==0)
    {
    	Vbat_ValueMIN=3000;
		flag=1;
	}
	HumidityMAX =data[2];
    if(HumidityMAX==0)
	{
		HumidityMAX=10;
		flag=1;
	}
	PressMIN = data[3];
	HeartBeatPeriod = data[5]*256+data[4];
    if(HeartBeatPeriod==0)
	{
		HeartBeatPeriod=60;
		flag=1;
	}
	BatCollectPeriod = data[7]*256+data[6];
    if(BatCollectPeriod==0)
	{
		BatCollectPeriod=20;
		flag=1;
	}
	N8 = data[8];
    if(N8==0)
	{
		N8=2;
		flag=1;
	}
	N9 = data[9];
    if(N9==0)
	{
		N9=2;
		flag=1;
	}
	if(flag==1)
	{
		WriteConfigEEPROM();
	}
}
unsigned char WriteConfigEEPROM(void)
{
	unsigned char data[20];
	
	memset(data,0,sizeof(data));
	data[0] = Vbat_ValueMIN%256;
	data[1] = Vbat_ValueMIN/256;
	data[2] = HumidityMAX;
	data[3] = PressMIN;
	data[4] = HeartBeatPeriod%256;
	data[5] = HeartBeatPeriod/256;
	data[6] = BatCollectPeriod%256;
	data[7] = BatCollectPeriod/256;
	data[8] = N8;
	data[9] = N9;
	EEPROM_Write_str(EEPROM_CTRL_ADDR, data, CTRL_CFG_LEN);
	return 0;
}
/*******************************************************************************
**�������ƣ�EEPROM_Write_str(unsigned int address,unsigned char *date,unsigned char lenth)
**������������EEPROM�й̶���ַд��һ���ַ���
**��ڲ�����unsigned int address , unsigned char *date , unsigned char lenth
            address  ��Ҫд�����ݵĴ洢��ַ
            date     ��Ҫд�������
            lenth    ��д����ַ�����Ԫ�صĸ���

**�������
*******************************************************************************/
void EEPROM_Write_str(unsigned int address,unsigned char *date,unsigned char lenth)
{
	unsigned char i;
	for(i=0;i<lenth;i++)
	{
		EEPROM_Byte_Write(address+i,date[i]);
	}
}
/*******************************************************************************
**�������ƣ�EEPROM_Read_str(unsigned int address,unsigned char *date,unsigned char lenth)
**������������EEPROM�й̶���ַ����һ���ַ���
**��ڲ�����unsigned int address , unsigned char *date , unsigned char lenth
            address  ��Ҫ��ȡ���ݵĴ洢��ַ
            date     ��Ҫ������������
            lenth    �������ַ�����Ԫ�صĸ���

**�������
*******************************************************************************/
void EEPROM_Read_str(unsigned int address,unsigned char *date,unsigned char lenth)
{
	unsigned char i;
	for(i=0;i<lenth;i++)
	{
		date[i]=FLASH_ReadByte(address+i);
	}
}
/*******************************************************************************
**�������ƣ�void EEPROM_Byte_Write(unsigned int address , unsigned char date)
**������������EEPROM�й̶���ַд��һ���ֽ�����
**��ڲ�����unsigned int address , unsigned char date
            address  ��Ҫд�����ݵĴ洢��ַ
              date   ��һ���ֽ�����
**�������
*******************************************************************************/
void EEPROM_Byte_Write(unsigned int address , unsigned char date)
{
	unsigned int retry=0;
	FLASH_SetProgrammingTime(FLASH_ProgramTime_TProg);		//�趨���ʱ��Ϊ��׼���ʱ��
	//MASS ��Կ�����EEPROM�ı���
	FLASH_Unlock(FLASH_MemType_Data);
	FLASH_ProgramByte(address , date);						//������д����Ӧ�Ĵ洢��ַ
	while(FLASH_GetFlagStatus(FLASH_FLAG_EOP) == 1)
	{
		retry++;
		if(retry > 10000)break;
	}
	FLASH_Lock(FLASH_MemType_Data);
}











