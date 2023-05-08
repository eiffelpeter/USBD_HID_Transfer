/**************************************************************************//**
 * @file     timer_ctrl.c
 * @version  V1.00
 * $Revision: $
 * $Date: $
 * @brief 
 * @note
 * Copyright (C) All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"

static volatile uint32_t _systick_co =  0;

void TMR0_IRQHandler(void)
{
    _systick_co++;
    /* clear timer interrupt flag */
    TIMER_ClearIntFlag(TIMER0);
}


uint32_t systick_ms(void)
{
    return _systick_co;
}


void TIMER_SET_DELAY(uint32_t delay_msec)
{
    uint32_t cur_tick;

    cur_tick = _systick_co;
    while ( ( systick_ms() - cur_tick ) < delay_msec);
}

void TIMER_Setup(void)
{
    /* Set timer frequency to 1:1Hz 1000:1kHz */
    TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 1000);   // 1ms

    /* Enable timer interrupt */
    TIMER_EnableInt(TIMER0);
    NVIC_EnableIRQ(TMR0_IRQn);

    /* Start Timer 0 */
    TIMER_Start(TIMER0);
}
