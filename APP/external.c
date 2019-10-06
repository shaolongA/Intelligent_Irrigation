#include "stm8l15x.h"
#include "external.h"
/*******************************************************************************
**�������ƣ�void Smoke1_Input_Init() 
**�������������÷����ж�����
**��ڲ�����
**�������
*******************************************************************************/
void External_Interrupt_Input_Init()
{
	GPIO_Init(PORT_EXTI_KEY , PIN_EXTI_KEY , GPIO_Mode_In_FL_IT);
	EXTI_SetPinSensitivity(EXTI_KEY_LINE, EXTI_Trigger_Falling);
	EXTI_SetHalfPortSelection(EXTI_HalfPort_D_LSB , DISABLE); 
}






