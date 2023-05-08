#include <stdio.h> // printf
#include <wchar.h> // wchar_t
#include <stdlib.h>
#include <hidapi/hidapi.h>

#pragma pack(1)

#define USB_VID 0x1234
#define USB_PID 0x5678
#define MAX_STR 255

#define HID_CMD_SIGNATURE   0x43444948

/* HID Transfer Commands */
#define HID_CMD_LED_ON          0x3D
#define HID_CMD_LED_OFF         0x41
#define HID_CMD_LED_FLASH       0x3E
#define HID_CMD_LED_SLOW_BLINK  0x3F
#define HID_CMD_LED_BREATH      0x42
#define HID_CMD_LED_PWM_DUTY    0x43

typedef struct
{
    unsigned char cmd;
    unsigned char len;
    unsigned int arg1;
    unsigned int arg2;
    unsigned int signature;
    unsigned int checksum;
} CMD_T;

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

int SendLedCmd(hid_device *handle, unsigned char command, unsigned int value)
{
    int res;
    CMD_T cmd;

    cmd.cmd = command;
    cmd.len = (sizeof(cmd) - 4); /* Not include checksum */
    cmd.arg1 = value;
    cmd.arg2 = 0;
    cmd.signature = HID_CMD_SIGNATURE;
    cmd.checksum = CalCheckSum((unsigned char *)&cmd, cmd.len);

    res = hid_write(handle, (unsigned char *)&cmd, sizeof(cmd));

    return res;
}

int main(int argc, char* argv[])
{
    int res;
    unsigned char buf[65];
    wchar_t wstr[MAX_STR];
    hid_device *handle;
    int i;

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

    unsigned int cmd = 0, value = 0;

    if(argc == 2)
    {
        cmd = atoi(argv[1]);
    }
    else if(argc == 3)
    {
        cmd = atoi(argv[1]);
        value = atoi(argv[2]);
    }
    else
    {
        goto end;
    }


    switch(cmd)
    {
    case 0:
        SendLedCmd(handle, HID_CMD_LED_OFF, value);
        break;
    case 1:
        SendLedCmd(handle, HID_CMD_LED_ON, value);
        break;
    case 2:
        SendLedCmd(handle, HID_CMD_LED_PWM_DUTY, value);
        break;
    case 3:
        SendLedCmd(handle, HID_CMD_LED_FLASH, value);
        break;
    case 4:
        SendLedCmd(handle, HID_CMD_LED_SLOW_BLINK, value);
        break;
    case 5:
        SendLedCmd(handle, HID_CMD_LED_BREATH, value);
        break;
    default:
        break;
    }

end:
    // Close the device
    hid_close(handle);

    // Finalize the hidapi library
    res = hid_exit();

    return 0;
}
