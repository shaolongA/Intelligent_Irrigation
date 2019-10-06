#ifndef __EXTERNAL_H
#define __EXTERNAL_H	 


//key2°´¼ü
#define PORT_EXTI_KEY		GPIOE
#define PIN_EXTI_KEY		GPIO_Pin_2
#define EXTI_KEY_LINE		EXTI_Pin_2
#define ReadKeyVal			GPIO_ReadInputData(PORT_EXTI_KEY)
#define RepBat_KEY			PIN_EXTI_KEY

void External_Interrupt_Input_Init();

#endif
