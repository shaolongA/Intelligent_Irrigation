/*******************************************************************************
*                       USART������USART1&USART1��
********************************************************************************
*��˾������Ƽ�
*ʱ�䣺2016-12-16
*******************************************************************************/

#ifndef UART_H
#define UART_H

/************************�궨��***************************************************/

#define USART1_CLK      CLK_Peripheral_USART1
#define PORT_USART1_RX  GPIOC
#define PIN_USART1_RX   GPIO_Pin_2
#define PORT_USART1_TX  GPIOC
#define PIN_USART1_TX   GPIO_Pin_3

#define USART2_CLK      CLK_Peripheral_USART2
#define PORT_USART2_RX  GPIOE
#define PIN_USART2_RX   GPIO_Pin_3
#define PORT_USART2_TX  GPIOE
#define PIN_USART2_TX   GPIO_Pin_4
//#define PORT_USART2_EN  GPIOE
//#define PIN_USART2_EN   GPIO_Pin_5

#define USART3_CLK      CLK_Peripheral_USART3
#define PORT_USART3_RX  GPIOE
#define PIN_USART3_RX   GPIO_Pin_7
#define PORT_USART3_TX  GPIOE
#define PIN_USART3_TX   GPIO_Pin_6
//#define PORT_USART3_EN  GPIOC
//#define PIN_USART3_EN   GPIO_Pin_7


#define RX_BUF_LEN 200 // ������ջ���������
#define TX_BUF_LEN 1  // ���巢�ͻ���������

typedef struct stUart // ���ն������ݽṹ
{
  volatile	unsigned char RxBuf[RX_BUF_LEN];
  volatile	unsigned char TxBuf[TX_BUF_LEN];
  volatile	unsigned char *pRxWr;
  volatile	unsigned char *pRxRd;
  volatile	unsigned char *pTxWr;
  volatile	unsigned char *pTxRd;
  //unsigned int RxCnt;
}stUart;


/************************ȫ�ֱ���**************************************************/

extern stUart uart_gprs;
extern stUart uart_sensor;
/************************��������**************************************************/
void USART1_Init(unsigned int baudrate);
void USART1_sendbuf(unsigned char *Str,unsigned int lenth);
void USART1_sendstr(unsigned char *p);
void USART1_sendchar(unsigned char c);

void USART2_Init(unsigned int baudrate);
void USART2_sendbuf(unsigned char *Str,unsigned int lenth);
void USART2_sendstr(unsigned char *p);
void USART2_sendchar(unsigned char c);
void USART2_LowPower(void);

void USART3_Init(unsigned long baudrate);
void USART3_sendbuf(unsigned char *Str,unsigned int lenth);
void USART3_sendstr(unsigned char *p);
void USART3_sendchar(unsigned char c);
void USART3_sendhex(unsigned char dat);
void USART3_LowPower(void);

/************************��������**************************************************/
unsigned int UartGetRxLen(stUart *st);
unsigned int UartGetTxLen(stUart *st);
void ResetUartBuf(stUart *st);
unsigned int UartRead(stUart *st, unsigned char *buf, unsigned int len);
unsigned int UartSend(stUart *st, unsigned char *buf, unsigned int len);
void UartRxISR(stUart *st,char c);

void RS485PrInit(void);
void RS485PrDeinit(void);

//################################################################################/


#endif
