#ifndef __EEPROM_H
#define __EEPROM_H	

/**********************************************************************************/

#define PLAN_BYTES_LEN		0x10
#define EEPROM_PLAN_ADDR	0x1080

#define CTRL_CFG_LEN		0x0f
#define EEPROM_CTRL_ADDR	0x1051
#define EEPROM_SOURCE_ID_ADDR	0x1061

void EEPROM_Write_str(unsigned int address,unsigned char *date,unsigned char lenth);
void EEPROM_Read_str(unsigned int address,unsigned char *date,unsigned char lenth);
void EEPROM_Byte_Write(unsigned int address , unsigned char date);
void Write_EEPROM(unsigned char *data);
void Clear_PlanEEPROM(void);
void Read_EEPROM(void);
void Update_EEPROM(unsigned char i,unsigned char data);
unsigned char ReadFreeSpace(void);
void ReadConfigEEPROM(void);
unsigned char WriteConfigEEPROM(void);

#endif



