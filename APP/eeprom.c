#include "stm8l15x.h"
#include "eeprom.h"
#include "rtc.h"
#include <string.h>


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
unsigned char g_PlanMode[TOTAL_SPACE];


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
extern unsigned char SOURCE_ID[];

//eepromaddress = 0x1000;//STM8L051F3P6的EEPROM的起始地址为：0x001000。结束地址为：0x0010FF
//有浇水计划完成后，清除此浇水计划的标志后，需要对EEPROM进行更新；
//i条浇水计划的有效标志写入data
void Update_EEPROM(unsigned char i,unsigned char data)
{
	EEPROM_Byte_Write(EEPROM_PLAN_ADDR + i*PLAN_BYTES_LEN, data);
}

//读取全部的浇水计划，及下发的控制参数；
void Read_EEPROM(void)
{
	unsigned char data[20];
	unsigned char i;
//	Update_EEPROM(0,1);
	//读取配置信息；
	ReadConfigEEPROM();
	//读取SOURCE_ID
	memset(data,0,sizeof(data));
	EEPROM_Read_str(EEPROM_SOURCE_ID_ADDR, data, 6);
	memcpy( SOURCE_ID,data,6);
	for(i=0;i<TOTAL_SPACE;i++)
	{
		memset(data,0,sizeof(data));
		EEPROM_Read_str(EEPROM_PLAN_ADDR + i*PLAN_BYTES_LEN, data, PLAN_BYTES_LEN);
		g_WaterPlanEnFlag[i] = data[0];
		//data[0]不为0，本条信息有效,读取信息，并存储在数组中，方便后续RTC中断中进行比较；
		//data[0]为0，则本条计划已经失效，不再读取信息，信息中直接写0
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
//存储1条浇水计划；
void Write_EEPROM(unsigned char *data)
{
	//记录上一次存储的位置，下次直接从此处开始查看，尽量均衡存储区域擦写的次数
	static unsigned char wr=0;
	unsigned long Starttime;
	unsigned long Totaltime;
	unsigned long Secbuf;
	unsigned char i=0;
	
	//浇水无效直接退出，浇水时间已经过了直接退出
	if(data[0]!=0)
	{
		Starttime = RTC_GetSec(2000 + data[6], data[5], data[4], data[3], data[2], data[1]);
		Totaltime = (data[8] + data[9]*256)*24*60*60L;
		Secbuf = RTC_GetCounter();
		if(Secbuf > (Starttime+Totaltime)) return ;
	}
	else return ;
	//存储有效的浇水计划
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
//读取浇水计划剩余空间
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
//对应的int，低位在前，高位在后；
//格式			0-1				2				3				4-5				6-7
//数据		Vbat_ValueMIN	HumidityMAX		PressMIN		HeartBeatPeriod		BatCollectPeriod
//意义		电池电量阈值	湿度阈值		压力阈值		心跳周期			电池采集周期
//			8					9
//			N8					N9
//	开阀按键去抖动时间		关阀去抖动时间
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
**函数名称：EEPROM_Write_str(unsigned int address,unsigned char *date,unsigned char lenth)
**功能描述：向EEPROM中固定地址写入一串字符串
**入口参数：unsigned int address , unsigned char *date , unsigned char lenth
            address  ：要写入数据的存储地址
            date     ：要写入的数组
            lenth    ：写入的字符串中元素的个数

**输出：无
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
**函数名称：EEPROM_Read_str(unsigned int address,unsigned char *date,unsigned char lenth)
**功能描述：从EEPROM中固定地址读出一串字符串
**入口参数：unsigned int address , unsigned char *date , unsigned char lenth
            address  ：要读取数据的存储地址
            date     ：要读出到的数组
            lenth    ：读的字符串中元素的个数

**输出：无
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
**函数名称：void EEPROM_Byte_Write(unsigned int address , unsigned char date)
**功能描述：向EEPROM中固定地址写入一个字节数据
**入口参数：unsigned int address , unsigned char date
            address  ：要写入数据的存储地址
              date   ：一个字节数据
**输出：无
*******************************************************************************/
void EEPROM_Byte_Write(unsigned int address , unsigned char date)
{
	unsigned int retry=0;
	FLASH_SetProgrammingTime(FLASH_ProgramTime_TProg);		//设定编程时间为标准编程时间
	//MASS 密钥，解除EEPROM的保护
	FLASH_Unlock(FLASH_MemType_Data);
	FLASH_ProgramByte(address , date);						//把数据写入相应的存储地址
	while(FLASH_GetFlagStatus(FLASH_FLAG_EOP) == 1)
	{
		retry++;
		if(retry > 10000)break;
	}
	FLASH_Lock(FLASH_MemType_Data);
}











