#include <stdio.h> // printf
#include <wchar.h> // wchar_t
#include <stdlib.h>
#include <hidapi/hidapi.h>
#include <string.h>

#pragma pack(1)

#define USB_VID 0x1234
#define USB_PID 0x5678
#define MAX_STR 255

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

int SendLedCmd(hid_device *handle, unsigned char command, unsigned char value)
{
    int res;

    BusyIndicator_t led_output_report;
    memset(&led_output_report, 0, sizeof(BusyIndicator_t));

    led_output_report.report_id = LED_REPORT_ID;
    led_output_report.mode = command;
    led_output_report.blue_pwm_l = value;

    res = hid_write(handle, (unsigned char *)&led_output_report, sizeof(led_output_report));

    return res;
}

int main(int argc, char* argv[])
{
    int res;
    unsigned char buf[65];
    wchar_t wstr[MAX_STR];
    hid_device *handle;
    int i;
    unsigned int cmd = 0, value = 0;

    if(argc == 2)
    {
        cmd = atoi(argv[1]);
    }
    else if(argc == 3)
    {
        cmd = atoi(argv[1]);
        value = atoi(argv[2]);
        value &= 0xFF;
    }
    else
    {
        return 1;
    }

    // Initialize the hidapi library
    res = hid_init();

    // Open the device using the VID, PID,
    // and optionally the Serial number.
    handle = hid_open(USB_VID, USB_PID, NULL);
    if(!handle)
    {
        printf("Unable to open device\n");
        hid_exit();
        return 1;
    }

    // Read the Manufacturer String
    res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
    printf("Manufacturer String: %ls\n", wstr);

    // Read the Product String
    res = hid_get_product_string(handle, wstr, MAX_STR);
    printf("Product String: %ls\n", wstr);

    // Read the Serial Number String
    res = hid_get_serial_number_string(handle, wstr, MAX_STR);
    printf("Serial Number String: (%d) %ls\n", wstr[0], wstr);

    // Read Indexed String 1
    res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
    printf("Indexed String 1: %ls\n", wstr);

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
    hid_close(handle);

    // Finalize the hidapi library
    res = hid_exit();

    return 0;
}
