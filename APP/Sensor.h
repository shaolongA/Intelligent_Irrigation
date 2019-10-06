#ifndef __SENSOR_H
#define __SENSOR_H	 

//485电源控制
#define PORT_V485_EN	GPIOA
#define PIN_V485_EN		GPIO_Pin_4
#define V485_EN_HIGH	GPIO_SetBits(PORT_V485_EN, PIN_V485_EN);
#define V485_EN_LOW		GPIO_ResetBits(PORT_V485_EN, PIN_V485_EN);
//UART2对应rs485:IO端口
#define PORT_RS485_CTRL2	GPIOE
#define PIN_RS485_CTRL2		GPIO_Pin_5
#define RS485_CTRL2_HIGH	GPIO_SetBits(PORT_RS485_CTRL2, PIN_RS485_CTRL2);
#define RS485_CTRL2_LOW		GPIO_ResetBits(PORT_RS485_CTRL2, PIN_RS485_CTRL2);
//UART3对应rs485:IO端口
#define PORT_RS485_CTRL3	GPIOC
#define PIN_RS485_CTRL3		GPIO_Pin_7
#define RS485_CTRL3_HIGH	GPIO_SetBits(PORT_RS485_CTRL3, PIN_RS485_CTRL3);
#define RS485_CTRL3_LOW		GPIO_ResetBits(PORT_RS485_CTRL3, PIN_RS485_CTRL3);



void SensorInit(void);
unsigned char GetSensorValue(void);


void RS485PrInit(void);
void RS485PrDeinit(void);
void RS485PrintCh(unsigned char Ch);
void RS485PrintStr(unsigned char *Str,unsigned int lenth);

#endif
