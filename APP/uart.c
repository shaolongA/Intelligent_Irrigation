/*******************************************************************************
*                       USART������USART1&USART1��
********************************************************************************
*��˾������Ƽ�
*ʱ�䣺2016-12-16
*******************************************************************************/

/************************ͷ�ļ�*************************************************/
#include "uart.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stm8l15x.h"

/************************ȫ�ֱ���*************************************************/
unsigned char HexTable[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

/*****************************************************************************
* ��������: void Init_USART1(void)
* ��������: USART1��ʼ������ 115200bps GPRSģ��ӿ�
* ��    ��:
* ����  ֵ:
*****************************************************************************/
void USART1_Init(unsigned int baudrate)
{
	USART_DeInit(USART1);
	CLK_PeripheralClockConfig(USART1_CLK , ENABLE);		//ʹ��USART1ʱ��
	GPIO_Init(PORT_USART1_RX , PIN_USART1_RX , GPIO_Mode_In_PU_No_IT);			//����PC2�������ģʽ  rx
	GPIO_Init(PORT_USART1_TX , PIN_USART1_TX , GPIO_Mode_Out_PP_Low_Fast);		//����PC3�������ģʽ  tx  
	GPIO_SetBits(PORT_USART1_RX, PIN_USART1_RX);
	GPIO_SetBits(PORT_USART1_TX, PIN_USART1_TX);
	USART_Init(USART1,						//����USART1
				baudrate,					//����������
				USART_WordLength_8b,		//���ݳ�����Ϊ8λ
				USART_StopBits_1,			//1λֹͣλ
				USART_Parity_No,			//��У��
				USART_Mode_Tx | USART_Mode_Rx
				);							//����Ϊ���ͽ���˫ģʽ  
	USART_ITConfig(USART1, USART_IT_RXNE , ENABLE);
	USART_Cmd(USART1 , ENABLE);
}
/*****************************************************************************
* ��������: void Init_USART2(void)
* ��������: USART2��ʼ������ 9600bps ���õ��Խӿ�
* ��    ��:
* ����  ֵ:
*****************************************************************************/
void USART2_Init(unsigned int baudrate)
{
	USART_DeInit(USART2);
	CLK_PeripheralClockConfig(USART2_CLK , ENABLE);		//ʹ��USART2ʱ��
	GPIO_Init(PORT_USART2_RX , PIN_USART2_RX , GPIO_Mode_In_PU_No_IT);
	GPIO_Init(PORT_USART2_TX , PIN_USART2_TX , GPIO_Mode_Out_PP_Low_Fast);
	GPIO_SetBits(PORT_USART2_RX, PIN_USART2_RX);
	GPIO_SetBits(PORT_USART2_TX, PIN_USART2_TX);
	USART_Init(USART2,						//����USART2
				baudrate,					//����������
				USART_WordLength_8b,		//���ݳ�����Ϊ8λ
				USART_StopBits_1,			//1λֹͣλ
				USART_Parity_No,			//��У��
				USART_Mode_Tx | USART_Mode_Rx
				);							//����Ϊ���ͽ���˫ģʽ  
	USART_ITConfig(USART2, USART_IT_RXNE , ENABLE);
	USART_Cmd(USART2 , ENABLE);
}
void USART2_LowPower(void)
{
	USART_DeInit(USART2);
	USART_ITConfig(USART2, USART_IT_RXNE , DISABLE);
	USART_Cmd(USART2 , DISABLE);
	CLK_PeripheralClockConfig(USART2_CLK , DISABLE);
	GPIO_Init(PORT_USART2_RX , PIN_USART2_RX , GPIO_Mode_Out_PP_Low_Fast);
	GPIO_Init(PORT_USART2_TX , PIN_USART2_TX , GPIO_Mode_Out_PP_Low_Fast);
	GPIO_ResetBits(PORT_USART2_RX, PIN_USART2_RX);
	GPIO_ResetBits(PORT_USART2_TX, PIN_USART2_TX);
}
/*****************************************************************************
* ��������: void Init_USART3(void)
* ��������: USART3��ʼ������ 9600bps ���õ��Խӿ�
* ��    ��:
* ����  ֵ:
*****************************************************************************/
void USART3_Init(unsigned long baudrate)
{
	USART_DeInit(USART3);
	CLK_PeripheralClockConfig(USART3_CLK , ENABLE);		//ʹ��USART3ʱ��
	GPIO_Init(PORT_USART3_RX , PIN_USART3_RX , GPIO_Mode_In_PU_No_IT);			//����PC2�������ģʽ  rx
	GPIO_Init(PORT_USART3_TX , PIN_USART3_TX , GPIO_Mode_Out_PP_Low_Fast);		//����PC3�������ģʽ  tx  
	GPIO_SetBits(PORT_USART3_RX, PIN_USART3_RX);
	GPIO_SetBits(PORT_USART3_TX, PIN_USART3_TX);
	USART_Init(USART3,						//����USART3
				baudrate,					//����������
				USART_WordLength_8b,		//���ݳ�����Ϊ8λ
				USART_StopBits_1,			//1λֹͣλ
				USART_Parity_No,			//��У��
				USART_Mode_Tx | USART_Mode_Rx
				);							//����Ϊ���ͽ���˫ģʽ  
//	USART_ITConfig(USART3, USART_IT_RXNE , ENABLE);
	USART_Cmd(USART3 , ENABLE);
}
void USART3_LowPower(void)
{
	USART_DeInit(USART3);
	USART_ITConfig(USART3, USART_IT_RXNE , DISABLE);
	USART_Cmd(USART3 , DISABLE);
	CLK_PeripheralClockConfig(USART3_CLK , DISABLE);
	GPIO_Init(PORT_USART3_RX , PIN_USART3_RX , GPIO_Mode_Out_PP_Low_Fast);
	GPIO_Init(PORT_USART3_TX , PIN_USART3_TX , GPIO_Mode_Out_PP_Low_Fast);
	GPIO_SetBits(PORT_USART3_RX, PIN_USART3_RX);
	GPIO_SetBits(PORT_USART3_TX, PIN_USART3_TX);
}
/*****************************************************************************
* ��������: void USART1_sendchar(unsigned char c)
* ��������: ����1�����ֽ�
* ��    ��:
* ����  ֵ:
*****************************************************************************/
void USART1_sendchar(unsigned char c)
{
	unsigned int retry=0;
	USART_SendData8(USART1, c);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == 0)// ���ж��Ƿ�Ϊ��
	{
		retry++;
		if(retry > 10000)
		return ;
	}
}
/*****************************************************************************
* ��������: void USART1_sendbuf(unsigned char *Str,unsigned int lenth) 
* ��������: ����1�����ַ���
* ��    ��:
* ����  ֵ:
*****************************************************************************/
void USART1_sendbuf(unsigned char *Str,unsigned int lenth)     
{
	unsigned int i=0;
	for(i=0;i<lenth;i++)
	{
		USART1_sendchar(Str[i]);
	}
}
void USART1_sendstr(unsigned char *p)
{
	while(*p)
	USART1_sendchar(*(p++));
}
/*****************************************************************************
* ��������: void USART3_sendchar(unsigned char c)
* ��������: ����3�����ַ�
* ��    ��:
* ����  ֵ:
*****************************************************************************/
void USART2_sendchar(unsigned char c)
{
	unsigned int retry=0;
	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == 0)
	{
		retry++;
		if(retry > 10000)
		return ;
	}
	USART_SendData8(USART2, c);
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == 0)
	{
		retry++;
		if(retry > 10000)
		return ;
	}
}
/*****************************************************************************
* ��������: void USART2_sendstr(unsigned char *p) 
* ��������: ����2�����ַ���
* ��    ��:
* ����  ֵ:
*****************************************************************************/
void USART2_sendbuf(unsigned char *Str,unsigned int lenth)
{
	unsigned int i=0;
	for(i=0;i<lenth;i++)
	{
		USART2_sendchar(Str[i]);
	}
}
void USART2_sendstr(unsigned char *p) 
{
	while(*p)
	USART2_sendchar(*(p++));
}
/*****************************************************************************
* ��������: void USART3_sendchar(unsigned char c)
* ��������: ����3�����ַ�
* ��    ��:
* ����  ֵ:
*****************************************************************************/
void USART3_sendchar(unsigned char c)
{
	unsigned int retry=0;
	while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == 0)
	{
		retry++;
		if(retry > 10000)
		return ;
	}
	USART_SendData8(USART3, c);
	while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == 0)
	{
		retry++;
		if(retry > 10000)
		return ;
	}
}
/*****************************************************************************
* ��������: void USART3_sendstr(unsigned char *p) 
* ��������: ����3�����ַ���
* ��    ��:
* ����  ֵ:
*****************************************************************************/
void USART3_sendbuf(unsigned char *Str,unsigned int lenth)
{
	unsigned int i=0;
	for(i=0;i<lenth;i++)
	{
		USART3_sendchar(Str[i]);
	}
}
void USART3_sendstr(unsigned char *p) 
{
	while(*p)
	USART3_sendchar(*(p++));
}
/*****************************************************************************
* ��������: void USART3_sendhex(unsigned char dat)
* ��������: ����3����ʮ��������
* ��    ��:
* ����  ֵ:
*****************************************************************************/
void USART3_sendhex(unsigned char dat)
{
	USART3_sendchar('0');	
	USART3_sendchar('x');	
	USART3_sendchar(HexTable[dat>>4]);	
	USART3_sendchar(HexTable[dat&0x0f]);	
	USART3_sendchar(' ');
}

//##############################################################################//

/*****************************************************************************
* ��������: unsigned int UartGetRxLen(stUart *st)
* ��������: ��ý������ݳ���
* ��    ��:
* ����  ֵ:
*****************************************************************************/
unsigned int UartGetRxLen(stUart *st)
{
	unsigned int rxcnt;
	if(st->pRxWr >= st->pRxRd)
		rxcnt = st->pRxWr - st->pRxRd;
	else
		rxcnt = st->pRxWr + sizeof(st->RxBuf) - st->pRxRd;
	return rxcnt;
}

/*****************************************************************************
* ��������: unsigned int UartGetTxLen(stUart *st)
* ��������: ��÷������ݳ���
* ��    ��:
* ����  ֵ:
*****************************************************************************/
unsigned int UartGetTxLen(stUart *st)
{
	return 0;
}

/*****************************************************************************
* ��������: void ResetUartBuf(stUart *st)
* ��������: ��λ���ڻ�����
* ��    ��:
* ����  ֵ:
*****************************************************************************/
void ResetUartBuf(stUart *st)
{
	memset((void *)(st->RxBuf), 0, RX_BUF_LEN);
	memset((void *)(st->TxBuf), 0, TX_BUF_LEN);
	st->pRxWr = st->RxBuf;
	st->pRxRd = st->RxBuf;
	st->pTxWr = st->TxBuf;
	st->pTxRd = st->TxBuf;
}

/*****************************************************************************
* ��������: unsigned int UartRead(stUart *st, unsigned char *buf, unsigned int len)
* ��������: ��ȡ���ڶ���
* ��    ��:
* ����  ֵ:
*****************************************************************************/
unsigned int UartRead(stUart *st, unsigned char *buf, unsigned int len)
{
	unsigned int rxcnt,rdlen,ret;
	unsigned char *p = buf;
	if(len == 0)
		return 0;
	rxcnt = UartGetRxLen(st);
	rdlen = (rxcnt < len) ? rxcnt : len;
	ret = rdlen;
	while(rdlen)
	{
		rdlen--;
		*p = *(st->pRxRd);
		p++;
		st->pRxRd++;
		if(st->pRxRd - st->RxBuf >= sizeof(st->RxBuf))
		st->pRxRd = st->RxBuf;
	}
	if(ret>0)
	{
#if	NB_PRINTF
		RS485PrInit();
		USART3_sendstr("NB_RX:");
		USART3_sendbuf(buf,ret);
		USART3_sendstr("\r\n");
		RS485PrDeinit();
#endif
	}
	return ret;
}

/*****************************************************************************
* ��������: unsigned int UartSend(stUart *st, unsigned char *buf, unsigned int len)
* ��������: ���ڷ�������
* ��    ��:
* ����  ֵ:
*****************************************************************************/
unsigned int UartSend(stUart *st, unsigned char *buf, unsigned int len)
{
	return 0;
}

/*****************************************************************************
* ��������: void UartRxISR(stUart *st,char c)
* ��������: �����ַ�ָ����д���
* ��    ��:
* ����  ֵ:
*****************************************************************************/
void UartRxISR(stUart *st,char c)
{	
	*(st->pRxWr) = c;
	st->pRxWr++;
	if(st->pRxWr - st->RxBuf >= sizeof(st->RxBuf))
		st->pRxWr = st->RxBuf;
	
	if(st->pRxRd == st->pRxWr)
	{	
		st->pRxRd++;
		if(st->pRxRd - st->RxBuf >= sizeof(st->RxBuf))
			st->pRxRd = st->RxBuf;
	}
}


//----------------------����----------------------------------------------------------------//

