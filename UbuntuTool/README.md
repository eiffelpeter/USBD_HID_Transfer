
# Install libusb
`sudo apt-get install libhidapi-dev libusb-1.0-0-dev `

# change to root
`sudo su -`

# Make
`gcc hid_led.c -o hid_led -lhidapi-libusb`

#execute
`./hid_led x`

```
1: on
2: flash
3: blink
5: off
6: breath
7: 0~255
```

# Make
`gcc hid_led_libusb.c -o hid_led_libusb -lusb-1.0`

#execute
`./hid_led_libusb x`

# Make
`gcc hid_led_raw.c -o hid_led_raw`

#execute
`./hid_led_raw n x`

check `n` in /dev/hidraw