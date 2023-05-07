#ifndef TIMER_CTRL_H
#define TIMER_CTRL_H
#include <stdio.h>
#include "NuMicro.h"

uint32_t systick_ms(void);
void TIMER_SET_DELAY(uint32_t delay_msec);
void TIMER_Setup(void);

#endif
