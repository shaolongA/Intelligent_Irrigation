/*******************************************************************************
*                       ģ������
********************************************************************************
*ʱ�䣺2018-05-20
*******************************************************************************/
//Ӳ��˵����

//--------------------------------------------------------------------------------//

/***********************ͷ�ļ�***************************************************/
#include "stm8l15x.h"
#include <string.h>
#include "gprs.h"
#include "uart.h"
#include "rtc.h"
#include "eeprom.h"

#define NB_PRINTF	1
/*********************��������*******************************************************/
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
static unsigned char Nsonmi_Flag=0;					//1:�������ݣ�2�������ݳɹ���3:���������������0���Ѿ������ݶ���

static unsigned char NBRxdata[RX_BUF_LEN];			//���·������ݴӽ��յ�NB�����а������
static unsigned int NB_Rxlen=0;
static unsigned char NBbuffer[RX_BUF_LEN];					//������ת��ΪHEX��ʽ
static unsigned int NBbuflenth=0;

static unsigned char CoapSendFlag=0;
static unsigned int CoapSendCnt=0;
static unsigned char SendSuccessedFlag=0;				//���ݷ��ͳɹ���־��Ϊ0����ʧ�ܣ�Ϊ1���ͳɹ�
extern unsigned char GprsStopSendFlag;

extern unsigned char SendAck_Flag;
extern unsigned int gRxFrameCount;
extern volatile unsigned short FrameCount;
volatile unsigned char Signal_Value=99;   //�洢�ź�ǿ��
//static unsigned char IMEI[15];          //�洢IMEI
stUart uart_gprs;
/*********************��������******************************************************/

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
* ��������: void GPRS_IO_Init()
* ��������: GPRS����IO��ʼ��
PC4---PWRKEY,Ĭ������͵�ƽ���ػ����ߵ�ƽ��������
����ʱͨ������300ms���Ͽ�����֮�󱣳ָߵ�ƽ��
��������״̬��ͨ�����ʹ˶˿ڽ���ģ�黽�ѣ�
PD2---NB_RTC_EINT��Ĭ������͵�ƽ��PSMģʽ�����߻��ѣ�
* ��    ��:
* ����  ֵ:
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
//���ڳ�ʼ��
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
//NB���ڷ�������
void NB_UsartSendstr(unsigned char *data , unsigned int strlen)
{
	LED0_ON();
    USART1_sendbuf(data,strlen);
	LED0_OFF();
}
/*****************************************************************************
* ��������: unsigned char OpenGPRS(void)
* ��������: GPRS������⺯��
* ��    ��: 1��׼��  0��δ׼����
* ����  ֵ:
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
* ��������: void reset_gprs(void)
* ��������: 
* ��    ��:
* ����  ֵ:
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
    delay_ms(1000);			// ��ʱ2000ms
}
//Usart���Ŀ��ƣ���������ǰ�Դ��ڵĴ�����
//stateΪ0����ֹ���ڹ��ܣ�����ΪIO����͵�ƽ��
//Ϊ1����ֹ���ڹ��ܣ�TX����Ϊ��ƽ�����RX����Ϊ�ⲿ�ж��������룬�½��ش����жϣ�
void NB_UsartMode(unsigned char state)
{
    if(state == 0)
    {
        //����NB_Usart
        USART_Cmd(USART1 , DISABLE);
        CLK_PeripheralClockConfig(USART1_CLK , DISABLE);
        GPIO_Init(PORT_USART1_RX , PIN_USART1_RX , GPIO_Mode_Out_PP_Low_Fast);
        GPIO_Init(PORT_USART1_TX , PIN_USART1_TX , GPIO_Mode_Out_PP_Low_Fast);
        GPIO_ResetBits(PORT_USART1_RX , PIN_USART1_RX );
        GPIO_ResetBits(PORT_USART1_TX , PIN_USART1_TX );
    }
    else
    {
        //nbģ�鳤���磬IO�����ߣ��͹���ģʽ��RX�����ⲿ�жϣ�
        USART_Cmd(USART1 , DISABLE);
        CLK_PeripheralClockConfig(USART1_CLK , DISABLE);
        GPIO_Init(PORT_USART1_RX , PIN_USART1_RX , GPIO_Mode_In_PU_IT);
        GPIO_Init(PORT_USART1_TX , PIN_USART1_TX , GPIO_Mode_Out_PP_Low_Fast);
        GPIO_SetBits(PORT_USART1_RX , PIN_USART1_RX );
        GPIO_SetBits(PORT_USART1_TX , PIN_USART1_TX );
    }
}
//�������ģ��IO�����������ͳɹ�����Rx�����ⲿ�ж�����״̬��
//ģ�������·�����ʱ����ģ�飬��תΪ���ڽ��գ����մ�������
void SendCompleteAction(void)
{
//    //����״̬�����ͳɹ���ģ�鲻����״̬�£����ý��йػ�������ֻ���е͹��Ĵ���
//    if((SendSuccessedFlag==1)&&(GprsStopSendFlag!=GPRS_SEND_LOCK))
//    {
//        NB_UsartMode(1);
//    }
//    else   //���򣬽��йػ������������ڹ��ܹر�
    {
    	NB_Link_step = NBLINK;
        GPRS_PWRKEY_LOW;
        NB_UsartMode(0);
		GPRS_PWREN_OFF;
    	GPRS_RTC_LOW;
    }
}
/**
*@brief	 	�����������ݵ�λ��
*@param
*@return	n:λ��
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
*@brief	 	16����ת���ɴ�д�ַ��� ���磺0xA1��ת����str[0] = ��A��,str[1] = ��1��,
*@param		n:16���ƣ�str���ַ���
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
//��HEXstrת��ΪHEX
//���磬buffer[0]='A';buffer[1]='1';ת��Ϊaa[0]=0xA1;
//ִ�н����������1���쳣����0��
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
*@brief	 	������ת��Ϊ�ַ�������
*@param		n:Ҫת�������� str[5]:���ת������ַ���  len������������
*@return	��
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
//�ж�dest��source��length�����Ƿ���ȫ��ͬ��
//����ͬ����1����ͬ����0
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
//�ж�buffer�ַ����Ƿ����scr�ַ�����
//�������scrΪ�ַ�����������\0����
//�������buflenthΪbuffer�ַ������ȣ�
//��buffer�ַ�������scr�򷵻�scr�ַ�����������һ���ַ�λ�ã����򷵻�0
//����:
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
* ��������: unsigned char sum_check(unsigned char* data, unsigned char len)
* ��������: �ۼӺ�У�麯��
* ��    ��:
* ����  ֵ:
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
* ��������: void Send_AT_Command(unsigned char step)
* ��������: ��������
* ��    ��:
* ����  ֵ:
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
//			memcpy(Data_Frame,"AT+ESOCON=0,9930,\"39.105.131.183\"\r\n",35);//����
//			memcpy(Data_Frame,"AT+ESOCON=0,6000,\"39.105.148.102\"\r\n",35);//Ӧ��  
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
//��ȡ�ź�ǿ�ȣ�����ʱ�����3s�����ʱ��1s
//�쳣����99�����������ź�ǿ��0-31��
unsigned char Signal_Check(void)
{
	unsigned int lenth=0;
	unsigned int buflenth=0;
	unsigned char i=0;
	unsigned char temp_Buffer[RX_BUF_LEN];
	unsigned char j=0;
    //�ٴλ��Ѻ���Ҫ��ȡ2�β��ܶ������ݣ�����ԭ��Ӧ��һ��������
    //ʱ����Ӧ�����������ݷ���ԭ��or���ݲ�ȫ����
	Send_AT_Command(NB_CSQ);
	delay_ms(500);
	Send_AT_Command(NB_CSQ);
	delay_ms(500);
	lenth = UartGetRxLen(&uart_gprs);
	if(lenth>0)
	{
		for(j=0;((buflenth!=lenth)&&(j<200));j++)			//��ֹ���ݶ�ȡ�б����
		{
			buflenth=lenth;
			delay_ms(10);
			lenth = UartGetRxLen(&uart_gprs);
		}
		lenth=UartRead(&uart_gprs, temp_Buffer, RX_BUF_LEN);
		if((lenth>=sizeof(ATCSQ))&&(strstr((const char *)temp_Buffer,ATOK ))&&(strstr((const char *)temp_Buffer,ATCSQ )))
		{
			//��ȡ�ź�ǿ�ȣ�����¼�ж��Ƿ��ź��㹻���㹻��ִ����һ��������ʱ��ѯ��
			//���ź�ǿ��<40�����ж�Ϊ������������
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
//��ȡʱ�䲢д��RTC�У�Ҳ�ɸ�Ϊд��ṹ���У�
//ʱ���ȡ��ȷ����1����ȡʧ�ܷ���0��
//����ִ�У��6s��
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
	// ��ȡͬ��ʱ�ӹ�6s
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
				for(j=0;((buflenth!=lenth)&&(j<200));j++)			//��ֹ���ݶ�ȡ�б���ϣ�10ms�����ݳ��Ȳ����������ɣ�
				{
					buflenth=lenth;
					delay_ms(10);
					lenth = UartGetRxLen(&uart_gprs);
				}
				lenth=UartRead(&uart_gprs, temp_Buffer, RX_BUF_LEN);
				if((lenth>=sizeof(ATCCLK))&&(strstr((const char *)temp_Buffer,ATOK ))&&(strstr((const char *)temp_Buffer,ATCCLK )))
				{
					//��ȡͬ��ʱ�ӣ�����¼�ж��Ƿ���ȷ����ȷ����ʱ�䣻
					i=IsStrInclude(temp_Buffer,(unsigned char *)ATCCLK,lenth);
					//���ݸ�ʽΪ	+CCLK:2018/02/10��06:04:03+32\r\n
					//��Ӧʱ��Ϊ18��2��10�գ�14:04:03�����8Сʱ��
					i+=2;	//���ʽΪ2018/��18/���ʲ�����2��
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
						//data[0-4]�ֱ�Ϊ����ʱ���룬�Խ��յ��鼮�����ж���
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
//�������ݴ�������
//����������������0�����������쳣����1��
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
		for(j=0;((buflenth!=lenth)&&(j<200));j++)			//��ֹ���ݶ�ȡ�б����
		{
			buflenth=lenth;
			delay_ms(10);
			lenth = UartGetRxLen(&uart_gprs);
		}
		memset(temp_Buffer,0,RX_BUF_LEN);
		lenth=UartRead(&uart_gprs, temp_Buffer, RX_BUF_LEN);//�������ݳɹ�

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
					//��ȡ�ź�ǿ�ȣ�����¼�ж��Ƿ��ź��㹻���㹻��ִ����һ��������ʱ��ѯ��
					//���ź�ǿ��<40�����ж�Ϊ������������
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
					//ASL.................ȱ�ٸ�ʽ�ж���
					//ע������ɹ���
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
//�ط����ƣ��ط�����������ط�ʱ������
//��������������0���ط������ﵽ����1��
unsigned char RetSendControl(void)
{
	static AT_link_ENUM NB_Status=NBLINK;		//��һ��NBģ��ִ�к���ʱ��״̬
	static unsigned int NB_Cnt=0;			//��ͬһ״̬�µ�ʱ�������100ms�ļ�����
	static unsigned int NB_ReSendTimes=0;
	static unsigned char ReSendFlag=0;		//�ط���־�����յ��������ݣ����߳�ʱʱ��λ��ʹ���ϱ���
    //�ط��������ƣ�
	NB_Cnt++;
	if(NB_Status!=NB_Link_step) //״̬���£������־��
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
    //�ط���������
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
//ģ��ע��������
//�ط�����������ط�ʱ�������ƣ����ջظ������ж���
//��������ÿ100msִ��1�Σ�
//��ȡCSQ�����У����ź�ǿ�ȴ洢��Signal_Value�У�
//����ֵ��Ϊ0��������
//Ϊ1��ģ����������쳣�����������ط��������ࣻ
//��ʱ��Ӧֱ���˳�ע�����̣�����ģ�飬��ֹ�����ȴ���
unsigned char RegisterToServer(void)
{
    unsigned char flag=0;
    flag=RecieveATCMDhandle();
    if(flag == 1) return 1;
    flag=RetSendControl();
    return flag;
}
/*****************************************************************************
* ��������: unsigned char gprs_send_start(void)
* ��������: ģ����������
* ��    ��:
* ����  ֵ:1:ע������ʧ��	0��ע������ɹ�
//������ִ��ʱ�����60s����ʱ����ģ�飻
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
			//ģ����ջ����쳣����������˳��ȴ���������������ֹ�����ȴ����Ĺ���
			t = 600;
			break;
		}
		else
		{

		}
		if (NB_Link_step==NB_CONNECT)						// ��������ɹ�
		{
			Signal_Value=Signal_Check();					// ����ź�ǿ��
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
			delay_ms(100);									// ��ʱ100ms
		}
	}
	if (t >= 600)											// �������糬ʱ
	{
		reset_gprs();										// ��λģ��
		return 1;
	}
	return 0;
}
/*****************************************************************************
* ��������: unsigned int BC95_Tx_Frame(unsigned char *Data,unsigned int DataLen)
* ��������: ��������
* ��    ��:	Data�����ݣ� DataLen�����ݳ���
* ����  ֵ:
ASL......������������,BC95һ���������512����
//��Data���ֽ�����ʽ�Ϸ�
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
	Len = judgeLen(DataLen);					//����λ��
	itoa((DataLen), ReadData, Len);				//ת�����ַ���
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
//�������ݲ�ת����
//��������ÿ250msִ��1�Σ��յ����ݺ��������2s��
//����ֵ��1����ʧ�ܣ���ʱ�ط���
//0���쳣
unsigned char RxFrameAnalysis(void)
{
	unsigned char Rxdata[RX_BUF_LEN];
	unsigned int rxlen,i,j=0;
	unsigned int lenth=0;
	unsigned int buflenth=0;
	lenth = UartGetRxLen(&uart_gprs);
	if(lenth>0)
	{
		for(j=0;((buflenth!=lenth)&&(j<200));j++)			//��ֹ���ݶ�ȡ�б����
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
				//ASL..............��������֡��ʽ����Ӧ������ǰ�˳��ķ���ֵ
#if NB_PRINTF
				RS485PrInit();
				USART3_sendstr("��������֡����");
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
					//��ȡ���յ����ݣ����ж��Ƿ�������
					NB_Rxlen=rxlen*2;
					memcpy(NBRxdata,(unsigned char *)&Rxdata[j+1],NB_Rxlen);
					if(NB_Rxlen)
					{
						Nsonmi_Flag=0;
#if NB_PRINTF
						RS485PrInit();
						USART3_sendstr("Rx��ȷ�������ݣ�");
						USART3_sendchar(NB_Rxlen/100+'0');
						USART3_sendchar((NB_Rxlen%100)/10+'0');
						USART3_sendchar(NB_Rxlen%10+'0');
						USART3_sendstr("����");
						USART3_sendbuf(NBRxdata,NB_Rxlen);
						USART3_sendstr("\r\n");
						RS485PrDeinit();
#endif
					}
				}
			}else if((lenth>=sizeof(ATERROR))&&(strstr((char const*)Rxdata,(char *)ATERROR)))
			{
				//ASL.........��ȡ����֡��ʽ����
#if NB_PRINTF
				RS485PrInit();
				USART3_sendstr("��ȡ����֡����");
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
				//�����Ƚ��ж����鿴�Ƿ������ݷ���
				USART3_sendstr("Rx����������ݣ�");
				USART3_sendchar(lenth/100+'0');
				USART3_sendchar((lenth%100)/10+'0');
				USART3_sendchar(lenth%10+'0');
				USART3_sendstr("����");
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
/*****************************************************************************
* ��������: unsigned char gprs_send_end(void)
* ��������: ����������Ӧ���
* ��   ��:
* ����  ֵ:���ճɹ�������0��
����ʧ�ܣ�����2��
����ʧ�ܣ�����3��
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
	ResetUartBuf(&uart_gprs);				// ��λ���ջ�����
#if	NB_PRINTF
	RS485PrInit();
	USART3_sendstr("Wait Server ACK !\r\n");
	RS485PrDeinit();
#endif
	for (t = 0; t < 130; t++)				// �ȴ�������Ӧ�� 30S
	{
		delay_ms(250);
		flag = RxFrameAnalysis();
		if(flag == 1) return 2;
		if (NB_Rxlen != 0)					// ���շ�����
		{
#if	NB_PRINTF
			RS485PrInit();
			USART3_sendstr("Server ACK !\r\n");
			RS485PrDeinit();
#endif
			memset(NBbuffer,0,sizeof(NBbuffer));
			flag=BC95_HexStrtoCharStr((char *)NBRxdata,NB_Rxlen,(char*)NBbuffer);
			//�������ݸ�ʽ��ȷ���
			if(flag==0)	//����ȷ
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
				//�·�ȷ��֡
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
                //�·��Ľ�ˮ�ƻ��Ľ���
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
							//ȫ�ֱ����洢���ص�ȷ��֡��ˮ�ţ�
							//���ݽ�������ˮ�ƻ�
							gRxFrameCount=NBbuffer[2+ii]+(NBbuffer[3+ii]<<8);
							//�޸ĺ�ˮ�ƻ������Ӳ����������ӣ����µĽ�ˮ�ƻ������������ӣ�
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
				//�����·�����0x82
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
								//���������ж�//&&((NBbuffer[ii + j+2]+NBbuffer[ii + j+3]*256)%60==0x0)
								if((NBbuffer[ii + j]==0x01)&&(NBbuffer[ii + j+1]==0x02)\
									&&(NBbuffer[ii + j+2]+NBbuffer[ii + j+3]*256<=1440)&&(NBbuffer[ii + j+2]+NBbuffer[ii + j+3]*256>=1))
								{
									HeartBeatPeriod = NBbuffer[ii + j+2]+NBbuffer[ii + j+3]*256;
									j+=4;
								}
								else
								//��ص����ɼ������ж�//&&((NBbuffer[ii + j+2]+NBbuffer[ii + j+3]*256)%60==0x0)
								if((NBbuffer[ii + j]==0x02)&&(NBbuffer[ii + j+1]==0x02)\
									&&(NBbuffer[ii + j+2]+NBbuffer[ii + j+3]*256<=1440)&&(NBbuffer[ii + j+2]+NBbuffer[ii + j+3]*256>=1))
								{
									BatCollectPeriod = NBbuffer[ii + j+2]+NBbuffer[ii + j+3]*256;
									j+=4;
								}
								else
								//K2������Чʱ���ж�//&&(NBbuffer[ii + j+2]%5==0x0)
								if((NBbuffer[ii + j]==0x03)&&(NBbuffer[ii + j+1]==0x01)\
									&&(NBbuffer[ii + j+2] <= 40)&&(NBbuffer[ii + j+2] >= 1))
								{
									N8 = N9 = NBbuffer[ii + j+2];
									j+=3;
								}
								else
								//����ʪ�Ȳɼ�ʱ���ж�//&&(NBbuffer[ii + j+2]%5==0x0)
								if((NBbuffer[ii + j]==0x04)&&(NBbuffer[ii + j+1]==0x01)\
									&&(NBbuffer[ii + j+2] <= 200)&&(NBbuffer[ii + j+2] >= 1))
								{
//									N8 = N9 = NBbuffer[ii + j+2];
									j+=3;
								}
								else
								//����ʪ�������ж�
								if((NBbuffer[ii + j]==0x05)&&(NBbuffer[ii + j+1]==0x01)&&(NBbuffer[ii + j+2] <= 100))
								{
									HumidityMAX = NBbuffer[ii + j+2];
									j+=3;
								}
								else
								//ѹ�������ж�
								if((NBbuffer[ii + j]==0x06)&&(NBbuffer[ii + j+1]==0x01)&&(NBbuffer[ii + j+2] <= 250))
								{
									PressMIN = NBbuffer[ii + j+2];
									j+=3;
								}
								else
								//ѹ�������ж�
								if((NBbuffer[ii + j]==0x06)&&(NBbuffer[ii + j+1]==0x01)&&(NBbuffer[ii + j+2] <= 250))
								{
									PressMIN = NBbuffer[ii + j+2];
									j+=3;
								}
								else
								//��ص�ѹ���������ж�
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
							//��������Ŀ��ȷ����Ӧ������ȷ��������Ч
							if((i==NBbuffer[ii + 24])&&(j==23+lenth))
							{
								//ȫ�ֱ����洢���ص�ȷ��֡��ˮ�ţ�
								gRxFrameCount=NBbuffer[2+ii]+(NBbuffer[3+ii]<<8);
//								�ж��·��Ķ�����Ŀ���·�����Ϣ�������д洢
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

//-----------------------------����-------------------------------------------------------------//




