#ifndef PWM_CTRL_H
#define PWM_CTRL_H
#include <stdio.h>
#include "NuMicro.h"

#define PWM0_CHANNEL_MASK   0x01    // 0
#define PWM1_CHANNEL_MASK   0x03    // 
#define PWM_FREQ	        20000   // 20k

#define LED_WHITE_BIT       0x01
#define LED_BLUE_BIT        0x02

#define SCALE_TO_SECOND         100  // use for LED blink and ALT    10 : tmr0 is 100ms   50 : tmr0 is 20ms  67 : tmr0 is 15ms  100 : tmr0 is 10ms 
#define BLINK_PER_SEC           4   // 2 blink in 1 sec
#define ALT_PER_SEC             2   // 1 alt in 1 sec

// i2c data of REG_STATUS_LED
typedef enum
{
    STATUS_LED_OFF                 = 0,                
    STATUS_LED_BLUE                = 1,                
    STATUS_LED_WHITE               = 2,                
    STATUS_LED_BLUE_BLINK          = 3,                
    STATUS_LED_WHITE_BLINK         = 4,                
    STATUS_LED_BLUE_BREATH         = 5,                
    STATUS_LED_WHITE_BREATH        = 6,                
    STATUS_LED_ALT_BLUE_WHITE      = 7,               
    STATUS_LED_BLUE_WHITE          = 8,
    STATUS_LED_ALL                 = 9,
    STATUS_LED_BLUE_FLASH          = 10
} status_led_t;

void PWM_Init(void);
void PWM_Loop(void);
void PWM_Set_Blue_Led(uint8_t duty);
void PWM_Set_White_Led(uint8_t duty);
void PWM_Set_Buzzer_Freq(uint16_t freq, uint8_t duty);
uint16_t PWM_Get_Buzzer_Freq(void);

void PWM_Set_Status_Led(uint8_t status);
uint8_t PWM_Get_Status_Led(void);
void PWM_Set_Status_Led_Period(uint8_t period);
uint8_t PWM_Get_Status_Led_Period(void);
void PWM_Set_Status_Led_Level(uint8_t level);
uint8_t PWM_Get_Status_Led_Level(void);
void PWM_Set_Status_Led_Breath_Max(uint8_t level);
uint8_t PWM_Get_Status_Led_Breath_Max(void);
#endif
