#ifndef __RTC_H
#define __RTC_H

#define TOTAL_SPACE 10

#define HALFHOUR	30
#define ANHOUR		1
#define AMIN		60

typedef volatile unsigned char	vu8;
typedef volatile unsigned int	vu16;
typedef volatile unsigned long	vu32;
//时间结构体
typedef struct
{
    vu8 hour;
    vu8 min;
    vu8 sec;
    //公历日月年周
    vu16 w_year;
    vu8  w_month;
    vu8  w_date;
    vu8  week;
} _calendar_obj;

extern unsigned char GetBat_Flag;
extern unsigned char HeartBeatFlag;
extern _calendar_obj calendar;

void RTC_CLOCK_Init(void);
void RTC_UpdateCounter(void);
u8 RTC_Get(void);
u32 RTC_GetCounter(void);
unsigned long RTC_GetSec(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec);
void RTC_Adjust(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec);
u8 RTC_Get_calendar(u32 timecount,_calendar_obj *prcalendar);
#endif


