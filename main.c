/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 12 $
 * $Date: 18/07/26 11:36a $
 * @brief    Demonstrate how to transfer data between USB device and PC through USB HID interface.
 *           A windows tool is also included in this sample code to connect with a USB device.
 *
 * @note
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#include <stdio.h>
#include "NuMicro.h"
#include "hid_transfer.h"
#include "pwm_ctrl.h"
#include "timer_ctrl.h"

#define CRYSTAL_LESS        1    /* CRYSTAL_LESS must be 1 if USB clock source is HIRC */
#define TRIM_INIT           (SYS_BASE+0x118)

extern uint8_t volatile g_u8Suspend;
int IsDebugFifoEmpty(void);

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable HIRC clock */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Waiting for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

    /* Switch HCLK clock source to HIRC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    /* Switch UART0 clock source to HIRC */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    /* Enable UART0 clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Enable IP clock */
    CLK_EnableModuleClock(TMR0_MODULE);

    /* Select IP clock source */
    CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);

    /* Switch USB clock source to HIRC & USB Clock = HIRC / 1 */
    CLK_SetModuleClock(USBD_MODULE, CLK_CLKSEL0_USBDSEL_HIRC, CLK_CLKDIV0_USB(1));

    /* Enable USB clock */
    CLK_EnableModuleClock(USBD_MODULE);

    /* Update System Core Clock */
    SystemCoreClockUpdate();

    /* Set PB multi-function pins for UART0 RXD=PB.12 and TXD=PB.13 */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk))
                    |(SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD);

    /* Lock protected registers */
    SYS_LockReg();
}


void PowerDown()
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    printf("Enter power down ...\n");
    while(!IsDebugFifoEmpty());

    /* Wakeup Enable */
    USBD_ENABLE_INT(USBD_INTEN_WKEN_Msk);

    CLK_PowerDown();

    /* Clear PWR_DOWN_EN if it is not clear by itself */
    if(CLK->PWRCTL & CLK_PWRCTL_PDEN_Msk)
        CLK->PWRCTL ^= CLK_PWRCTL_PDEN_Msk;

    printf("device wakeup!\n");

    /* Lock protected registers */
    SYS_LockReg();
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
    static uint32_t pwm_ms = 0;
#if CRYSTAL_LESS
    uint32_t u32TrimInit;
#endif
    /*
        This sample code demonstrate how to use HID interface to transfer data
        between PC and USB device.
        A demo window tool are also included in "WindowsTool" directory with this
        sample code. User can use it to test data transfer with this sample code.
    */

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Init System, peripheral clock and multi-function I/O */
    SYS_Init();

    PWM_Init();

    /* Init UART0 to 115200-8n1 for print message */
    UART_Open(UART0, 115200);

    TIMER_Setup();

    printf("\n");
    printf("+--------------------------------------------------------+\n");
    printf("|          NuMicro USB HID Transfer Sample Code          |\n");
    printf("+--------------------------------------------------------+\n");

    /* Open USB controller */
    USBD_Open(&gsInfo, HID_ClassRequest, NULL);

    /* Endpoint configuration */
    HID_Init();

    /* Start USB device */
    USBD_Start();

    NVIC_EnableIRQ(USBD_IRQn);

#if CRYSTAL_LESS
    /* Backup default trim */
    u32TrimInit = M32(TRIM_INIT);
#endif

    /* Clear SOF */
    USBD->INTSTS = USBD_INTSTS_SOFIF_Msk;

    while(1)
    {
#if CRYSTAL_LESS
       /* Start USB trim if it is not enabled. */
        if((SYS->HIRCTRIMCTL & SYS_HIRCTRIMCTL_FREQSEL_Msk) != 1)
        {
            /* Start USB trim only when SOF */
            if(USBD->INTSTS & USBD_INTSTS_SOFIF_Msk)
            {
                /* Clear SOF */
                USBD->INTSTS = USBD_INTSTS_SOFIF_Msk;

                /* Re-enable crystal-less */
                SYS->HIRCTRIMCTL = 0x01;
                SYS->HIRCTRIMCTL |= SYS_HIRCTRIMCTL_REFCKSEL_Msk;
            }
        }

        /* Disable USB Trim when error */
        if(SYS->HIRCTRIMSTS & (SYS_HIRCTRIMSTS_CLKERIF_Msk | SYS_HIRCTRIMSTS_TFAILIF_Msk))
        {
            /* Init TRIM */
            M32(TRIM_INIT) = u32TrimInit;

            /* Disable crystal-less */
            SYS->HIRCTRIMCTL = 0;

            /* Clear error flags */
            SYS->HIRCTRIMSTS = SYS_HIRCTRIMSTS_CLKERIF_Msk | SYS_HIRCTRIMSTS_TFAILIF_Msk;

            /* Clear SOF */
            USBD->INTSTS = USBD_INTSTS_SOFIF_Msk;
        }
#endif
        /* Enter power down when USB suspend */
        // peter if(g_u8Suspend)
        // peter    PowerDown();

        if ( (systick_ms() - pwm_ms) >= 10 )    // 10ms
        {
            pwm_ms = systick_ms();
            PWM_Loop();
        }
    }
}


/*** (C) COPYRIGHT 2018 Nuvoton Technology Corp. ***/

