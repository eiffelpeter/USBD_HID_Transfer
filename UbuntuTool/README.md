
# Install libusb
`sudo apt-get install libhidapi-dev`

# Make
`gcc hid_led.c -o hid_led -lhidapi-libusb`

#execute
`sudo ./hid_led x`

```
1: on
2: flash
3: blink
5: off
6: breath
7: 0~255
```

# Make
`gcc hid_led_raw.c -o hid_led_raw`

#execute
`sudo ./hid_led_raw n x`

check `n` in /dev/hidraw