 /*******************************************************************************
*                       USR-GM3驱动
********************************************************************************
*公司：富邦科技
*时间：2018-05-20
*******************************************************************************/

#ifndef GPRS_H
#define GPRS_H

/***********************头文件**************************************************/
#include "stm8l15x.h"
/***********************宏定义***************************************************/
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
	NBLINK= 0,						/*AT连接*/
	NB_ATE0,                        /*关闭回显功能*/
	NB_CLOSE_PSM,          			/*关闭PSM功能*/
	NB_EDRXEN,						/*设置为eDRX工作模式*/
//	NB_CGSN,						/*查询IMEI号*/
//	NB_CGATT,						/*查询附着状态*/
	NB_CEREG,						/*查询注册网络状态*/
	NB_CSQ,							/*查询信号强度*/
	NB_TCPEN,                       /*创建TCP连接*/
	NB_SERVER,                      /*设置TCP SERVER地址端口号*/
	NB_CONNECT						/*网络连接状态*/
}AT_link_ENUM;

/*********************函数声明****************************************************/
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
/*********************外部变量声明*************************************************/

#endif
