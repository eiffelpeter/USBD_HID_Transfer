#include <stdio.h> // printf
#include <wchar.h> // wchar_t
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#pragma pack(1)

#define LED_REPORT_ID        0x01

typedef enum LedMode_e {
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

int SendLedCmd(int handle, unsigned char command, unsigned char value)
{
    int res;

    BusyIndicator_t led_output_report;
    memset(&led_output_report, 0, sizeof(BusyIndicator_t));

    led_output_report.report_id = LED_REPORT_ID;
    led_output_report.mode = command;
    led_output_report.blue_pwm_l = value;

    res = write(handle, (unsigned char *)&led_output_report, sizeof(led_output_report));

    return res;
}

int main(int argc, char* argv[])
{
    int handle;
    char path[15];
    unsigned int cmd = 0, value = 0;

    if( atoi(argv[1]) < 0 || atoi(argv[1]) > 9)
    {
        printf("please enter hidraw 0~9 \n");
        return 1;
    }

    if(argc == 3)
    {
        cmd = atoi(argv[2]);
    }
    else if(argc == 4)
    {
        cmd = atoi(argv[2]);
        value = atoi(argv[3]);
        value &= 0xFF;
    }
    else
    {
        printf("please reset mcu and find /dev/hidrawX, execute (./hid_led_raw X 1) to turn on LED \n");
        return 1;
    }

    strcpy(path, "/dev/hidraw");
    strcat(path, argv[1]);
    printf("open device %s  \n", path);

    handle = open(path, O_RDWR);
    if(!handle)
    {
        printf("Unable to open device\n");
        return 1;
    }


    switch(cmd)
    {
    case 1:
        SendLedCmd(handle, LED_ON, value);
        break;
    case 2:
        SendLedCmd(handle, LED_FLASH, value);
        break;
    case 3:
        SendLedCmd(handle, LED_SLOW_BLINK, value);
        break;
    case 4:
        SendLedCmd(handle, LED_FAST_BLINK, value);
        break;
    case 5:
        SendLedCmd(handle, LED_OFF, value);
        break;
    case 6:
        SendLedCmd(handle, LED_BREATH, value);
        break;
    case 7:
        SendLedCmd(handle, LED_UPDATE_PARAMETER, value);  // set duty
        break;
    default:
        printf("unknow cmd:%d \n", cmd); 
        break;
    }

    // Close the device
    close(handle);

    return 0;
}
