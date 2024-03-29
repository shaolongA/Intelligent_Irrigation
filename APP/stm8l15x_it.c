/**
  ******************************************************************************
  * @file    GPIO/GPIO_Toggle/stm8l15x_it.c
  * @author  MCD Application Team
  * @version V1.3.0
  * @date    07/14/2010
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all peripherals interrupt service routine.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */

/* Includes ------------------------------------------------------------------*/
#include "stm8l15x_it.h"
#include "rtc.h"
#include "uart.h"
#include "timer2.h"  
extern void delay_ms(unsigned int ms);
//extern unsigned char g_RxInterruptFlag;
extern unsigned char gKey2;
extern unsigned int  Press_Time_Count1;
/** @addtogroup STM8L15x_StdPeriph_Examples
  * @{
  */

/** @addtogroup GPIO_Toggle
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

#ifdef _COSMIC_
/**
  * @brief  Dummy interrupt routine
  * @param  None
  * @retval None
*/
INTERRUPT_HANDLER(NonHandledInterrupt, 0)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}
#endif

/**
  * @brief  TRAP interrupt routine
  * @param  None
  * @retval None
*/
INTERRUPT_HANDLER_TRAP(TRAP_IRQHandler)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}
/**
  * @brief  FLASH Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(FLASH_IRQHandler, 1)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}
/**
  * @brief  DMA1 channel0 and channel1 Interrupt routine.
  * @param  None
  * @retval None
  */
extern volatile  unsigned short  ADC_ConvertedValue[ ];
extern volatile  float ADC1_Channel1_Battery,ADC1_Channel_Vrefint;
 

INTERRUPT_HANDLER(DMA1_CHANNEL0_1_IRQHandler, 2)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  
  


}
/**
  * @brief  DMA1 channel2 and channel3 Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(DMA1_CHANNEL2_3_IRQHandler, 3)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}
/**
  * @brief  RTC Interrupt routine.
  * @param  None
  * @retval None
  */


INTERRUPT_HANDLER(RTC_IRQHandler, 4)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
	RTC_ClearITPendingBit(RTC_IT_WUT);
	RTC_SetWakeUpCounter(2047);
	RTC_WakeUpCmd(ENABLE);
	RTC_UpdateCounter();
}
/**
  * @brief  External IT PORTE/F and PVD Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(EXTIE_F_PVD_IRQHandler, 5)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  if( PWR_PVDGetITStatus( ) == SET)//如果是PVD的中断
  {
    if(PWR_GetFlagStatus(PWR_FLAG_PVDOF) == SET)//电源电压低于PVD
    {
       //此处添加报警处理
      
      //LCD_GLASS_DisplayString( "PWR"); //test
      
    }
    
    PWR_PVDClearFlag( );//清除PVD中断标志位
  }
  
  
  
}

/**
  * @brief  External IT PORTB Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(EXTIB_IRQHandler, 6)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}

/**
  * @brief  External IT PORTD Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(EXTID_IRQHandler, 7)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}

/**
  * @brief  External IT PIN0 Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(EXTI0_IRQHandler, 8)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}

/**
  * @brief  External IT PIN1 Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(EXTI1_IRQHandler, 9)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}

/**
  * @brief  External IT PIN2 Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(EXTI2_IRQHandler, 10)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
	if(EXTI_GetITStatus(EXTI_IT_Pin2)!= RESET)
        {
  if( GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2)== 0)
  {
    delay_ms(10);//if(GPIO_ReadInputDataBit(GPIOD , GPIO_Pin_0) == 0)
      if( GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2)== 0)
	{
//		g_RxInterruptFlag=1;
////          #if	NB_PRINTF
////		RS485PrInit();
////		USART3_sendstr("....11111111111....\r\n");
////		RS485PrDeinit();
////#endif
		gKey2=1;
		Press_Time_Count1=0;
		TIM2_Init();
	}  
  }
		EXTI_ClearITPendingBit(EXTI_IT_Pin2);  //清除标志位 
        }
}

/**
  * @brief  External IT PIN3 Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(EXTI3_IRQHandler, 11)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}

/**
  * @brief  External IT PIN4 Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(EXTI4_IRQHandler, 12)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}

/**
  * @brief  External IT PIN5 Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(EXTI5_IRQHandler, 13)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}

/**
  * @brief  External IT PIN6 Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(EXTI6_IRQHandler, 14)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}

/**
  * @brief  External IT PIN7 Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(EXTI7_IRQHandler, 15)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}
/**
  * @brief  LCD start of new frame Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(LCD_IRQHandler, 16)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}
/**
  * @brief  CLK switch/CSS/TIM1 break Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(SWITCH_CSS_BREAK_DAC_IRQHandler, 17)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  //判断是否是时钟切换中断
  if(CLK->SWCR & CLK_SWCR_SWIF == 0X08)
  {
  // LCD_GLASS_DisplayString("switch!"); 
   CLK->SWCR &= (~CLK_SWCR_SWIF);//清除中断标志位
  }    
}

/**
  * @brief  ADC1/Comparator Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(ADC1_COMP_IRQHandler, 18)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}

/**
  * @brief  TIM2 Update/Overflow/Trigger/Break Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(TIM2_UPD_OVF_TRG_BRK_IRQHandler, 19)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
//	if(TIM2_GetFlagStatus(TIM2_FLAG_Update) != RESET)
	if(TIM2_GetITStatus(TIM2_IT_Update) != RESET)
	{
		TIM2_ClearITPendingBit(TIM2_IT_Update);                             //清除中断标志
		TIM2_interrupt();
	}
}

/**
  * @brief  Timer2 Capture/Compare Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(TIM2_CAP_IRQHandler, 20)
//INTERRUPT_HANDLER(TIM2_CC_USART2_RX_IRQHandler, 20)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
	unsigned char Res;
	USART_ClearITPendingBit(USART2,USART_IT_OR);
	if(USART_GetITStatus(USART2,USART_IT_RXNE) == SET)
	{
		USART_ClearITPendingBit(USART2,USART_IT_OR);
		USART_ClearITPendingBit(USART2,USART_IT_RXNE);
		Res = USART_ReceiveData8(USART2);
		UartRxISR(&uart_sensor,Res);
	}
}


/**
  * @brief  Timer3 Update/Overflow/Trigger/Break Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(TIM3_UPD_OVF_TRG_BRK_IRQHandler, 21)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}
/**
  * @brief  Timer3 Capture/Compare Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(TIM3_CAP_IRQHandler, 22)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
	unsigned char Res;
	if(USART_GetITStatus(USART3,USART_IT_RXNE) == SET)
	{
		USART_ClearITPendingBit(USART3,USART_IT_OR);
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);
		Res = USART_ReceiveData8(USART3);
		UartRxISR(&uart_sensor,Res);
	}
}
/**
  * @brief  TIM1 Update/Overflow/Trigger/Commutation Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(TIM1_UPD_OVF_TRG_COM_IRQHandler, 23)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}
/**
  * @brief  TIM1 Capture/Compare Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(TIM1_CAP_IRQHandler, 24)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}

/**
  * @brief  TIM4 Update/Overflow/Trigger Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(TIM4_UPD_OVF_TRG_IRQHandler, 25)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  
//  ms_count++;                 
//  if(ms_count >= 1000)      //记录1秒定时已到
//  {
//    ms_count = 0;
//    time_count++;
//    if( time_count >= 255)
//    {
//      time_count  = 0;
//    }
//    timeflag = 1;           //存储标志置位
//  }
  TIM4_ClearITPendingBit(TIM4_IT_Update); //清除中断标志
  
  
}
/**
  * @brief  SPI1 Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(SPI1_IRQHandler, 26)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}

/**
  * @brief  USART1 TX Interrupt routine.
  * @param  None
  * @retval None
  */ 

INTERRUPT_HANDLER(USART1_TX_IRQHandler, 27)
{
 
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */

   
  
}

/**
  * @brief  USART1 RX Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(USART1_RX_IRQHandler, 28)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */ 
	unsigned char Res;
	if(USART_GetITStatus(USART1,USART_IT_RXNE) == SET)
	{
		USART_ClearITPendingBit(USART1,USART_IT_OR);
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
		Res = USART_ReceiveData8(USART1);
		UartRxISR(&uart_gprs,Res);
	}
}

/**
  * @brief  I2C1 Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(I2C1_IRQHandler, 29)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
}

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/

