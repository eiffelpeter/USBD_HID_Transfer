***

# Install Keil 
[link](https://www2.keil.com/mdk5)

# Install Nu-Link_Keil_Driver
[link](https://www.nuvoton.com/tool-and-software/ide-and-compiler/)

# NuMaker-M032KI
<img src="./doc/NuMaker-M032KI.png">

## Pin connection for test
| Item | pin | dir | pin |
|----------|-----|----|------------|
| IEC60730_IO_Test | PB4 | -> | PC1 | 
| IEC60730_SPI_Test | MOSI | -> | MISO |
| IEC60730_PWM_Test | PB.5 | -> | PB.3 |
| IEC60730_AD_Test | PB.0 | -> | LDO_CAP |


## Open IEC60730_TestCode.uvprojx in keil, build and download
<img src="./doc/build_and_download.png">

## log
<img src="./doc/printf.png">