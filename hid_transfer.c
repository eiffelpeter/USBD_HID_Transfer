/******************************************************************************
 * @file     hid_mouse.c
 * @version  V1.00
 * $Revision: 11 $
 * $Date: 18/07/18 4:54p $
 * @brief    M031 series USBD driver Sample file
 *
 * @note
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
/*!<Includes */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "NuMicro.h"
#include "hid_transfer.h"
#include "pwm_ctrl.h"

int32_t ProcessCommand(void);

uint8_t volatile g_u8EP2Ready = 0;
uint8_t volatile g_u8Suspend = 0;
uint8_t g_u8Idle = 0, g_u8Protocol = 0;
static BusyIndicator_t led_output_report;
static bool handle_ep1_output_report = false;

void USBD_IRQHandler(void)
{
    uint32_t volatile u32IntSts = USBD_GET_INT_FLAG();
    uint32_t volatile u32State = USBD_GET_BUS_STATE();

    if(u32IntSts & USBD_INTSTS_FLDET)
    {
        /* Floating detect */
        USBD_CLR_INT_FLAG(USBD_INTSTS_FLDET);

        if(USBD_IS_ATTACHED())
        {
            /* USB Plug In */
            USBD_ENABLE_USB();
        }
        else
        {
            /* USB Un-plug */
            USBD_DISABLE_USB();
        }
    }

    if(u32IntSts & USBD_INTSTS_BUS)
    {
        /* Clear event flag */
        USBD_CLR_INT_FLAG(USBD_INTSTS_BUS);

        if(u32State & USBD_STATE_USBRST)
        {
            /* Bus reset */
            USBD_ENABLE_USB();
            USBD_SwReset();
            g_u8Suspend = 0;
        }
        if(u32State & USBD_STATE_SUSPEND)
        {
            /* Enter power down to wait USB attached */
            g_u8Suspend = 1;

            /* Enable USB but disable PHY */
            USBD_DISABLE_PHY();
        }
        if(u32State & USBD_STATE_RESUME)
        {
            /* Enable USB and enable PHY */
            USBD_ENABLE_USB();
            g_u8Suspend = 0;
        }
    }

    if(u32IntSts & USBD_INTSTS_SOF)
    {
        /* Clear SOF flag */
        USBD_CLR_INT_FLAG(USBD_INTSTS_SOF);
    }

    if(u32IntSts & USBD_INTSTS_WAKEUP)
    {
        /* Clear event flag */
        USBD_CLR_INT_FLAG(USBD_INTSTS_WAKEUP);
    }

    if(u32IntSts & USBD_INTSTS_USB)
    {
        /* USB event */
        if(u32IntSts & USBD_INTSTS_SETUP)
        {
            /* Setup packet */
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_SETUP);

            /* Clear the data IN/OUT ready flag of control end-points */
            USBD_STOP_TRANSACTION(EP0);
            USBD_STOP_TRANSACTION(EP1);

            USBD_ProcessSetupPacket();
        }

        /* EP events */
        if(u32IntSts & USBD_INTSTS_EP0)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP0);
            /* control IN */
            USBD_CtrlIn();
        }

        if(u32IntSts & USBD_INTSTS_EP1)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP1);
            /* control OUT */
            USBD_CtrlOut();

            if(handle_ep1_output_report)
            {
                ProcessCommand();
                handle_ep1_output_report = false;
            }
        }

        if(u32IntSts & USBD_INTSTS_EP2)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP2);
            /* Interrupt IN */
            EP2_Handler();
        }

        if(u32IntSts & USBD_INTSTS_EP3)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP3);
            /* Interrupt OUT */
            EP3_Handler();
        }

        if(u32IntSts & USBD_INTSTS_EP4)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP4);
        }

        if(u32IntSts & USBD_INTSTS_EP5)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP5);
        }

        if(u32IntSts & USBD_INTSTS_EP6)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP6);
        }

        if(u32IntSts & USBD_INTSTS_EP7)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP7);
        }
    }
}

void EP2_Handler(void)  /* Interrupt IN handler */
{

}

void EP3_Handler(void)  /* Interrupt OUT handler */
{
    uint8_t *ptr;
    /* Interrupt OUT */
    ptr = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP3));
    HID_GetOutReport(ptr, USBD_GET_PAYLOAD_LEN(EP3));
    USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);
}


/*--------------------------------------------------------------------------*/
/**
  * @brief  USBD Endpoint Config.
  * @param  None.
  * @retval None.
  */
void HID_Init(void)
{
    /* Init setup packet buffer */
    /* Buffer range for setup packet -> [0 ~ 0x7] */
    USBD->STBUFSEG = SETUP_BUF_BASE;

    /*****************************************************/
    /* EP0 ==> control IN endpoint, address 0 */
    USBD_CONFIG_EP(EP0, USBD_CFG_CSTALL | USBD_CFG_EPMODE_IN | 0);
    /* Buffer range for EP0 */
    USBD_SET_EP_BUF_ADDR(EP0, EP0_BUF_BASE);

    /* EP1 ==> control OUT endpoint, address 0 */
    USBD_CONFIG_EP(EP1, USBD_CFG_CSTALL | USBD_CFG_EPMODE_OUT | 0);
    /* Buffer range for EP1 */
    USBD_SET_EP_BUF_ADDR(EP1, EP1_BUF_BASE);

    /*****************************************************/
    /* EP2 ==> Interrupt IN endpoint, address 1 */
    USBD_CONFIG_EP(EP2, USBD_CFG_EPMODE_IN | INT_IN_EP_NUM);
    /* Buffer range for EP2 */
    USBD_SET_EP_BUF_ADDR(EP2, EP2_BUF_BASE);

    /* EP3 ==> Interrupt OUT endpoint, address 2 */
    USBD_CONFIG_EP(EP3, USBD_CFG_EPMODE_OUT | INT_OUT_EP_NUM);
    /* Buffer range for EP3 */
    USBD_SET_EP_BUF_ADDR(EP3, EP3_BUF_BASE);
    /* trigger to receive OUT data */
    USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);

}

void HID_ClassRequest(void)
{
    uint8_t buf[8];

    USBD_GetSetupPacket(buf);

    if(buf[0] & 0x80)    /* request data transfer direction */
    {
        /* Device to host */
        switch(buf[1])
        {
        case GET_IDLE:
            {
                USBD_SET_PAYLOAD_LEN(EP1, buf[6]);
                /* Data stage */
                USBD_PrepareCtrlIn(&g_u8Idle, buf[6]);
                /* Status stage */
                USBD_PrepareCtrlOut(0, 0); 
                break;
            }
        case GET_PROTOCOL:
            {
                USBD_SET_PAYLOAD_LEN(EP1, buf[6]);
                /* Data stage */
                USBD_PrepareCtrlIn(&g_u8Protocol, buf[6]);
                /* Status stage */
                USBD_PrepareCtrlOut(0, 0); 
                break;
            }
        case GET_REPORT:
//             {
//                 break;
//             }
        default:
            {
                /* Setup error, stall the device */
                USBD_SetStall(EP0);
                USBD_SetStall(EP1);
                break;
            }
        }
    }
    else
    {
        /* Host to device */
        switch(buf[1])
        {
        case SET_REPORT:  // 0x09
            {
                printf("Host to device SET_REPORT %02x %02x %02x %02x %02x %02x %02x %02x  \n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]); 
                if(buf[3] == 3)
                {
                    /* Request Type = Feature */
                    USBD_SET_DATA1(EP1);
                    USBD_SET_PAYLOAD_LEN(EP1, 0);
                }
                else if(buf[3] == 2)    /* report type output */
                {
                    //printf("report id 0x%x, report length 0x%x \n", buf[2], (buf[7] << 8) | buf[6]);
                    if(LED_REPORT_ID == buf[2])
                    {
                        printf("match report id 0x%x, report length 0x%x \n", buf[2], (buf[7] << 8) | buf[6]);
                        USBD_PrepareCtrlOut((uint8_t *)&led_output_report, sizeof(led_output_report));
                        handle_ep1_output_report = true;
                    }
                    else
                    {
                        printf("dismatch report id 0x%x, expect:0x%x\n", buf[2], LED_REPORT_ID );
                    }

                    /* Request Type = Output */
                    USBD_SET_DATA1(EP1);
                    USBD_SET_PAYLOAD_LEN(EP1, buf[6]);

                    /* Status stage */
                    USBD_PrepareCtrlIn(0, 0);
                }
                else
                {
                    printf("Host to device SET_REPORT buf[3]:%d\n", buf[3]);
                }
                break;
            }
        case SET_IDLE:
            {
                g_u8Idle = buf[3]; 
                /* Status stage */
                USBD_SET_DATA1(EP0);
                USBD_SET_PAYLOAD_LEN(EP0, 0);
                break;
            }
        case SET_PROTOCOL:
            {
                g_u8Protocol = buf[2]; 
                /* Status stage */
                USBD_SET_DATA1(EP0);
                USBD_SET_PAYLOAD_LEN(EP0, 0);
                break;
            }
        default:
            {
                /* Stall */
                /* Setup error, stall the device */
                USBD_SetStall(EP0);
                USBD_SetStall(EP1);
                break;
            }
        }
    }
}

/***************************************************************/

uint32_t CalCheckSum(uint8_t *buf, uint32_t size)
{
    uint32_t sum;
    int32_t i;

    i = 0;
    sum = 0;
    while(size--)
    {
        sum+=buf[i++];
    }

    return sum;

}

int32_t ProcessCommand(void)
{
    int i = 0;

    for(i=0 ; i<sizeof(led_output_report) ; i++)
    {
        printf("%02X ", (unsigned int) ((char*)&led_output_report)[i]);

        if(i%16 == 15)
        {
            printf("\n");
        }
    }
    printf("\n");

    switch(led_output_report.mode)
    {
        case LED_ON:
        {
            printf("LED_ON\n"); 
            PWM_Set_Status_Led(STATUS_LED_BLUE);
            break;
        }
        case LED_OFF:
        {
            printf("LED_OFF\n"); 
            PWM_Set_Status_Led(STATUS_LED_OFF);
            break;
        }
        case LED_FLASH:
        {
            printf("LED_FLASH\n"); 
            PWM_Set_Status_Led(STATUS_LED_BLUE_FLASH);
            break;
        }
        case LED_SLOW_BLINK:
        {
            printf("LED_BLINK\n"); 
            PWM_Set_Status_Led(STATUS_LED_BLUE_BLINK);
            break;
        }
        case LED_BREATH:
        {
            PWM_Set_Status_Led(STATUS_LED_BLUE_BREATH);
            break;
        }
        case LED_UPDATE_PARAMETER:
        {
            printf("LED_PWM_DUTY %d\n", led_output_report.blue_pwm_l ); 
            PWM_Set_Status_Led_Level(led_output_report.blue_pwm_l);
            break;
        }
        default:
        {
            printf("unknow command 0x%x \n", led_output_report.mode);
            return -1;
        }
    }

    return 0;
}

void HID_GetOutReport(uint8_t *pu8EpBuf, uint32_t u32Size)
{
    /* Check and process the command packet */
    USBD_MemCopy((uint8_t *)&led_output_report, pu8EpBuf, u32Size);
    if(ProcessCommand())
    {
        printf("Unknown HID command!\n");
    }

}




