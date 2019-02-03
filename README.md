STM32F303K8 cmake template
----
This is just a cmake template for the stmr32f303k8 mcu. For the
development I've used the [NUCLEO-F303K8](https://www.st.com/en/evaluation-tools/nucleo-f303k8.html)
board. Therefore, if you use another board then you might need
to do some modifications. This template project only uses the
USART port and a blinking led.

## NUCLEO-F303K8
Regarding the `NUCLEO-F303K8`, the USART2 port, which uses the
pins PA2 and PA15 is proxied though the second mcu (`stm32f103c8t6`).
The second mcu is actually the onboard st-link, but it also
enumerates a CDC USB interface that proxies the trafic from the
stm32f303k8 to your workstation host.

Therefore, if you're using another board, then be aware that you
need to change the USART configuration in the `main.c`.

## Clocks
By default this template uses the internal clock `(HCI)` and the
PLL is set to `72MHz`. You can easily overclock to 128MHz by
changing this line in the `main.c`:

```cpp
sysclk_init(SYSCLK_SOURCE_INTERNAL, SYSCLK_72MHz, 1);
```

to this one:
```cpp
sysclk_init(SYSCLK_SOURCE_INTERNAL, SYSCLK_128MHz, 1);
```

In this call, the last argument (in this case `1`), is the flag
for using safe clocks for the `PCLK1` and `PCLK2`. That means that
for frequencies higher than 72MHz the function will take care of
the clocks not to exceed the max limits. If it's set to `0` then
also the `PCLK1` and `PCLK2` will be overclocked.

## Build and flash
This project was developed and tested on a Linux machine only.
Nevertheless, I've provided also a script for windows machines
to build it, but this script is un-tested and it migh not work.

To build the project you need cmake to be installed to your host
and also you need an ARM toolchain. The toolchain I've used is:
```
gcc-arm-none-eabi-7-2017-q4-major
```

You need to set your toolchain in the `cmake/TOOLCHAIN_arm_none_eabi_cortex_m4.cmake`
file. The current default location is:
```cmake
set(TOOLCHAIN_DIR /opt/toolchains/gcc-arm-none-eabi-7-2017-q4-major)
```

Therefore, you need to change this line to point to your toolchain's
path.

To build the project run:
```sh
./build.sh
```

To make a clean build and delete all previous builds:
```sh
CLEANBUILD=true ./build.sh
```

Then to flash the bin in the mcu:
```sh
./flash.bash
```

## Author
Dimitris Tassopoulos <dimtass@gmail.com>