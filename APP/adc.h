#ifndef __ADC_H
#define __ADC_H	 


#define PORT_BAT_ADCCON				GPIOD
#define PIN_BAT_ADCCON				GPIO_Pin_1
#define PORT_BAT_ADCIN				GPIOD
#define PIN_BAT_ADCIN				GPIO_Pin_0

//AD采集开关。控制AD采集上的MOS管，导通的时候才有电压值可以采集。
#define ADCBat_OFF		GPIO_ResetBits(PORT_BAT_ADCCON,PIN_BAT_ADCCON)			//关闭MOS
#define ADCBat_ON		GPIO_SetBits(PORT_BAT_ADCCON,PIN_BAT_ADCCON)			//打开MOS管


void ADCConver_Init(void);
unsigned int ADC_Data_Read(void);
unsigned int GetBatValueAve(void);

#endif
