/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) 2010 Nuvoton Technology Corp. All rights reserved.                                        */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "HIDTransferTest.h"
#include "HID.hpp"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

#define USB_VID         0x1234  /* Vendor ID */
#define USB_PID         0x5678  /* Product ID */
#define LED_REPORT_ID   0x01

#define USB_TIME_OUT    100

// 僅有的一個應用程式物件

CWinApp theApp;

using namespace std;

int main(void);

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
    int nRetCode = 0;

    // 初始化 MFC 並於失敗時列印錯誤
    if(!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
    {
        // TODO: 配合您的需要變更錯誤碼
        _tprintf(_T("嚴重錯誤: MFC 初始化失敗\n"));
        nRetCode = 1;
    }
    else
    {
        // TODO: 在此撰寫應用程式行為的程式碼。
        main();




    }

    return nRetCode;
}

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef enum LedMode_e
{
    LED_DO_NOT_CHANGE,
    LED_ON,
    LED_FLASH,
    LED_SLOW_BLINK,
    LED_FAST_BLINK,
    LED_OFF,
    LED_BREATH,
    LED_UPDATE_PARAMETER
} LedMode;

typedef struct BusyIndicatorDef_t
{
    unsigned char report_id;
    unsigned char mode;
    unsigned char red_pwm_l;
    unsigned char red_pwm_h;
    unsigned char green_pwm_l;
    unsigned char green_pwm_h;
    unsigned char blue_pwm_l;
    unsigned char blue_pwm_h;
    unsigned char flash_on_time;
    unsigned char slow_blink_on_time;
    unsigned char slow_blink_off_time;
    unsigned char fast_blink_on_time;
    unsigned char fast_blink_off_time;
} BusyIndicator_t;

#pragma pack(pop)   /* restore original alignment from stack */

unsigned int CalCheckSum(unsigned char *buf, unsigned int size)
{
    unsigned int sum;
    int i;

    i = 0;
    sum = 0;
    while(size--)
    {
        sum += buf[i++];
    }

    return sum;

}

int SendLedCmd(unsigned char command, unsigned char value)
{
    CHidCmd io;
    unsigned long length;
    BOOL bRet;
    bool isDeviceOpened;
    BusyIndicator_t led_output_report;
    memset(&led_output_report, 0, sizeof(BusyIndicator_t));


    isDeviceOpened = 0;
    if(!io.OpenDevice(USB_VID, USB_PID))
    {
        printf("Can't Open HID Device\n");
        goto lexit;
    }
    else
    {
        isDeviceOpened = TRUE;
        printf("USB HID Device VID[%04x] PID[%04x] Open Success.\n", USB_VID, USB_PID);
        printf(">>> Test command\n");

        led_output_report.report_id = LED_REPORT_ID;
        led_output_report.mode = command;
        led_output_report.blue_pwm_l = value;

        bRet = io.WriteFile((unsigned char *)&led_output_report, sizeof(led_output_report), &length, USB_TIME_OUT);
        if(!bRet)
        {
            printf("ERROR: Send test command error!\n");
            goto lexit;
        }
    }


    lexit:

    if(isDeviceOpened)
        io.CloseDevice();

    return 0;
}

int main(void)
{
    char ch[20];
    unsigned int cmd, value;

    do
    {
        memset(ch, '\0' ,sizeof(ch));
        printf("please enter cmd and value, q for quit \n");

        gets(ch);
        //printf("you enter %s\n", ch);   // getchar()

        if( ch[0] == 'q' )
            break;
        else if( ch[2] != '\0')
        {
            sscanf(ch, "%d %d", &cmd, &value);
            printf("you enter cmd:%d and value:%d\n", cmd, value); 
            value &= 0xFF;
        }
        else
        {
            sscanf(ch, "%d", &cmd);
            printf("you enter cmd:%d \n", cmd); 
            value = 0;
        }

        switch(cmd)
        {
        case 1:
            SendLedCmd(LED_ON, value);
            break;
        case 2:
            SendLedCmd(LED_FLASH, value);
            break;
        case 3:
            SendLedCmd(LED_SLOW_BLINK, value);
            break;
        case 4:
            SendLedCmd(LED_FAST_BLINK, value);
            break;
        case 5:
            SendLedCmd(LED_OFF, value);
            break;
        case 6:
            SendLedCmd(LED_BREATH, value);
            break;
        case 7:
            SendLedCmd(LED_UPDATE_PARAMETER, value);  // set duty
            break;
        default:
            printf("unknow cmd:%d \n", cmd); 
            break;
        }


    } while( 1 );


    return 0;


}