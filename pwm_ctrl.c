/**************************************************************************//**
 * @file     pwm_ctrl.c
 * @version  V1.00
 * $Revision: $
 * $Date: $
 * @brief    PWM counter output waveform.
 * @note
 * Copyright (C) 2018 Ubiquiti network. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"
#include "pwm_ctrl.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/

static unsigned char ubnt_LATC;
static int led_timer, breath_timer=0;
static char fadeAmount;
static char breath_step = 2;
static unsigned char breathing_duty = 0, breath_count;
static unsigned char led_period = 5, led_status = STATUS_LED_OFF;//STATUS_LED_BLUE_BREATH ;
static unsigned char led_level = 100,  led_apply_change=1;
static unsigned char breathing_max= 100;
static uint16_t buzzer_freq = 0;
/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/


void PWM_Set_Status_Led(uint8_t status)
{
    led_status = status;
    led_apply_change = 1;
}

uint8_t PWM_Get_Status_Led(void)
{
    return led_status;
}

void PWM_Set_Status_Led_Period(uint8_t period)
{
    led_period = period;
}

uint8_t PWM_Get_Status_Led_Period(void)
{
    return led_period;
}

void PWM_Set_Status_Led_Level(uint8_t level)
{
    led_level = level;
    led_apply_change = 1;
}

uint8_t PWM_Get_Status_Led_Level(void)
{
    return led_level;
}

void PWM_Set_Status_Led_Breath_Max(uint8_t level)
{
    breathing_max = level;
    led_apply_change = 2;
}

uint8_t PWM_Get_Status_Led_Breath_Max(void)
{
    return breathing_max;
}


void PWM_Init(void)
{
    breath_count = (led_period - 1);

    /* Enable PWM0 module clock */
    CLK_EnableModuleClock(PWM0_MODULE);
    CLK_EnableModuleClock(PWM1_MODULE);

    /*---------------------------------------------------------------------------------------------------------*/
    /* PWM clock frequency configuration                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Select HCLK clock source as PLL and and HCLK clock divider as 2 */
//    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_PLL, CLK_CLKDIV0_HCLK(2));

    /* PWM clock frequency can be set equal or double to HCLK by choosing case 1 or case 2 */
    /* case 1.PWM clock frequency is set equal to HCLK: select PWM module clock source as PCLK */
    CLK_SetModuleClock(PWM0_MODULE, CLK_CLKSEL2_PWM0SEL_PCLK0, 0);
    CLK_SetModuleClock(PWM1_MODULE, CLK_CLKSEL2_PWM1SEL_PCLK1, 0);

    /* case 2.PWM clock frequency is set double to HCLK: select PWM module clock source as PLL */
    //CLK_SetModuleClock(PWM0_MODULE, CLK_CLKSEL2_PWM0SEL_PLL, NULL);
    /*---------------------------------------------------------------------------------------------------------*/

    /* Reset PWM0 module */
    SYS_ResetModule(PWM0_RST);
    SYS_ResetModule(PWM1_RST);

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Set PB multi-function pins for PWM0 Channel 0,1  */
    SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB5MFP_Msk);
    SYS->GPB_MFPL |= (SYS_GPB_MFPL_PB5MFP_PWM0_CH0);

    SYS->GPB_MFPH = (SYS->GPB_MFPH & (~SYS_GPB_MFPH_PB14MFP_Msk)) | SYS_GPB_MFPH_PB14MFP_PWM1_CH1;
    SYS->GPB_MFPH = (SYS->GPB_MFPH & (~SYS_GPB_MFPH_PB15MFP_Msk)) | SYS_GPB_MFPH_PB15MFP_PWM1_CH0;


    /* PWM0 channel 0,1 frequency and duty configuration are as follows */
    PWM_ConfigOutputChannel(PWM0, 0, PWM_FREQ, 0);    // freq and duty 
    PWM_ConfigOutputChannel(PWM1, 0, PWM_FREQ, 0);
    PWM_ConfigOutputChannel(PWM1, 1, PWM_FREQ, 0);

	 
    /* Enable output of PWM0 channel  */
    PWM_EnableOutput(PWM0, PWM0_CHANNEL_MASK);   // PWM_CHANNEL_MASK 0x2F
    PWM_EnableOutput(PWM1, PWM1_CHANNEL_MASK); 

    /* Start PWM0 counter */
    PWM_Start(PWM0, PWM0_CHANNEL_MASK);
    PWM_Start(PWM1, PWM1_CHANNEL_MASK);

    /* Start PWM0 counter */
    //PWM_ForceStop(PWM0, PWM_CHANNEL_MASK);
		
}


void PWM_Set_Blue_Led(uint8_t duty)   // PB14
{
    PWM_ConfigOutputChannel(PWM1, 1, PWM_FREQ, duty); 
}

void PWM_Set_White_Led(uint8_t duty)   // PB15
{
    PWM_ConfigOutputChannel(PWM1, 0, PWM_FREQ, duty); 
}


void PWM_Set_Buzzer_Freq(uint16_t freq, uint8_t duty)   // PB5
{
    if ((freq == 0) || (duty == 0))
    {
        PWM_ConfigOutputChannel(PWM0, 0, 100, 0);    // do not set freq to zero
    }
    else
    {
        PWM_ConfigOutputChannel(PWM0, 0, freq, duty); 
    }
		
		buzzer_freq = freq;
}

uint16_t PWM_Get_Buzzer_Freq(void)
{
	return buzzer_freq;
}


//
// low level LED calls, mainly to deal with the hardware
//

// for flashing and alternating
int ledBlueGet(void) {
   return (!!(ubnt_LATC & LED_BLUE_BIT));
}

void ledBlueSet(int level)
{
    if (level)
    {
        ubnt_LATC |= LED_BLUE_BIT;
    } 
    else if (0 == level) {
        ubnt_LATC &= ~LED_BLUE_BIT;
    }
    PWM_Set_Blue_Led(level);
}



void ledWhiteSet(int level)
{
  if (level)
  {
      ubnt_LATC |= LED_WHITE_BIT;
  } 
  else if (0 == level) {
      ubnt_LATC &= ~LED_WHITE_BIT;
  } 
  PWM_Set_White_Led(level);
}

int ledWhiteGet(void) {
   return (!!(ubnt_LATC & LED_WHITE_BIT));
}

void PWM_Loop(void)
{
    // LED change , set led off first
    if (led_apply_change)
    {
        //printf(" led_apply_change to:%d\r\n", led_apply_change);
        if ((led_status <= STATUS_LED_WHITE_BREATH) && (led_status >= STATUS_LED_BLUE_BREATH))             // breath
        {
            if (led_period > 1)
            {
                //breath_step = (2*breathing_max) / (((1000*(led_period - 1))/10)/4);   // calculate breath_step by 10ms
                breath_step = (2*breathing_max) / (25*(led_period - 1));
            }
            else if (led_period <= 1)
            {
                //breath_step = (2*breathing_max) / (((1000)/10)/4);   // calculate breath_step by 10ms
                breath_step = (2*breathing_max) / (25);
            }

            // printf(" breath_step :%d\r\n", breath_step);

            if (breath_step == 0) // force breath_step to 1 in special case
            {
                breath_step = 1;
                //breath_count = (breath_step*(1000*(led_period))) / (2*breathing_max*10);
                breath_count = (50*led_period) / (breathing_max);
            }
            else
            {
                breath_count = (led_period - 1);
            }

            //printf(" breath_count :%d\r\n", breath_count);

            if (led_apply_change == 1)  
            {
                breathing_duty = (led_level/breath_step)*breath_step;         // align to multi of breath_step
                if (breathing_duty >= (breathing_max - breath_step))
                {
                    fadeAmount = -breath_step;
                }
                else
                {
                    fadeAmount = breath_step;
                }
            }
            else // if (led_apply_change == 2)
            {
                ledBlueSet(0);   
                ledWhiteSet(0);  
                breath_timer = 0;
                breathing_duty = 0; 
                fadeAmount = breath_step;
            }
        }
        else
        {
            ledBlueSet(0);
            ledWhiteSet(0);
        }
        led_apply_change = 0;
            //fadeAmount = 250 * 15 * (led_period+1)* 2 / led_period / 1000;
            //fadeAmount = ((15 * (led_period+1)) / (led_period * 2)) ;
    }



    if ((led_status <= STATUS_LED_WHITE) && (led_status >= STATUS_LED_BLUE))                                // On
    {
        switch(led_status)
        {   

            case STATUS_LED_BLUE: 
                if (!ledBlueGet() || led_apply_change)
                {
                    ledWhiteSet(0);
                    ledBlueSet(led_level);
                }
                break;
            case STATUS_LED_WHITE:
                if (!ledWhiteGet() || led_apply_change)
                {
                    ledBlueSet(0);
                    ledWhiteSet(led_level);
                }
                break;
        }
    }
    else if ((led_status <= STATUS_LED_ALL) && (led_status >= STATUS_LED_BLUE_WHITE))        // Both                       
    {
        switch(led_status)
        {   

            case STATUS_LED_BLUE_WHITE:
                if (!ledBlueGet() || !ledWhiteGet())
                {
                    ledBlueSet(led_level);
                    ledWhiteSet(led_level);
                }
                break;
            case STATUS_LED_ALL:  
                if (!ledBlueGet() || !ledWhiteGet())
                {
                    ledBlueSet(led_level);
                    ledWhiteSet(led_level);
                }
                break;
        }
    }
    else if ((led_status <= STATUS_LED_WHITE_BREATH) && (led_status >= STATUS_LED_BLUE_BREATH))             // breath
    {
        if (breath_timer <= 0)
        {
            // change the brightness for next time through the loop:
            breathing_duty += fadeAmount;

            // reverse the direction of the fading at the ends of the fade:
            //if (breathing_duty <= 0 || breathing_duty >= 0x3f0) 
//          if ((breathing_duty <= 0) || (breathing_duty >= 170))   // do not set to 250 full scale for power saving at idle
//          {
//              fadeAmount = -fadeAmount;
//          }
            if (breathing_duty <= 0) 
            {
                fadeAmount = breath_step;
            }
            else if (breathing_duty >= (breathing_max - breath_step))   // do not set to 250 full scale for power saving at idle
            {
                fadeAmount = -breath_step;
            }
            //printf("breath breathing_duty:%d fadeAmount:%d breathing_max:%d\r\n",  breathing_duty, fadeAmount, breathing_max);

            switch(led_status)
            {
            case STATUS_LED_BLUE_BREATH:
                ledBlueSet(breathing_duty);
                break;
            case STATUS_LED_WHITE_BREATH:
                ledWhiteSet(breathing_duty);
                break;
            //printf("pwm breathing_duty 0x%x led_status:%d\r\n", breathing_duty, led_status);
            }
            //breath_timer = SCALE_TO_SECOND / (BREATH_PER_SEC);
            if (led_period == 0)
            {
                breath_timer = 0;
            }
            else
            {
                breath_timer = breath_count;
            }

        }
        else
        {
            breath_timer --;
            //printf("breath_timer:%d  \r\n", breath_timer);
        }
    }
    else if(led_status == STATUS_LED_BLUE_FLASH)
    {
        if (led_timer <= 0)
        {
            switch(led_status)
            {
            case STATUS_LED_BLUE_FLASH: 
                if (ledBlueGet())
                {
                    led_timer = 98; //SCALE_TO_SECOND / (BLINK_PER_SEC);
                    ledBlueSet(0);
                }
                else
                {
                    led_timer = 2; //SCALE_TO_SECOND / (BLINK_PER_SEC/2);
                    ledBlueSet(led_level);
                }
                break;
            }
            
        }
        else
        {
           // decrement at end, so we can set values elsewhere and rely on the above logic to control
           led_timer--;
        }
    }
    else if ((led_status <= STATUS_LED_WHITE_BLINK) && (led_status >= STATUS_LED_BLUE_BLINK))          // blink
    {
        if (led_timer <= 0)
        {
            switch(led_status)
            {
            case STATUS_LED_BLUE_BLINK: 
                if (ledBlueGet())
                    ledBlueSet(0);
                else
                    ledBlueSet(led_level);
                break;
            case STATUS_LED_WHITE_BLINK:
                if (ledWhiteGet())
                    ledWhiteSet(0);
                else
                    ledWhiteSet(led_level);
                break;
            }
            led_timer = SCALE_TO_SECOND / (BLINK_PER_SEC);
            //led_timer = SCALE_TO_SECOND / (1<<led_period);
            //printf("led_timer:%d led_period:%d \r\n", led_timer, led_period);
        }
        else
        {
           // decrement at end, so we can set values elsewhere and rely on the above logic to control
           led_timer--;
        }
    }
    else if ((led_status <= STATUS_LED_ALT_BLUE_WHITE) )            // ALT
    {
        if (led_timer <= 0)
        {
            switch(led_status)
            {
            case STATUS_LED_ALT_BLUE_WHITE:
                 if (ledBlueGet())                       
                 {                                         
                     ledBlueSet(0); 
                     ledWhiteSet(led_level);              
                 }                                         
                 else                                      
                 {                                         
                     ledBlueSet(led_level); 
                     ledWhiteSet(0);            
                 }
                break; 
            }
            led_timer = SCALE_TO_SECOND / (ALT_PER_SEC);
            //printf("led_timer:%d led_period:%d \r\n", led_timer, led_period);
        }
        else
        {
            // decrement at end, so we can set values elsewhere and rely on the above logic to control
            led_timer--;
        }
    }
    else  // off
    {
        if (ledBlueGet())
          ledBlueSet(0);
        if (ledWhiteGet())
          ledWhiteSet(0);
    }
}

