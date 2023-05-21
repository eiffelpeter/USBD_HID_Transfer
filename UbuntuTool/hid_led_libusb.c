#include <stdio.h> // printf
#include <stdint.h>
#include <wchar.h> // wchar_t
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <libusb-1.0/libusb.h>

#pragma pack(1)

#define USB_VID 0x1234
#define USB_PID 0x5678
#define MAX_STR 255

#define LED_REPORT_ID        0x01

// HID Class-Specific Requests values. See section 7.2 of the HID specifications
#define HID_GET_REPORT                0x01
#define HID_SET_REPORT                0x09
#define HID_REPORT_TYPE_INPUT         0x01
#define HID_REPORT_TYPE_OUTPUT        0x02

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

int SendLedCmd(libusb_device_handle  *handle, unsigned char command, unsigned char value)
{
    int res;
    int report_number = LED_REPORT_ID;
    BusyIndicator_t led_output_report;
    memset(&led_output_report, 0, sizeof(BusyIndicator_t));

    led_output_report.report_id = LED_REPORT_ID;
    led_output_report.mode = command;
    led_output_report.blue_pwm_l = value;

    /* Use the Control Endpoint */
    res = libusb_control_transfer(handle,
                                  LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_OUT,
                                  HID_SET_REPORT,
                                  (HID_REPORT_TYPE_OUTPUT << 8) | report_number,
                                  0,
                                  (unsigned char *)&led_output_report, sizeof(led_output_report),
                                  1000/*timeout millis*/);

    if(res < 0)
        printf("libusb_control_transfer return err:%d\n", res);

    return res;
}

int main(int argc, char* argv[])
{
    int ret = 0;
    libusb_device_handle *handle;
    unsigned int cmd = 0, value = 0;

    // get intput parameter
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

    ret = libusb_init(NULL);
    if(ret < 0)
    {
        printf("Unable to init\n");
        return ret;
    }

    printf("Opening device...\n");
    handle = libusb_open_device_with_vid_pid(NULL, USB_VID, USB_PID);
    if(handle == NULL)
    {
        printf("Unable to open device\n");
        return -1;
    }

    if(libusb_kernel_driver_active(handle, 0))
    {
        if(libusb_detach_kernel_driver(handle, 0) != 0)
        {
            printf("libusb_detach_kernel_driver error \n");
            ret = -1;
            goto end;
        }
    }

    ret = libusb_claim_interface(handle, 0);  // bInterfaceNumber
    if(ret < 0)
    {
        printf("libusb_claim_interface(0) failed:[%s] \n", libusb_strerror(ret));
        ret = -1;
        goto end;
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

    libusb_release_interface(handle, 0);  // bInterfaceNumber

end:
    // Close the device
    libusb_close(handle);

    libusb_exit(NULL);
    return ret;
}
