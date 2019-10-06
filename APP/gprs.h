 /*******************************************************************************
*                       USR-GM3����
********************************************************************************
*��˾������Ƽ�
*ʱ�䣺2018-05-20
*******************************************************************************/

#ifndef GPRS_H
#define GPRS_H

/***********************ͷ�ļ�**************************************************/
#include "stm8l15x.h"
/***********************�궨��***************************************************/
#define GPRS_SEND_ENABLE 0
#define GPRS_SEND_DISABLE 1
#define GPRS_SEND_LOCK 2
#define TX_MAX_LEN       250

#define PORT_GPRS_PWREN			GPIOA
#define PIN_GPRS_PWREN			GPIO_Pin_6
#define PORT_GPRS_PWRKEY		GPIOC
#define PIN_GPRS_PWRKEY			GPIO_Pin_4
#define PORT_GPRS_RTC			GPIOD
#define PIN_GPRS_RTC			GPIO_Pin_2


#define GPRS_PWREN_ON			GPIO_SetBits(PORT_GPRS_PWREN , PIN_GPRS_PWREN )//A6
#define GPRS_PWREN_OFF			GPIO_ResetBits(PORT_GPRS_PWREN , PIN_GPRS_PWREN )
#define GPRS_PWRKEY_HIGH		GPIO_SetBits(PORT_GPRS_PWRKEY , PIN_GPRS_PWRKEY )//C4
#define GPRS_PWRKEY_LOW			GPIO_ResetBits(PORT_GPRS_PWRKEY , PIN_GPRS_PWRKEY )
#define GPRS_RTC_HIGH			GPIO_SetBits(PORT_GPRS_RTC , PIN_GPRS_RTC )//D2
#define GPRS_RTC_LOW			GPIO_ResetBits(PORT_GPRS_RTC , PIN_GPRS_RTC )

#define PORT_LED		GPIOB
#define PIN_LED			GPIO_Pin_1
#define LED0_OFF()		GPIO_ResetBits(PORT_LED, PIN_LED);
#define LED0_ON()		GPIO_SetBits(PORT_LED, PIN_LED);

typedef enum
{
	NBLINK= 0,						/*AT����*/
	NB_ATE0,                        /*�رջ��Թ���*/
	NB_CLOSE_PSM,          			/*�ر�PSM����*/
	NB_EDRXEN,						/*����ΪeDRX����ģʽ*/
//	NB_CGSN,						/*��ѯIMEI��*/
//	NB_CGATT,						/*��ѯ����״̬*/
	NB_CEREG,						/*��ѯע������״̬*/
	NB_CSQ,							/*��ѯ�ź�ǿ��*/
	NB_TCPEN,                       /*����TCP����*/
	NB_SERVER,                      /*����TCP SERVER��ַ�˿ں�*/
	NB_CONNECT						/*��������״̬*/
}AT_link_ENUM;

/*********************��������****************************************************/
extern void delay_ms(unsigned int n);
void NB_Init(void);
void NB_UsartInit(unsigned long baudrate);
unsigned char OpenGPRS(void);
AT_link_ENUM ReadModStatus(void);
unsigned char Signal_Check(void);
unsigned char ReadTimerFromServer(void);
unsigned char gprs_send_start(void);
unsigned char gprs_send_end(void);
void reset_gprs(void);
void SendCompleteAction(void);
unsigned int BC95_Tx_Frame(unsigned char *Data,unsigned int DataLen);
unsigned char RxFrameAnalysis(void);
/*********************�ⲿ��������*************************************************/

#endif
