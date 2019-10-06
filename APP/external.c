#include "stm8l15x.h"
#include "external.h"
/*******************************************************************************
**函数名称：void Smoke1_Input_Init() 
**功能描述：配置发送中断设置
**入口参数：
**输出：无
*******************************************************************************/
void External_Interrupt_Input_Init()
{
	GPIO_Init(PORT_EXTI_KEY , PIN_EXTI_KEY , GPIO_Mode_In_FL_IT);
	EXTI_SetPinSensitivity(EXTI_KEY_LINE, EXTI_Trigger_Falling);
	EXTI_SetHalfPortSelection(EXTI_HalfPort_D_LSB , DISABLE); 
}






