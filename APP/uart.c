/*******************************************************************************
*                       USART驱动（USART1&USART1）
********************************************************************************
*公司：富邦科技
*时间：2016-12-16
*******************************************************************************/

/************************头文件*************************************************/
#include "uart.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stm8l15x.h"

/************************全局变量*************************************************/
unsigned char HexTable[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

/*****************************************************************************
* 函数名称: void Init_USART1(void)
* 功能描述: USART1初始化函数 115200bps GPRS模块接口
* 参    数:
* 返回  值:
*****************************************************************************/
void USART1_Init(unsigned int baudrate)
{
	USART_DeInit(USART1);
	CLK_PeripheralClockConfig(USART1_CLK , ENABLE);		//使能USART1时钟
	GPIO_Init(PORT_USART1_RX , PIN_USART1_RX , GPIO_Mode_In_PU_No_IT);			//设置PC2推完输出模式  rx
	GPIO_Init(PORT_USART1_TX , PIN_USART1_TX , GPIO_Mode_Out_PP_Low_Fast);		//设置PC3推完输出模式  tx  
	GPIO_SetBits(PORT_USART1_RX, PIN_USART1_RX);
	GPIO_SetBits(PORT_USART1_TX, PIN_USART1_TX);
	USART_Init(USART1,						//设置USART1
				baudrate,					//波特率设置
				USART_WordLength_8b,		//数据长度设为8位
				USART_StopBits_1,			//1位停止位
				USART_Parity_No,			//无校验
				USART_Mode_Tx | USART_Mode_Rx
				);							//设置为发送接收双模式  
	USART_ITConfig(USART1, USART_IT_RXNE , ENABLE);
	USART_Cmd(USART1 , ENABLE);
}
/*****************************************************************************
* 函数名称: void Init_USART2(void)
* 功能描述: USART2初始化函数 9600bps 配置调试接口
* 参    数:
* 返回  值:
*****************************************************************************/
void USART2_Init(unsigned int baudrate)
{
	USART_DeInit(USART2);
	CLK_PeripheralClockConfig(USART2_CLK , ENABLE);		//使能USART2时钟
	GPIO_Init(PORT_USART2_RX , PIN_USART2_RX , GPIO_Mode_In_PU_No_IT);
	GPIO_Init(PORT_USART2_TX , PIN_USART2_TX , GPIO_Mode_Out_PP_Low_Fast);
	GPIO_SetBits(PORT_USART2_RX, PIN_USART2_RX);
	GPIO_SetBits(PORT_USART2_TX, PIN_USART2_TX);
	USART_Init(USART2,						//设置USART2
				baudrate,					//波特率设置
				USART_WordLength_8b,		//数据长度设为8位
				USART_StopBits_1,			//1位停止位
				USART_Parity_No,			//无校验
				USART_Mode_Tx | USART_Mode_Rx
				);							//设置为发送接收双模式  
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
* 函数名称: void Init_USART3(void)
* 功能描述: USART3初始化函数 9600bps 配置调试接口
* 参    数:
* 返回  值:
*****************************************************************************/
void USART3_Init(unsigned long baudrate)
{
	USART_DeInit(USART3);
	CLK_PeripheralClockConfig(USART3_CLK , ENABLE);		//使能USART3时钟
	GPIO_Init(PORT_USART3_RX , PIN_USART3_RX , GPIO_Mode_In_PU_No_IT);			//设置PC2推完输出模式  rx
	GPIO_Init(PORT_USART3_TX , PIN_USART3_TX , GPIO_Mode_Out_PP_Low_Fast);		//设置PC3推完输出模式  tx  
	GPIO_SetBits(PORT_USART3_RX, PIN_USART3_RX);
	GPIO_SetBits(PORT_USART3_TX, PIN_USART3_TX);
	USART_Init(USART3,						//设置USART3
				baudrate,					//波特率设置
				USART_WordLength_8b,		//数据长度设为8位
				USART_StopBits_1,			//1位停止位
				USART_Parity_No,			//无校验
				USART_Mode_Tx | USART_Mode_Rx
				);							//设置为发送接收双模式  
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
* 函数名称: void USART1_sendchar(unsigned char c)
* 功能描述: 串口1发送字节
* 参    数:
* 返回  值:
*****************************************************************************/
void USART1_sendchar(unsigned char c)
{
	unsigned int retry=0;
	USART_SendData8(USART1, c);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == 0)// 先判断是否为空
	{
		retry++;
		if(retry > 10000)
		return ;
	}
}
/*****************************************************************************
* 函数名称: void USART1_sendbuf(unsigned char *Str,unsigned int lenth) 
* 功能描述: 串口1发送字符串
* 参    数:
* 返回  值:
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
* 函数名称: void USART3_sendchar(unsigned char c)
* 功能描述: 串口3发送字符
* 参    数:
* 返回  值:
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
* 函数名称: void USART2_sendstr(unsigned char *p) 
* 功能描述: 串口2发送字符串
* 参    数:
* 返回  值:
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
* 函数名称: void USART3_sendchar(unsigned char c)
* 功能描述: 串口3发送字符
* 参    数:
* 返回  值:
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
* 函数名称: void USART3_sendstr(unsigned char *p) 
* 功能描述: 串口3发送字符串
* 参    数:
* 返回  值:
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
* 函数名称: void USART3_sendhex(unsigned char dat)
* 功能描述: 串口3发送十六进制数
* 参    数:
* 返回  值:
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
* 函数名称: unsigned int UartGetRxLen(stUart *st)
* 功能描述: 获得接收数据长度
* 参    数:
* 返回  值:
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
* 函数名称: unsigned int UartGetTxLen(stUart *st)
* 功能描述: 获得发送数据长度
* 参    数:
* 返回  值:
*****************************************************************************/
unsigned int UartGetTxLen(stUart *st)
{
	return 0;
}

/*****************************************************************************
* 函数名称: void ResetUartBuf(stUart *st)
* 功能描述: 复位串口缓冲区
* 参    数:
* 返回  值:
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
* 函数名称: unsigned int UartRead(stUart *st, unsigned char *buf, unsigned int len)
* 功能描述: 读取串口队列
* 参    数:
* 返回  值:
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
* 函数名称: unsigned int UartSend(stUart *st, unsigned char *buf, unsigned int len)
* 功能描述: 串口发送数据
* 参    数:
* 返回  值:
*****************************************************************************/
unsigned int UartSend(stUart *st, unsigned char *buf, unsigned int len)
{
	return 0;
}

/*****************************************************************************
* 函数名称: void UartRxISR(stUart *st,char c)
* 功能描述: 接收字符指针队列处理
* 参    数:
* 返回  值:
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


//----------------------结束----------------------------------------------------------------//

