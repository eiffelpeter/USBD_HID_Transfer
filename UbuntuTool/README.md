
# Install libusb dev lib
`sudo apt-get install libhidapi-dev`

# Make
`gcc hid_led.c -o hid_led -lhidapi-libusb`

# Execute on Ubuntu 20.04
`sudo ./hid_led x`

```
x
0: off
1: on
2: with 0~255
3: flash
4: blink
5: breath
```
