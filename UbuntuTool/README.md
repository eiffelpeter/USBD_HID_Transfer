
# Install libusb
`sudo apt-get install libhidapi-dev`

# Make
`gcc hid_led.c -o hid_led -lhidapi-libusb`

#execute
`sudo ./hid_led x`

```
0: off
1: on
2: 0~255
3: flash
4: blink
5: breath
```