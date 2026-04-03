# picoSSTV
This project is a SSTV (Martin 1) transmitter utilizing Raspberry Pi Pico and SX1276 module. It is designed to be used on a High Altitude Balloon (HAB) flight.

# Work principle
The images are packed into Pico memory in RGB565 format. SSTV signal is generated using PWM synthesis technique that achieves nearly pure sine wave on average while still being a fully digital signal. It is then fed into the SX1276 module DIO2 (DATA IN) pin. The module is set to FSK continuous mode, changing frequency by deviation value according to the input signal state. This creates (nearly) perfect FM spectrum modulated with the SSTV signal.

# Inspirations and used code
The project is created in the [Raspberry Pi Pico C SDK](https://download.kamami.pl/p587098-raspberry-pi-pico-c-sdk.pdf) using Visual Studio Code extension.

Functioning is highly inspired by https://github.com/SP8ESA/SX1276_HAB_SSTV_TX, differing in modulation method, the method itself is based on [ESP32 SSTV cam](https://www.instructables.com/SSTV-Capsule-V2-for-High-Altitude-Balloons/) project.

# Licence
The code is distributed under GNU GPL 3.0 licence, there is no waranty. See LICENCE.

# Wiring
The RPI Pico should be wired to the SX1276 module accordingly:
| RPI Pico | SX1276 |
|-|-|
| 3V3(OUT) | 3.3V(VCC) |
| GND | GND |
| GPIO 10 | SCK |
| GPIO 11 | MOSI |
| GPIO 12 | MISO |
| GPIO 13 | NSS(CS) |
| GPIO 14 | RST |
| GPIO 15 | DIO2 |

# Loading images
Images are stored in Flash memory in RGB565 format. It allows for 13 maximum images, that is enough for most SSTV events. All images should be stored in `images/` directory, then the `get_images.py` script should be run that will save all of them in a combined binary file. The SSTV mode is Martin 1, the standard resolution is 320x240 pixels, images should be loaded in this resolution. They can also be in higher multiples of it, sustaining 4:3 ratio, they will get automatically converted.

# Compiling


The code can be compiled by installing the RPI Pico SDK and VS Code extension and running the build procedure.

# Images

![2026-04-02-13-30-26-034](https://github.com/user-attachments/assets/00fd1771-06e6-407c-83df-d471dd7539fc)
<img width="642" height="845" alt="image" src="https://github.com/user-attachments/assets/0d3f75e2-7922-456e-ae1f-11bc0237589c" />
