#ifndef __ADC_H
#define __ADC_H	 


#define PORT_BAT_ADCCON				GPIOD
#define PIN_BAT_ADCCON				GPIO_Pin_1
#define PORT_BAT_ADCIN				GPIOD
#define PIN_BAT_ADCIN				GPIO_Pin_0

//AD�ɼ����ء�����AD�ɼ��ϵ�MOS�ܣ���ͨ��ʱ����е�ѹֵ���Բɼ���
#define ADCBat_OFF		GPIO_ResetBits(PORT_BAT_ADCCON,PIN_BAT_ADCCON)			//�ر�MOS
#define ADCBat_ON		GPIO_SetBits(PORT_BAT_ADCCON,PIN_BAT_ADCCON)			//��MOS��


void ADCConver_Init(void);
unsigned int ADC_Data_Read(void);
unsigned int GetBatValueAve(void);

#endif
