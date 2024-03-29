/*******************************************************************************
*                       模块驱动
********************************************************************************
*时间：2018-05-20
*******************************************************************************/
//硬件说明：

//--------------------------------------------------------------------------------//

/***********************头文件***************************************************/
#include "stm8l15x.h"
#include <string.h>
#include "gprs.h"
#include "uart.h"
#include "rtc.h"
#include "eeprom.h"

#define NB_PRINTF	1
/*********************变量定义*******************************************************/
const char ATOK[]="OK\r\n";
const char ATERROR[]="ERROR\r\n";
const char ATCCLK[]="+CCLK:";
const char ATCSQ[]="+CSQ: ";
const char ATTCPEN[]="+ESOC=";
const char ATCGATT[]="+CGATT:";
const char ATCGSN[]="+CGSN:";
const char ATCEREG[]="+CEREG: ";
const char ATNBAND[]="+NBAND:";
const char ATNSMI[]="+NSMI";
const char ATNNMI[]="+NNMI";
const char ATESONMI[]="+ESONMI=";
const char ATCGPADDR[]="+CGPADDR:";

static AT_link_ENUM NB_Link_step=NBLINK;
static unsigned char Nsonmi_Flag=0;					//1:发送数据；2发送数据成功；3:接收数据命令发出；0：已经将数据读出

static unsigned char NBRxdata[RX_BUF_LEN];			//将下发的数据从接收的NB数据中剥离出来
static unsigned int NB_Rxlen=0;
static unsigned char NBbuffer[RX_BUF_LEN];					//将数据转换为HEX格式
static unsigned int NBbuflenth=0;

static unsigned char CoapSendFlag=0;
static unsigned int CoapSendCnt=0;
static unsigned char SendSuccessedFlag=0;				//数据发送成功标志，为0发送失败，为1发送成功
extern unsigned char GprsStopSendFlag;

extern unsigned char SendAck_Flag;
extern unsigned int gRxFrameCount;
extern volatile unsigned short FrameCount;
volatile unsigned char Signal_Value=99;   //存储信号强度
//static unsigned char IMEI[15];          //存储IMEI
stUart uart_gprs;
/*********************函数声明******************************************************/

void delay_mss(unsigned int n)
{
	unsigned  short j,g;
	for(j=0;j<n;j++)
	{
		if(j%9 == 0)
		{
			FeedDog();
		}
		for(g=0;g<2000;g++)
		{
			asm("nop");
		}
	}
}
void delay_ms(unsigned int ms)
{
//	unsigned int j,i;
//	for(i=0;i<ms;i++)
//		for(j=0;j<10350;j++);
	delay_mss(ms);
}
/*****************************************************************************
* 函数名称: void GPRS_IO_Init()
* 功能描述: GPRS控制IO初始化
PC4---PWRKEY,默认输出低电平，关机；高电平，工作；
开机时通过拉低300ms以上开机，之后保持高电平；
进入休眠状态后通过拉低此端口进行模块唤醒；
PD2---NB_RTC_EINT，默认输出低电平，PSM模式下休眠唤醒；
* 参    数:
* 返回  值:
*****************************************************************************/
void GPRS_IO_Init(void)
{
	GPIO_Init(PORT_GPRS_PWREN , PIN_GPRS_PWREN , GPIO_Mode_Out_PP_Low_Slow);
	GPIO_Init(PORT_GPRS_PWRKEY , PIN_GPRS_PWRKEY , GPIO_Mode_Out_PP_Low_Slow);
	GPIO_Init(PORT_GPRS_RTC , PIN_GPRS_RTC , GPIO_Mode_Out_PP_Low_Slow);

    GPRS_PWREN_ON;
    GPRS_RTC_HIGH;
//    GPRS_PWRKEY_LOW;
//    delay_ms(500);
//    GPRS_PWRKEY_HIGH;
	reset_gprs();
}
//串口初始化
void NB_UsartInit(unsigned long baudrate)
{
    USART1_Init(baudrate);
}
void NB_UsartMode(unsigned char state);
void NB_Init(void)
{
	GPRS_IO_Init();
    NB_UsartMode(1);
//	NB_UsartInit(9600);
}
//NB串口发送数据
void NB_UsartSendstr(unsigned char *data , unsigned int strlen)
{
	LED0_ON();
    USART1_sendbuf(data,strlen);
	LED0_OFF();
}
/*****************************************************************************
* 函数名称: unsigned char OpenGPRS(void)
* 功能描述: GPRS启动检测函数
* 参    数: 1：准备  0：未准备好
* 返回  值:
*****************************************************************************/
unsigned char OpenGPRS(void)
{
    #if	NB_PRINTF
        RS485PrInit();
        USART3_sendstr("NB_OpenGPRS!\r\n");
        RS485PrDeinit();
    #endif
    NB_UsartInit(9600);
    GPRS_PWREN_ON;
    GPRS_RTC_HIGH;
    GPRS_PWRKEY_LOW;
    delay_ms(300);
    GPRS_PWRKEY_HIGH;
    return 1;
}
/*****************************************************************************
* 函数名称: void reset_gprs(void)
* 功能描述: 
* 参    数:
* 返回  值:
*****************************************************************************/
void reset_gprs(void)
{
    #if	NB_PRINTF
        RS485PrInit();
        USART3_sendstr("NB_Reset!\r\n");
        RS485PrDeinit();
    #endif
//	GPRS_PWREN_OFF;
    GPRS_PWRKEY_LOW;
    delay_ms(3000);
    GPRS_PWRKEY_HIGH;
//	GPRS_PWREN_ON;
    NB_Link_step = NBLINK;
    delay_ms(1000);			// 延时2000ms
}
//Usart功耗控制，进入休眠前对串口的处理；
//state为0，禁止串口功能，设置为IO输出低电平；
//为1，禁止串口功能，TX设置为电平输出，RX设置为外部中断上拉输入，下降沿触发中断；
void NB_UsartMode(unsigned char state)
{
    if(state == 0)
    {
        //禁能NB_Usart
        USART_Cmd(USART1 , DISABLE);
        CLK_PeripheralClockConfig(USART1_CLK , DISABLE);
        GPIO_Init(PORT_USART1_RX , PIN_USART1_RX , GPIO_Mode_Out_PP_Low_Fast);
        GPIO_Init(PORT_USART1_TX , PIN_USART1_TX , GPIO_Mode_Out_PP_Low_Fast);
        GPIO_ResetBits(PORT_USART1_RX , PIN_USART1_RX );
        GPIO_ResetBits(PORT_USART1_TX , PIN_USART1_TX );
    }
    else
    {
        //nb模块长供电，IO口拉高，低功耗模式，RX引脚外部中断；
        USART_Cmd(USART1 , DISABLE);
        CLK_PeripheralClockConfig(USART1_CLK , DISABLE);
        GPIO_Init(PORT_USART1_RX , PIN_USART1_RX , GPIO_Mode_In_PU_IT);
        GPIO_Init(PORT_USART1_TX , PIN_USART1_TX , GPIO_Mode_Out_PP_Low_Fast);
        GPIO_SetBits(PORT_USART1_RX , PIN_USART1_RX );
        GPIO_SetBits(PORT_USART1_TX , PIN_USART1_TX );
    }
}
//发送完成模块IO处理；若发送成功，则Rx保持外部中断输入状态；
//模块主动下发数据时唤醒模块，并转为串口接收，接收串口数据
void SendCompleteAction(void)
{
//    //开机状态，发送成功，模块不锁死状态下，不用进行关机操作，只进行低功耗处理
//    if((SendSuccessedFlag==1)&&(GprsStopSendFlag!=GPRS_SEND_LOCK))
//    {
//        NB_UsartMode(1);
//    }
//    else   //否则，进行关机操作并将串口功能关闭
    {
    	NB_Link_step = NBLINK;
        GPRS_PWRKEY_LOW;
        NB_UsartMode(0);
		GPRS_PWREN_OFF;
    	GPRS_RTC_LOW;
    }
}
/**
*@brief	 	计算整形数据的位数
*@param
*@return	n:位数
*/
static unsigned char judgeLen(int  a)
{
    int n=0;
    while (a)
    {
        a = a / 10;
        n++;
    }
    return n;
}
/**
*@brief	 	16进制转换成大写字符串 例如：0xA1，转换成str[0] = ‘A’,str[1] = ‘1’,
*@param		n:16进制，str：字符串
*@return
*/
static void BC95_chartoasc(char aa,unsigned char *buffer)
{
    unsigned char i=0;
    buffer[0]=aa/16;
    buffer[1]=aa%16;
    for(i=0;i<2;i++)
    {
        if(buffer[i]<10)buffer[i]+='0';
        else buffer[i]=buffer[i]-10+'A';
    }
}
//将HEXstr转换为HEX
//例如，buffer[0]='A';buffer[1]='1';转换为aa[0]=0xA1;
//执行结果正常返回1，异常返回0；
static unsigned char BC95_hextochar(char *buffer,char *aa)
{
    unsigned char i=0;
    for(i=0;i<2;i++)
    {
        if(((buffer[i]>='A')&&(buffer[i]<='F')))
        {
            buffer[i]=buffer[i]-'A'+10;
        }
		else if((buffer[i]>='a')&&(buffer[i]<='f'))
		{
			buffer[i]=buffer[i]-'a'+10;
		}
        else if((buffer[i]>='0')&&(buffer[i]<='9'))
        {
            buffer[i]=buffer[i]-'0';
        }
        else
        {
            return 0;
        }
    }
    *aa=(buffer[0]<<4)+buffer[1];
    return 1;
}
static unsigned char BC95_HexStrtoCharStr(char *buffer,unsigned int buflen,char *aa)
{
    unsigned int i=0;
    unsigned char flag=0;
    if((buflen%2==1)||(buflen==0))return 0;
    for(i=0;i<buflen/2;i++)
    {
        flag=BC95_hextochar((char *)&buffer[i*2],(char *)&aa[i]);
        if(flag==0)return 0;
    }
    return 1;
}
/**
*@brief	 	整型数转化为字符串函数
*@param		n:要转化整数， str[5]:存放转化后的字符串  len：整型数长度
*@return	无
*/
static void itoa(unsigned int n,unsigned char str[5], unsigned char len)
{
	unsigned char i=len-1;
	memset(str,0x20,len);
	do{
		str[i--]=n%10+'0';
	}while((n/=10)>0);
	return;
}
//判断dest与source的length长度是否完全相同；
//若相同返回1，不同返回0
static unsigned char Str_Is_Equal(unsigned char *dest,unsigned char *source,unsigned int length)
{
	unsigned int i=0;
	unsigned int flag=1;
	for(i=0;i<length;i++)
	{
		if(*dest++!=*source++)
		{
			flag=0;
			break;
		}
	}
	return flag;
}
//判断buffer字符串是否包含scr字符串；
//输入参数scr为字符串，包含‘\0’；
//输入参数buflenth为buffer字符串长度；
//若buffer字符串包含scr则返回scr字符串结束后下一个字符位置，否则返回0
//例如:
unsigned int IsStrInclude(unsigned char *buffer,unsigned char *scr,unsigned int buflenth)
{
	unsigned int i=0;
	unsigned char flag=0;
	if(buflenth<strlen((char const*)scr))return 0;
	for(i=0;i<buflenth+1-strlen((char const*)scr);i++)
	{
		flag=Str_Is_Equal((unsigned char *)&buffer[i],(unsigned char *)scr,strlen((char const*)scr));
		if(flag==1)
		{
			i+=strlen((char const*)scr);
			return i;
		}
	}
	if(i==buflenth+1-strlen((char const*)scr))i=0;
	return i;
}
/*****************************************************************************
* 函数名称: unsigned char sum_check(unsigned char* data, unsigned char len)
* 功能描述: 累加和校验函数
* 参    数:
* 返回  值:
*****************************************************************************/
unsigned char sum_check(unsigned char *data, unsigned char len)
{
	unsigned char sum = 0;
	unsigned char i = 0;
	for (i = 0; i < len; i++)
	{
		sum += data[i];
	}
	return sum;
}

unsigned char TCPsocket = 0;
/*****************************************************************************
* 函数名称: void Send_AT_Command(unsigned char step)
* 功能描述: 发送命令
* 参    数:
* 返回  值:
*****************************************************************************/
void Send_AT_Command(unsigned char step)
{
	unsigned char Data_Frame[100];
	unsigned char Count=0;
	memset(Data_Frame,0,100);
	switch(step)
	{
		case  NBLINK:
			memcpy(Data_Frame,"AT\r\n",4);
			Count = 4;
			TCPsocket=0;
		break;
		case  NB_ATE0:
			memcpy(Data_Frame,"ATE0\r\n",6);
			Count = 6;
		break;
		case  NB_CLOSE_PSM:
			memcpy(Data_Frame,"AT+CPSMS=0\r\n",12);
			Count = 12;
		break;
		case  NB_EDRXEN:
			memcpy(Data_Frame,"AT+CEDRXS=1\r\n",13);
			Count = 13;
		break;
		case  NB_CEREG:
			memcpy(Data_Frame,"AT+CEREG?\r\n",11);
			Count = 11;
		break;
		case  NB_CSQ:
			memcpy(Data_Frame,"AT+CSQ\r\n",8);
			Count = 8;
		break;
		case  NB_TCPEN:
			if(TCPsocket == 0)
			{
				memcpy(Data_Frame,"AT+ESOC=1,1,1\r\n",15);
				Count = 15;
			}
			else
			{
				memcpy(Data_Frame,"AT+ESOCL=0\r\n",12);
				Data_Frame[9]=TCPsocket;
				Count = 12;
				TCPsocket=0;
			}
		break;
		case  NB_SERVER:
//			memcpy(Data_Frame,"AT+ESOCON=0,16666,\"124.192.224.226\"\r\n",37);
//			Count = 37;
			memcpy(Data_Frame,"AT+ESOCON=0,6000,\"39.105.148.102\"\r\n",35);
			Count = 35;
//			memcpy(Data_Frame,"AT+ESOCON=0,9930,\"39.105.131.183\"\r\n",35);//测试
//			memcpy(Data_Frame,"AT+ESOCON=0,6000,\"39.105.148.102\"\r\n",35);//应用  
//			Count = 41;
			Data_Frame[10]=TCPsocket;
		break;
		default:
		break;
	}
#if	NB_PRINTF
	RS485PrInit();
	USART3_sendstr("NB_TX:");
	USART3_sendbuf(Data_Frame,Count);
	USART3_sendstr("\r\n");
	RS485PrDeinit();
#endif
	NB_UsartSendstr(Data_Frame,Count);
}
//读取信号强度，阻塞时间最多3s，最短时间1s
//异常返回99，正常返回信号强度0-31；
unsigned char Signal_Check(void)
{
	unsigned int lenth=0;
	unsigned int buflenth=0;
	unsigned char i=0;
	unsigned char temp_Buffer[RX_BUF_LEN];
	unsigned char j=0;
    //再次唤醒后需要读取2次才能读到数据，具体原因应进一步分析，
    //时间间隔应调整，（数据发送原因or数据不全？）
	Send_AT_Command(NB_CSQ);
	delay_ms(500);
	Send_AT_Command(NB_CSQ);
	delay_ms(500);
	lenth = UartGetRxLen(&uart_gprs);
	if(lenth>0)
	{
		for(j=0;((buflenth!=lenth)&&(j<200));j++)			//防止数据读取中被打断
		{
			buflenth=lenth;
			delay_ms(10);
			lenth = UartGetRxLen(&uart_gprs);
		}
		lenth=UartRead(&uart_gprs, temp_Buffer, RX_BUF_LEN);
		if((lenth>=sizeof(ATCSQ))&&(strstr((const char *)temp_Buffer,ATOK ))&&(strstr((const char *)temp_Buffer,ATCSQ )))
		{
			//读取信号强度，并记录判定是否信号足够。足够则执行下一步，否则定时查询；
			//若信号强度<40，则判定为可以正常工作
			i=IsStrInclude(temp_Buffer,(unsigned char *)ATCSQ,lenth);
			if((temp_Buffer[i+1]==',')&&(temp_Buffer[i] > '0')&&(temp_Buffer[i] <= '9'))
			{
				return  temp_Buffer[i]-0x30;
			}
			else if((temp_Buffer[i+2]==',')&&(((temp_Buffer[i] >= '0')&&(temp_Buffer[i]<='2')&&(temp_Buffer[i+1] >= '0')&&(temp_Buffer[i+1] <= '9'))\
												||((temp_Buffer[i]=='3')&&(temp_Buffer[i+1] >= '0')&&(temp_Buffer[i+1] <= '3'))))
			{
				return  (temp_Buffer[i]-0x30)*10+temp_Buffer[i+1]-0x30;
			}
		}
	}
	return 99;
}
//读取时间并写入RTC中；也可改为写入结构体中；
//时间读取正确返回1，读取失败返回0；
//阻塞执行，最长6s；
unsigned char ReadTimerFromServer(void)
{
	unsigned int lenth=0;
	unsigned char i,j,k;
	unsigned char temp_Buffer[RX_BUF_LEN];
	unsigned int buflenth=0;
	unsigned char Data_Frame[20];
	unsigned char year,month,day,hour,min,sec;
	unsigned char data[8];
	unsigned char mn=0;

	memset(Data_Frame,0,sizeof(Data_Frame));
	memcpy(Data_Frame,"AT+CCLK?\r\n",10);
	// 读取同步时钟共6s
	for(k=0;k<3;k++)
	{
		delay_ms(400);
#if	NB_PRINTF
		RS485PrInit();
		USART3_sendstr("NB_TX:");
		USART3_sendbuf(Data_Frame,10);
		USART3_sendstr("\r\n");
		RS485PrDeinit();
#endif
		NB_UsartSendstr(Data_Frame,10);

		for(i=0;i<10;i++)
		{
			delay_ms(200);
			lenth = UartGetRxLen(&uart_gprs);
			if(lenth>0)
			{
				for(j=0;((buflenth!=lenth)&&(j<200));j++)			//防止数据读取中被打断，10ms若数据长度不变则接收完成；
				{
					buflenth=lenth;
					delay_ms(10);
					lenth = UartGetRxLen(&uart_gprs);
				}
				lenth=UartRead(&uart_gprs, temp_Buffer, RX_BUF_LEN);
				if((lenth>=sizeof(ATCCLK))&&(strstr((const char *)temp_Buffer,ATOK ))&&(strstr((const char *)temp_Buffer,ATCCLK )))
				{
					//读取同步时钟，并记录判定是否正确；正确更新时间；
					i=IsStrInclude(temp_Buffer,(unsigned char *)ATCCLK,lenth);
					//数据格式为	+CCLK:2018/02/10，06:04:03+32\r\n
					//对应时间为18年2月10日，14:04:03，相差8小时；
					i+=2;	//因格式为2018/非18/，故补加了2；
					if(temp_Buffer[i+2]=='/')//((temp_Buffer[i+2]=='/')&&(temp_Buffer[i+5]=='/')&&(temp_Buffer[i+8]==',')&&(temp_Buffer[i+11]==':')&&(temp_Buffer[i+14]==':'))
					{
						memset(data,0,sizeof(data));
						mn=0;
						for(j=i+3;j<i+14;j++)
						{
							if((temp_Buffer[j] <= '9')&&(temp_Buffer[j] >= '0'))
							{
								data[mn] *= 10;
								data[mn] += temp_Buffer[j]-'0';
							}
							else if((temp_Buffer[j] == '/')||(temp_Buffer[j] == ':')||(temp_Buffer[j] == ','))
							{
								mn++;
							}
							else
							{
								break;
							}
						}
						//data[0-4]分别为月日时分秒，对接收的书籍进行判定；
						if((data[0]>0)&&(data[0]<13)&&(data[1]>0)&&(data[1]<32)&&(data[2]<24)&&(data[3]<60)&&(data[4]<60))
						{
							year=(temp_Buffer[i]-'0')*10+(temp_Buffer[i+1]-'0');
							month=data[0];
							day=data[1];
							hour=data[2];
							min=data[3];
							sec=data[4];
							
							RTC_Adjust(2000+year,month,day,hour,min,sec);
							return 1;
						}
					}
				}
			}
		}
	}
	return 0;
}
//接收数据处理函数
//接收数据正常返回0，接收数据异常返回1；
unsigned char RecieveATCMDhandle(void)
{
	static unsigned char ErrorFlag=0;
	unsigned int lenth=0;
	unsigned int buflenth=0;
	unsigned char temp_Buffer[RX_BUF_LEN];
	unsigned int j=0;
	unsigned int i=0;

	lenth = UartGetRxLen(&uart_gprs);
	if(lenth>0)
	{
		for(j=0;((buflenth!=lenth)&&(j<200));j++)			//防止数据读取中被打断
		{
			buflenth=lenth;
			delay_ms(10);
			lenth = UartGetRxLen(&uart_gprs);
		}
		memset(temp_Buffer,0,RX_BUF_LEN);
		lenth=UartRead(&uart_gprs, temp_Buffer, RX_BUF_LEN);//接收数据成功

		switch(NB_Link_step)
		{
			case  NBLINK:
			case  NB_ATE0:
			case  NB_CLOSE_PSM:
			case  NB_EDRXEN:
				if((lenth<10)&&(lenth>=sizeof(ATOK))&&(strstr((char const*)temp_Buffer, ATOK)))
				{
					NB_Link_step++;
				}else if((lenth>=sizeof(ATERROR))&&(strstr((char const*)temp_Buffer, ATERROR)))
				{
					ErrorFlag=1;
				}
				else
				{
				}
			break;
			case  NB_CEREG:
				if((lenth>=sizeof(ATCEREG))&&(strstr((char const*)temp_Buffer,ATCEREG))&&(strstr((char const*)temp_Buffer,ATOK)))
				{
					i=IsStrInclude(temp_Buffer,(unsigned char *)ATCEREG,lenth);
					if(temp_Buffer[i+2]=='1')
					{
						NB_Link_step++;
					}
					else if(temp_Buffer[i+2]=='0')
					{
					}
				}else if((lenth>=sizeof(ATERROR))&&(strstr((char const*)temp_Buffer, ATERROR)))
				{
					ErrorFlag=1;
				}
				else
				{
				}
			break;
			case  NB_CSQ:
				if((lenth>=sizeof(ATCSQ))&&(strstr((const char *)temp_Buffer,ATCSQ ))&&(strstr((const char *)temp_Buffer,ATOK )))
				{
					//读取信号强度，并记录判定是否信号足够。足够则执行下一步，否则定时查询；
					//若信号强度<40，则判定为可以正常工作
					i=IsStrInclude(temp_Buffer,(unsigned char *)ATCSQ,lenth);
					if((temp_Buffer[i+1]==',')&&(temp_Buffer[i] > '0')&&(temp_Buffer[i] <= '9'))
					{
                        Signal_Value = temp_Buffer[i]-'0';
						NB_Link_step++;
					}
					else if((temp_Buffer[i+2]==',')&&(((temp_Buffer[i] >= '0')&&(temp_Buffer[i]<='2')&&(temp_Buffer[i+1] >= '0')&&(temp_Buffer[i+1] <= '9'))\
												||((temp_Buffer[i]=='3')&&(temp_Buffer[i+1] >= '0')&&(temp_Buffer[i+1] <= '3'))))
					{
                        Signal_Value = (temp_Buffer[i]-'0') * 10 +( temp_Buffer[i+1]-'0');
						NB_Link_step++;
					}
				}else if((lenth>=sizeof(ATERROR))&&(strstr((char const*)temp_Buffer, ATERROR)))
				{
					ErrorFlag=1;
				}
				else
				{
				}
			break;
			case  NB_TCPEN:
				if((lenth>=sizeof(ATTCPEN))&&(strstr((const char *)temp_Buffer,ATTCPEN ))&&(strstr((const char *)temp_Buffer,ATOK )))
				{
					i=IsStrInclude(temp_Buffer,(unsigned char *)ATTCPEN,lenth);
					if((temp_Buffer[i] >= '0')&&(temp_Buffer[i] < '8'))
					{
						TCPsocket=temp_Buffer[i];
						NB_Link_step++;
					}
				}else if((lenth>=sizeof(ATERROR))&&(strstr((char const*)temp_Buffer, ATERROR)))
				{
					ErrorFlag=1;
					if(lenth>30)return 1;
				}
				else
				{
				}
			break;
			case  NB_SERVER:
				if((lenth>=sizeof(ATOK))&&(strstr((char const*)temp_Buffer,ATOK)))
				{
					//ASL.................缺少格式判定；
					//注册网络成功，
					NB_Link_step=NB_CONNECT;
				}else if((lenth>=sizeof(ATERROR))&&(strstr((char const*)temp_Buffer, ATERROR)))
				{
					ErrorFlag=1;
				}
				else
				{
				}
			break;/**/
			case  NB_CONNECT:

			break;
			default :
			break;
		}
	}

	return 0;
}
AT_link_ENUM ReadModStatus(void)
{
    return NB_Link_step;
}
//重发控制；重发命令次数及重发时间间隔；
//若发送正常返回0，重发次数达到返回1；
unsigned char RetSendControl(void)
{
	static AT_link_ENUM NB_Status=NBLINK;		//上一次NB模块执行函数时的状态
	static unsigned int NB_Cnt=0;			//在同一状态下的时间计数（100ms的计数）
	static unsigned int NB_ReSendTimes=0;
	static unsigned char ReSendFlag=0;		//重发标志，在收到错误数据，或者超时时置位，使能上报；
    //重发次数控制；
	NB_Cnt++;
	if(NB_Status!=NB_Link_step) //状态更新，清零标志；
	{
		NB_Status=NB_Link_step;
		NB_Cnt=0;
		NB_ReSendTimes=0;
		ReSendFlag=1;
	}
	else
	{
		switch(NB_Link_step)
		{
			case  NB_ATE0:
			case  NB_CLOSE_PSM:
			case  NB_EDRXEN:
			case  NB_TCPEN:
				if(NB_Cnt>=20)
				{
					NB_Cnt=0;
					NB_ReSendTimes++;
					if(NB_ReSendTimes>=4)
					{
						NB_ReSendTimes=0;
						return 1;
					}
					else
					{
						ReSendFlag=1;
					}
				}
			break;
			case  NBLINK:
			case  NB_SERVER:
				if(NB_Cnt>=60)
				{
					NB_Cnt=0;
					NB_ReSendTimes++;
					if(NB_ReSendTimes>=5)
					{
						NB_ReSendTimes=0;
						return 1;
					}
					else
					{
						ReSendFlag=1;
					}
				}
			break;
			case  NB_CSQ:
			case  NB_CEREG:
				if(NB_Cnt>=40)
				{
					NB_Cnt=0;
					NB_ReSendTimes++;
					if(NB_ReSendTimes>=8)
					{
						NB_ReSendTimes=0;
						return 1;
					}
					else
					{
						ReSendFlag=1;
					}
				}
			break;
			default:
			break;
		}
	}
    //重发控制命令
	if(ReSendFlag!=0)
	{
		ResetUartBuf(&uart_gprs);
		if(NB_Link_step<NB_CONNECT)
		{
			ReSendFlag=0;
			Send_AT_Command(NB_Link_step);
		}
	}
    return 0;
}
//模块注网函数；
//重发命令次数及重发时间间隔控制，接收回复数据判定；
//非阻塞，每100ms执行1次；
//读取CSQ过程中，将信号强度存储于Signal_Value中；
//返回值：为0，正常；
//为1，模块接收数据异常，或者命令重发次数过多；
//此时，应直接退出注网过程，重启模块，防止持续等待；
unsigned char RegisterToServer(void)
{
    unsigned char flag=0;
    flag=RecieveATCMDhandle();
    if(flag == 1) return 1;
    flag=RetSendControl();
    return flag;
}
/*****************************************************************************
* 函数名称: unsigned char gprs_send_start(void)
* 功能描述: 模块联网控制
* 参    数:
* 返回  值:1:注册网络失败	0：注册网络成功
//阻塞，执行时间最多60s，超时重启模块；
*****************************************************************************/
unsigned char gprs_send_start(void)
{
	unsigned int t = 0;
	unsigned char flag=0;
    
    #if	NB_PRINTF
        RS485PrInit();
        USART3_sendstr("NB_gprs_send_start!\r\n");
        RS485PrDeinit();
    #endif
	for (t = 0; t < 600; t++)
	{
		flag=RegisterToServer();
		if(flag==1)
		{
			//模块接收或发送异常情况，尽快退出等待，进行重启，防止持续等待消耗功耗
			t = 600;
			break;
		}
		else
		{

		}
		if (NB_Link_step==NB_CONNECT)						// 挂载网络成功
		{
			Signal_Value=Signal_Check();					// 检测信号强度
#if	NB_PRINTF
			RS485PrInit();
			USART3_sendstr("Register OK !\r\n");
			USART3_sendstr("BC95+CSQ:");
			USART3_sendchar((Signal_Value%100)/10+'0');
			USART3_sendchar(Signal_Value%10+'0');
			RS485PrDeinit();
#endif
			break;
		}
		else
		{
			delay_ms(100);									// 延时100ms
		}
	}
	if (t >= 600)											// 挂载网络超时
	{
		reset_gprs();										// 复位模块
		return 1;
	}
	return 0;
}
/*****************************************************************************
* 函数名称: unsigned int BC95_Tx_Frame(unsigned char *Data,unsigned int DataLen)
* 功能描述: 发送数据
* 参    数:	Data：数据， DataLen：数据长度
* 返回  值:
ASL......添加限制条件,BC95一包数据最多512个；
//将Data以字节流格式上发
*****************************************************************************/
unsigned int BC95_Tx_Frame(unsigned char *Data,unsigned int DataLen)
{
	unsigned int i;
	unsigned char Data_Frame[100];
	unsigned char ReadData[20];
	unsigned int Count = 0;
	unsigned char Len = 0;
	if((NB_Link_step!=NB_CONNECT)||(DataLen>512/2))
	{
		return 0;
	}
	memset(ReadData, 0, sizeof(ReadData));
	memset(Data_Frame, 0, sizeof(Data_Frame));
	i = 0;
	memcpy(Data_Frame,"AT+ESOSEND=",11);
	Count = 11;
	Data_Frame[Count++] = TCPsocket;
	Data_Frame[Count++] = ',';
	Len = judgeLen(DataLen);					//计算位数
	itoa((DataLen), ReadData, Len);				//转换成字符串
	for(i = 0; i< Len;i++)
	{
		Data_Frame[Count++] =  ReadData[i];
	}
	Data_Frame[Count++] = ',';
	
#if	NB_PRINTF
	RS485PrInit();
	USART3_sendstr("NB_TX:");
	USART3_sendbuf(Data_Frame,Count);
//	RS485PrDeinit();
#endif
	NB_UsartSendstr(Data_Frame,Count);

	for( i = 0;i < DataLen; i++)
	{
		BC95_chartoasc(Data[i], Data_Frame);
		NB_UsartSendstr(Data_Frame,2);
#if	NB_PRINTF
//		RS485PrInit();
		USART3_sendbuf(Data_Frame,2);
//		RS485PrDeinit();
#endif
	}
	Data_Frame[0] = '\r';
	Data_Frame[1] = '\n';
	NB_UsartSendstr(Data_Frame,2);
#if	NB_PRINTF
//	RS485PrInit();
    USART3_sendbuf(Data_Frame,2);
	RS485PrDeinit();
#endif
	return Count;
}
//接收数据并转换；
//非阻塞，每250ms执行1次；收到数据后阻塞最多2s，
//返回值：1发送失败，延时重发；
//0无异常
unsigned char RxFrameAnalysis(void)
{
	unsigned char Rxdata[RX_BUF_LEN];
	unsigned int rxlen,i,j=0;
	unsigned int lenth=0;
	unsigned int buflenth=0;
	lenth = UartGetRxLen(&uart_gprs);
	if(lenth>0)
	{
		for(j=0;((buflenth!=lenth)&&(j<200));j++)			//防止数据读取中被打断
		{
			buflenth=lenth;
			delay_ms(10);
			lenth = UartGetRxLen(&uart_gprs);
		}
		memset(Rxdata,0,RX_BUF_LEN);
		lenth=UartRead(&uart_gprs, Rxdata, RX_BUF_LEN);

		switch(CoapSendFlag)
		{
			case  0:
			if((lenth>=sizeof(ATOK))&&(strstr((char const*)Rxdata,(char *)ATOK)))
			{
				CoapSendFlag=3;
				CoapSendCnt=0;
				return 0;
			}else if((lenth>=sizeof(ATERROR))&&(strstr((char const*)Rxdata,(char *)ATERROR)))
			{
				//ASL..............发送数据帧格式错误；应做出提前退出的返回值
#if NB_PRINTF
				RS485PrInit();
				USART3_sendstr("发送数据帧错误");
				RS485PrDeinit();
#endif
				return 1;
			}
			break;
			case  3:
			if((lenth>=sizeof(ATESONMI))&&(strstr((char const*)Rxdata,(char *)ATESONMI)))
			{
				i=IsStrInclude(Rxdata,(unsigned char *)ATESONMI,lenth);
				rxlen=0;
				for(j=i+2;j<lenth;j++)
				{
					if((Rxdata[j]>='0')&&(Rxdata[j]<='9'))
					{
						rxlen=rxlen*10;
						rxlen+=Rxdata[j]-'0';
					}
					else break;
				}
				if((rxlen>RX_BUF_LEN)||(rxlen==0))
				{
					return 0;
				}
				else
				{
					//提取接收的数据，并判定是否接收完成
					NB_Rxlen=rxlen*2;
					memcpy(NBRxdata,(unsigned char *)&Rxdata[j+1],NB_Rxlen);
					if(NB_Rxlen)
					{
						Nsonmi_Flag=0;
#if NB_PRINTF
						RS485PrInit();
						USART3_sendstr("Rx正确接收数据：");
						USART3_sendchar(NB_Rxlen/100+'0');
						USART3_sendchar((NB_Rxlen%100)/10+'0');
						USART3_sendchar(NB_Rxlen%10+'0');
						USART3_sendstr("个，");
						USART3_sendbuf(NBRxdata,NB_Rxlen);
						USART3_sendstr("\r\n");
						RS485PrDeinit();
#endif
					}
				}
			}else if((lenth>=sizeof(ATERROR))&&(strstr((char const*)Rxdata,(char *)ATERROR)))
			{
				//ASL.........读取数据帧格式错误；
#if NB_PRINTF
				RS485PrInit();
				USART3_sendstr("读取数据帧错误");
				RS485PrDeinit();
#endif
			}
			else if((lenth>=sizeof(ATESONMI))&&(strstr((char const*)Rxdata,(char *)ATESONMI)))
			{
				
			}
			else
			{

#if NB_PRINTF
				RS485PrInit();
				//或者先进行读，查看是否有数据返回
				USART3_sendstr("Rx错误接收数据：");
				USART3_sendchar(lenth/100+'0');
				USART3_sendchar((lenth%100)/10+'0');
				USART3_sendchar(lenth%10+'0');
				USART3_sendstr("个，");
				USART3_sendbuf(Rxdata,lenth);
				USART3_sendstr("\r\n");
				RS485PrDeinit();
#endif
			}
			break;
			default:
			break;
		}
	}
	else
	{
		CoapSendCnt++;
		if((CoapSendFlag!=0)&&(CoapSendCnt%10==0))
		{
//			ATGetMsg();
			CoapSendCnt=0;
			CoapSendFlag=3;
		}
	}
	return 0;
}
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
/*****************************************************************************
* 函数名称: unsigned char gprs_send_end(void)
* 功能描述: 并处理接收应答包
* 参   数:
* 返回  值:接收成功，返回0；
发送失败，返回2；
接收失败，返回3；
*****************************************************************************/
void PrintInfor(void);
unsigned char gprs_send_end(void)
{
	unsigned short t;
	unsigned short ii;
	unsigned char flag=0;
	unsigned int i=0;
	unsigned char Check_Sum=0;
    unsigned int lenth=0;
	unsigned char j=0;

	CoapSendFlag=0;
	CoapSendCnt=0;
	Nsonmi_Flag=1;
	ResetUartBuf(&uart_gprs);				// 复位接收缓冲区
#if	NB_PRINTF
	RS485PrInit();
	USART3_sendstr("Wait Server ACK !\r\n");
	RS485PrDeinit();
#endif
	for (t = 0; t < 130; t++)				// 等待服务器应答 30S
	{
		delay_ms(250);
		flag = RxFrameAnalysis();
		if(flag == 1) return 2;
		if (NB_Rxlen != 0)					// 接收服务器
		{
#if	NB_PRINTF
			RS485PrInit();
			USART3_sendstr("Server ACK !\r\n");
			RS485PrDeinit();
#endif
			memset(NBbuffer,0,sizeof(NBbuffer));
			flag=BC95_HexStrtoCharStr((char *)NBRxdata,NB_Rxlen,(char*)NBbuffer);
			//接收数据格式正确与否
			if(flag==0)	//不正确
			{
				NBbuflenth=0;
			}
			else
			{
				NBbuflenth=NB_Rxlen/2;
			}
			memset(NBRxdata,0,sizeof(NBRxdata));
			NB_Rxlen = 0;
			for (ii = 0; ii + 30-4 <= NBbuflenth; ii++)
			{
				//下发确认帧
				if ((NBbuffer[ii] == 0xA5) && (NBbuffer[ii + 1] == 0xA5) && (NBbuffer[ii + 28-4] == 0xbe) && (NBbuffer[ii + 29-4] == 0xef) \
//                    && (NBbuffer[ii+2]==FrameCount%256)&&(NBbuffer[ii+3]==FrameCount/256)
					&& (NBbuffer[ii + 24-4] == 0x0) && (NBbuffer[ii + 25-4] == 0x0) && (NBbuffer[ii + 26-4] == 0x03))
				{
					Check_Sum=0;
					for(i = 2;i < 27-4;i++ )
					{
						Check_Sum=Check_Sum + NBbuffer[ii + i];
					}
					if((NBbuffer[ii+27-4] == Check_Sum) && (Nsonmi_Flag==0))
					{
						SendSuccessedFlag=1;
						return 0;
					}
				}
                //下发的浇水计划的解析
				if ((NBbuffer[ii] == 0xA5) && (NBbuffer[ii + 1] == 0xA5) \
					&& (NBbuffer[ii + 26-4] == 0x01)&& (NBbuffer[ii + 27-4] == 0x81))
				{
//                    lenth=NBbuffer[ii + 24-4] + NBbuffer[ii + 25-4] * 256;
                                  lenth =16;
                    if( (ii + 30-4 + lenth <= NBbuflenth )&&(14*NBbuffer[ii + 24]+2 == lenth)  \
                       && (NBbuffer[ii + 28-4 + lenth] == 0xbe) && (NBbuffer[ii + 29-4 + lenth] == 0xef) )
                    {
						Check_Sum=0;
						for(i = 2;i < 27-4 + lenth;i++ )
						{
							Check_Sum=Check_Sum + NBbuffer[ii + i];
						}
//						if((NBbuffer[ii + 27-4 + lenth] == Check_Sum) && (Nsonmi_Flag==0))
						{
							//全局变量存储返回的确认帧流水号；
							//数据解析，浇水计划
							gRxFrameCount=NBbuffer[2+ii]+(NBbuffer[3+ii]<<8);
							//修改后浇水计划的添加不是增量添加，是新的浇水计划表格整体添加；
							Clear_PlanEEPROM();
							for(i=0;i<NBbuffer[ii + 24];i++)
							{
								if((NBbuffer[ii + 25+i*14] <= 1)&&(NBbuffer[ii + 26+i*14] < 60)&&(NBbuffer[ii + 27+i*14] < 60)&&(NBbuffer[ii + 28+i*14] < 24)				\
										&&((NBbuffer[ii + 29+i*14] >= 1)&&(NBbuffer[ii + 29+i*14] < 31))&&((NBbuffer[ii + 30+i*14] >= 1)&&(NBbuffer[ii + 30+i*14] < 13))	\
											&&(NBbuffer[ii + 31+i*14] < 99)&&(NBbuffer[ii + 32+i*14] <= 1)&&(NBbuffer[ii + 33+i*14] + NBbuffer[ii + 34+i*14]*256 < 366)		\
												&&(NBbuffer[ii + 35+i*14] + NBbuffer[ii + 36+i*14]*256 < 720)&&(NBbuffer[ii + 37+i*14] + NBbuffer[ii + 38+i*14]*256 < 360))
								{
									Write_EEPROM(&NBbuffer[ii + 25+i*14]);
								}
							}
							Read_EEPROM();
                            PrintInfor();
							SendSuccessedFlag=1;
							SendAck_Flag=1;
							return 0;
						}
                    }
				}
				//解析下发配置0x82
				if ((NBbuffer[ii] == 0xA5) && (NBbuffer[ii + 1] == 0xA5) \
					&& (NBbuffer[ii + 26-4] == 0x01)&& (NBbuffer[ii + 27-4] == 0x82))
				{
                    lenth=NBbuffer[ii + 24-4] + NBbuffer[ii + 25-4] * 256;
					//(NBbuffer[ii + 28-4] <= 3) && 
                    if( (ii + 30-4 + lenth <= NBbuflenth )  \
                       && (NBbuffer[ii + 28-4 + lenth] == 0xbe) && (NBbuffer[ii + 29-4 + lenth] == 0xef))
                    {
						Check_Sum=0;
						for(i = 2;i < 27-4 + lenth;i++ )
						{
							Check_Sum=Check_Sum + NBbuffer[ii + i];
						}
						if((NBbuffer[ii + 27-4 + lenth] == Check_Sum) && (Nsonmi_Flag==0))
						{
							j=25;
							for(i=0;(i<NBbuffer[ii + 24])&&(j<23+lenth);i++)
							{
								//心跳周期判定//&&((NBbuffer[ii + j+2]+NBbuffer[ii + j+3]*256)%60==0x0)
								if((NBbuffer[ii + j]==0x01)&&(NBbuffer[ii + j+1]==0x02)\
									&&(NBbuffer[ii + j+2]+NBbuffer[ii + j+3]*256<=1440)&&(NBbuffer[ii + j+2]+NBbuffer[ii + j+3]*256>=1))
								{
									HeartBeatPeriod = NBbuffer[ii + j+2]+NBbuffer[ii + j+3]*256;
									j+=4;
								}
								else
								//电池电量采集周期判定//&&((NBbuffer[ii + j+2]+NBbuffer[ii + j+3]*256)%60==0x0)
								if((NBbuffer[ii + j]==0x02)&&(NBbuffer[ii + j+1]==0x02)\
									&&(NBbuffer[ii + j+2]+NBbuffer[ii + j+3]*256<=1440)&&(NBbuffer[ii + j+2]+NBbuffer[ii + j+3]*256>=1))
								{
									BatCollectPeriod = NBbuffer[ii + j+2]+NBbuffer[ii + j+3]*256;
									j+=4;
								}
								else
								//K2按键有效时延判定//&&(NBbuffer[ii + j+2]%5==0x0)
								if((NBbuffer[ii + j]==0x03)&&(NBbuffer[ii + j+1]==0x01)\
									&&(NBbuffer[ii + j+2] <= 40)&&(NBbuffer[ii + j+2] >= 1))
								{
									N8 = N9 = NBbuffer[ii + j+2];
									j+=3;
								}
								else
								//土壤湿度采集时延判定//&&(NBbuffer[ii + j+2]%5==0x0)
								if((NBbuffer[ii + j]==0x04)&&(NBbuffer[ii + j+1]==0x01)\
									&&(NBbuffer[ii + j+2] <= 200)&&(NBbuffer[ii + j+2] >= 1))
								{
//									N8 = N9 = NBbuffer[ii + j+2];
									j+=3;
								}
								else
								//土壤湿度上限判定
								if((NBbuffer[ii + j]==0x05)&&(NBbuffer[ii + j+1]==0x01)&&(NBbuffer[ii + j+2] <= 100))
								{
									HumidityMAX = NBbuffer[ii + j+2];
									j+=3;
								}
								else
								//压力下限判定
								if((NBbuffer[ii + j]==0x06)&&(NBbuffer[ii + j+1]==0x01)&&(NBbuffer[ii + j+2] <= 250))
								{
									PressMIN = NBbuffer[ii + j+2];
									j+=3;
								}
								else
								//压力下限判定
								if((NBbuffer[ii + j]==0x06)&&(NBbuffer[ii + j+1]==0x01)&&(NBbuffer[ii + j+2] <= 250))
								{
									PressMIN = NBbuffer[ii + j+2];
									j+=3;
								}
								else
								//电池电压工作下限判定
								if((NBbuffer[ii + j]==0x07)&&(NBbuffer[ii + j+1]==0x02)\
									&&(NBbuffer[ii + j+2]+NBbuffer[ii + j+3]*256<=7400)&&(NBbuffer[ii + j+2]+NBbuffer[ii + j+3]*256>=3200))
								{
									Vbat_ValueMIN = NBbuffer[ii + j+2]+NBbuffer[ii + j+3]*256;
									j+=4;
								}
								else
								{
									break;
								}
							}
							//若对象数目正确，响应解析正确则配置有效
							if((i==NBbuffer[ii + 24])&&(j==23+lenth))
							{
								//全局变量存储返回的确认帧流水号；
								gRxFrameCount=NBbuffer[2+ii]+(NBbuffer[3+ii]<<8);
//								判定下发的对象数目与下发的信息，并进行存储
//								Vbat_ValueMIN = data[1]*256+data[0];
//								HumidityMAX =data[2];
//								PressMIN = data[3];
//								HeartBeatPeriod = data[5]*256+data[4];
//								BatCollectPeriod = data[7]*256+data[6];
//								N8 = data[8];
//								N9 = data[9];
								WriteConfigEEPROM();
								SendSuccessedFlag=1;
								SendAck_Flag=1;
								return 0;
							}
							else
							{
								ReadConfigEEPROM();
							}
                            PrintInfor();
						}
                    }
				}
			}
		}
	}

	#if NB_PRINTF
	RS485PrInit();
	USART3_sendstr("Wait Server ACK Out Time !\r\n");
	RS485PrDeinit();
	#endif
	return 3;
}

//-----------------------------结束-------------------------------------------------------------//





